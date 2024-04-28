#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>

#include "Standard.hpp"


extern std::vector<std::string> include_path;

namespace Parser {

    Standard::Standard(std::string code, std::string path) :
        code(std::move(code)), path(std::move(path)) {}

    Standard::Word::Word(std::string word, Parser::Position position) :
        std::string(std::move(word)), position(std::move(position)) {}

    Standard::Standard(std::filesystem::path const& path) :
        path(path) {
        std::ostringstream oss;
        std::ifstream src(path);
        if (src) {
            oss << src.rdbuf();
            code = oss.str();
        }
    }

    Standard::Exception::Exception(std::vector<ParsingError> const& errors) {
        std::ostringstream oss;

        for (auto const& e : errors)
            oss << e.message << " in " << e.position << std::endl;

        message = oss.str();
    }

    const char* Standard::Exception::what() const noexcept {
        return message.c_str();
    }


    std::string const operators = "!$%&*+-/:;<=>?@^|~";
    bool is_operator(char c) {
        return operators.find(c) < operators.size();
    }

    bool is_number(char c) {
        return std::isdigit(c) || c == '.';
    }

    bool is_alphanum(char c) {
        return std::isalnum(c) || c == '_' || c == '`';
    }

    bool is_operator(std::string const& str) {
        return std::all_of(
            str.begin(), str.end(),
            [](char c) { return is_operator(c); }
        );
    }

    bool is_number(std::string const& str) {
        return std::all_of(
            str.begin(), str.end(),
            [](char c) { return is_number(c); }
        );
    }

    bool is_alnum(std::string const& str) {
        return std::all_of(
            str.begin(), str.end(),
            [](char c) { return is_alphanum(c); }
        );
    }

    std::set<std::string> const system_chars = { ".", ",", "\\", "|->" };
    bool is_system(std::string const& str) {
        return system_chars.find(str) != system_chars.end();
    }


    std::vector<Standard::Word> Standard::get_words() const {
        std::vector<Word> words;

        size_t b = 0;
        auto last = '\n';
        bool is_comment = false;
        bool is_str = false;
        bool escape = false;

        size_t line = 1;
        size_t column = 1;
        std::string position = "file " + path + ":" + std::to_string(line) + ":" + std::to_string(column);

        size_t i{};
        for (i = 0; i < code.size(); ++i) {
            char c = code[i];

            if (!is_comment) {
                if (!is_str) {
                    if (c == '#') {
                        if (b < i) words.emplace_back(code.substr(b, i - b), position);
                        b = i + 1;
                        is_comment = true;
                    } else if (std::isspace(c)) {
                        if (b < i) words.emplace_back(code.substr(b, i - b), position);
                        b = i + 1;
                    } else if (c == ',' || c == '\\' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}') {
                        if (b < i) words.emplace_back(code.substr(b, i - b), position);
                        words.emplace_back(std::string(1, c), position);
                        b = i + 1;
                    } else if (c == '\"') {
                        if (b < i) words.emplace_back(code.substr(b, i - b), position);
                        b = i;
                        is_str = true;
                    } else if (is_operator(last) && !is_operator(c) && b < i) {
                        words.emplace_back(code.substr(b, i - b), position);
                        b = i;
                    } else if (is_number(last) && !is_number(c) && b < i) {
                        words.emplace_back(code.substr(b, i - b), position);
                        b = i;
                    } else if (is_alphanum(last) && !is_alphanum(c) && b < i && (!is_number(code.substr(b, i - b)) || c != '.')) {
                        words.emplace_back(code.substr(b, i - b), position);
                        b = i;
                    }
                } else {
                    if (!escape) {
                        if (c == '\"') {
                            words.emplace_back(code.substr(b, i - b + 1), position);
                            b = i + 1;
                            is_str = false;
                        } else if (c == '\\') escape = true;
                    } else escape = false;
                }
            } else {
                if (c == '\n') is_comment = false;
                b = i + 1;
            }

            last = c;
            if (c == '\n') {
                ++line;
                column = 1;
            } else ++column;
            position = "file " + path + ":" + std::to_string(line) + ":" + std::to_string(column);
        }
        if (b < i && !is_comment) words.emplace_back(code.substr(b, i - b), position);

        return words;
    }

    int get_char_priority(char const& c) {
        if (c == '^') return 1;
        if (c == '*' || c == '/' || c == '%') return 2;
        if (c == '+' || c == '-') return 3;
        //if (c == '!' || c == '=' || c == '<' || c == '>') return 4;
        if (c == '&' || c == '|') return 5;
        if (c == ':') return 6;
        if (c == ';') return 7;
        return 4;
    }

    int compare_operators(std::string const& a, std::string const& b) {
        for (size_t i = 0; i < a.size() && i < b.size(); ++i) {
            if (get_char_priority(a[i]) < get_char_priority(b[i])) return 1;
            if (get_char_priority(a[i]) > get_char_priority(b[i])) return -1;
        }
        if (a.size() < b.size()) return 1;
        else if (a.size() > b.size()) return -1;
        else return 0;
    }


    std::shared_ptr<Expression> expressions_to_expression(std::vector<Standard::ParsingError>& errors, std::vector<std::shared_ptr<Expression>> expressions, std::shared_ptr<Expression> expression, bool is_function, std::vector<std::shared_ptr<Expression>> const& escaped) {
        if (expressions.empty()) {
            return expression;
        } else {
            expressions.push_back(expression);

            if (is_function) {
                auto function_call = std::make_shared<FunctionCall>();
                function_call->position = expressions[0]->position;
                function_call->function = expressions[0];

                if (expressions.size() != 2) {
                    auto tuple = std::make_shared<Tuple>();
                    tuple->position = function_call->position;
                    for (size_t i = 1; i < expressions.size(); ++i)
                        tuple->objects.push_back(expressions[i]);
                    function_call->arguments = tuple;
                } else
                    function_call->arguments = expressions[1];

                return function_call;
            } else {
                std::vector<std::vector<std::string>> operators;
                for (auto const& expr : expressions) {
                    if (auto symbol = std::dynamic_pointer_cast<Symbol>(expr)) {
                        if (is_operator(symbol->name) && std::find(escaped.begin(), escaped.end(), symbol) == escaped.end()) {
                            bool put = false;
                            for (auto& op : operators) {
                                if (compare_operators(symbol->name, op[0]) == 0) {
                                    op.push_back(symbol->name);
                                    put = true;
                                    break;
                                }
                            }
                            if (!put) {
                                std::vector<std::string> op;
                                op.push_back(symbol->name);
                                operators.push_back(op);
                            }
                        }
                    }
                }

                std::sort(operators.begin(), operators.end(), [](auto const& a, auto const& b) {
                    return compare_operators(a[0], b[0]) >= 0;
                });

                for (auto const& op : operators) {
                    for (auto it = expressions.begin(); it != expressions.end();) {
                        if (auto symbol = std::dynamic_pointer_cast<Symbol>(*it)) {
                            if (std::find(op.begin(), op.end(), symbol->name) != op.end() && std::find(escaped.begin(), escaped.end(), symbol) == escaped.end()) {
                                if (expressions.begin() <= it - 1 && it + 1 < expressions.end()) {
                                    auto function_call = std::make_shared<FunctionCall>();
                                    function_call->position = symbol->position;
                                    function_call->function = *it;
                                    auto tuple = std::make_shared<Tuple>();
                                    tuple->position = function_call->position;
                                    tuple->objects.push_back(*(it - 1));
                                    tuple->objects.push_back(*(it + 1));
                                    function_call->arguments = tuple;

                                    *(it - 1) = function_call;
                                    it = expressions.erase(it);
                                    it = expressions.erase(it);
                                } else errors.emplace_back("operator " + symbol->name + " must be placed between two expressions", symbol->position);
                            } else ++it;
                        } else ++it;
                    }
                }

                return expressions[0];
            }
        }
    }

    std::shared_ptr<Expression> get_expression(std::vector<Standard::ParsingError>& errors, std::vector<Standard::Word> const& words, size_t& i, std::vector<std::shared_ptr<Expression>>& escaped, bool in_tuple, bool in_function, bool in_operator, bool priority) {
        std::shared_ptr<Expression> expression = nullptr;

        if (words.at(i) == "(") {
            ++i;
            expression = get_expression(errors, words, i, escaped, false, false, false, true);
            escaped.push_back(expression);
            if (words.at(i) == ")") ++i;
            else errors.emplace_back(") expected", words.at(i).position);
        } else if (words.at(i) == ")") {
            expression = std::make_shared<Tuple>();
            if (words[i - 1] != "(") {
                errors.emplace_back(") unexpected", words.at(i).position);
                return expression;
            }
            escaped.push_back(expression);
            expression->position = words[i - 1].position;
        } else if (words.at(i) == "[") {
            ++i;
            expression = get_expression(errors, words, i, escaped, false, false, false, true);
            escaped.push_back(expression);
            if (words.at(i) == "]") ++i;
            else errors.emplace_back("] expected", words.at(i).position);
        } else if (words.at(i) == "]") {
            expression = std::make_shared<Tuple>();
            if (words[i - 1] != "[") {
                errors.emplace_back("] unexpected", words.at(i).position);
                return expression;
            }
            escaped.push_back(expression);
            expression->position = words[i - 1].position;
        } else if (words.at(i) == "{") {
            ++i;
            expression = get_expression(errors, words, i, escaped, false, false, false, true);
            escaped.push_back(expression);
            if (words.at(i) == "}") ++i;
            else errors.emplace_back("} expected", words.at(i).position);
        } else if (words.at(i) == "}") {
            expression = std::make_shared<Tuple>();
            if (words[i - 1] != "{") {
                errors.emplace_back("} unexpected", words.at(i).position);
                return expression;
            }
            escaped.push_back(expression);
            expression->position = words[i - 1].position;
        } else {
            auto symbol = std::make_shared<Symbol>();
            symbol->position = words.at(i).position;
            symbol->name = words.at(i);
            expression = symbol;
            if (is_system(words.at(i)))
                errors.emplace_back(words.at(i) + " is reserved", words.at(i).position);
            ++i;
        }

        auto is_function = false;
        std::vector<std::shared_ptr<Expression>> expressions;
        while (i < words.size()) {

            if (words.at(i) == ")") break;
            if (words.at(i) == "]") break;
            if (words.at(i) == "}") break;

            if (words.at(i) == ".") {
                if (is_function) {
                    expression = expressions_to_expression(errors, expressions, expression, is_function, escaped);
                    expressions.clear();
                }

                if (!in_function) {
                    auto property = std::make_shared<Property>();
                    property->position = words.at(i).position;
                    property->object = expression;
                    ++i;
                    property->name = words.at(i);
                    if (is_system(words.at(i)) && words.at(i) != ".")
                        errors.emplace_back(words.at(i) + " is reserved", words.at(i).position);
                    expression = property;
                    ++i;
                    continue;
                } else break;
            }
            if (words.at(i) == ",") {
                if (!in_tuple && priority) {
                    auto tuple = std::make_shared<Tuple>();
                    tuple->position = words.at(i).position;
                    tuple->objects.push_back(expressions_to_expression(errors, expressions, expression, is_function, escaped));
                    while (words.at(i) == ",") {
                        ++i;
                        auto const& w = words.at(i);
                        if (w == ")" || w == "]" || w == "}")
                            break;
                        else
                            tuple->objects.push_back(get_expression(errors, words, i, escaped, true, false, false, true));
                    }
                    return tuple;
                } else break;
            }
            if (words.at(i) == "\\") {
                if (priority) {
                    ++i;
                    auto filter = get_expression(errors, words, i, escaped, false, false, false, false);
                    if (words.at(i) == "|->") {
                        ++i;
                        auto function_definition = std::make_shared<FunctionDefinition>();
                        function_definition->position = words[i - 1].position;
                        function_definition->parameters = expression;
                        function_definition->filter = filter;
                        function_definition->body = get_expression(errors, words, i, escaped, in_tuple, in_function, in_operator, false);
                        expression = function_definition;
                        continue;
                    } else {
                        errors.emplace_back("|-> expected", words.at(i).position);
                        continue;
                    }
                } else break;
            }
            if (words.at(i) == "|->") {
                if (priority) {
                    ++i;
                    auto function_definition = std::make_shared<FunctionDefinition>();
                    function_definition->position = words[i - 1].position;
                    function_definition->parameters = expression;
                    function_definition->filter = nullptr;
                    function_definition->body = get_expression(errors, words, i, escaped, in_tuple, in_function, in_operator, false);
                    expression = function_definition;
                    continue;
                } else break;
            }
            if (auto symbol = std::dynamic_pointer_cast<Symbol>(expression)) {
                if (is_operator(symbol->name) && std::find(escaped.begin(), escaped.end(), symbol) == escaped.end()) {
                    auto function_call = std::make_shared<FunctionCall>();
                    function_call->position = symbol->position;

                    function_call->function = expression;
                    function_call->arguments = get_expression(errors, words, i, escaped, in_tuple, in_function, in_operator, false);

                    expression = function_call;
                    continue;
                }
            }
            if (is_operator(words.at(i))) {
                if (priority) {
                    if (in_operator) break;
                    else {
                        if (is_function) {
                            expression = expressions_to_expression(errors, expressions, expression, is_function, escaped);
                            expressions.clear();
                            is_function = false;
                        }
                        expressions.push_back(expression);
                        auto symbol = std::make_shared<Symbol>();
                        symbol->position = words.at(i).position;
                        symbol->name = words.at(i);
                        expressions.push_back(symbol);
                        ++i;
                        if (i >= words.size() || words.at(i) == ")" || words.at(i) == "]" || words.at(i) == "}")
                            expression = std::make_shared<Tuple>();
                        else
                            expression = get_expression(errors, words, i, escaped, in_tuple, in_function, true, false);
                        continue;
                    }
                } else break;
            }
            if (!in_function) {
                is_function = true;
                expressions.push_back(expression);
                expression = get_expression(errors, words, i, escaped, in_tuple, true, in_operator, false);
                continue;
            } else break;

        }

        return expressions_to_expression(errors, expressions, expression, is_function, escaped);
    }

    std::shared_ptr<Expression> Standard::get_tree() const {
        if (code == "")
            return nullptr;

        std::vector<Standard::ParsingError> errors;

        try {
            auto words = get_words();
            size_t i = 0;
            std::vector<std::shared_ptr<Expression>> escaped;
            auto expression = get_expression(errors, words, i, escaped, false, false, false, true);

            if (errors.empty())
                return expression;
            else
                throw Standard::Exception(errors);
        } catch (std::out_of_range const&) {
            throw Standard::IncompleteCode();
        }
    }

    std::optional<std::filesystem::path> Standard::get_canonical_path(Position& position, std::filesystem::path const& p) {
        position = position.substr(sizeof("file ") - 1);
        position = position.substr(0, position.find(':'));

        if (position.length() > 0) {
            try {
                auto path = std::filesystem::path(p);

                if (!path.is_absolute())
                    path = std::filesystem::path(position).parent_path() / path;
                return std::filesystem::canonical(path);
            } catch (std::exception const&) {
                for (auto const& i : include_path) {
                    try {
                        auto path = std::filesystem::path(i) / p;

                        return std::filesystem::canonical(path);
                    } catch (std::exception const&) {
                        return p;
                    }
                }

                return {};
            }
        } else return {};
    }

}
