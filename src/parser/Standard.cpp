#include <algorithm>
#include <fstream>
#include <iostream>

#include "Standard.hpp"


namespace Parser {

    namespace Standard {

        TextPosition::TextPosition(std::string const& path, unsigned int line, unsigned int column): line(line), column(column) {
            this->path = path;
        }

        void TextPosition::store_stack_trace(Context & context) {
            stack_trace = "\tin file \"" + path + "\" at line " + std::to_string(line) + ", column " + std::to_string(column) + "\n";

            Context* old_c = nullptr;
            Context* c = &context;
            while (c != old_c) {
                if (c->get_position() != nullptr) {
                    auto text_position = std::static_pointer_cast<TextPosition>(c->get_position());
                    stack_trace += "\tin file \"" + text_position->path + "\" at line " + std::to_string(text_position->line) + ", column " + std::to_string(text_position->column) + "\n";
                }
                old_c = c;
                c = &c->get_parent();
            }
        }

        void TextPosition::notify_error(std::string const& message, bool print_stack_trace) {
            std::cerr << message << std::endl;
            if (print_stack_trace) std::cerr << stack_trace;
        }

        Word::Word(std::string const& word, TextPosition const& position): word(word), position(position) {}


        std::string operators = "!$%&*+-/:;<=>?@^|~";
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
            for (char c : str)
                if (!is_operator(c))
                    return false;
            return true;
        }

        bool is_number(std::string const& str) {
            for (char c : str)
                if (!is_number(c))
                    return false;
            return true;
        }

        bool is_alnum(std::string const& str) {
            for (char c : str)
                if (!is_alphanum(c))
                    return false;
            return true;
        }

        std::vector<std::string> system_chars = {"->", ",", "\\", "|->"};
        bool is_system(std::string const& str) {
            return std::find(system_chars.begin(), system_chars.end(), str) != system_chars.end();
        }


        std::vector<Word> get_words(std::string const& path, std::string const& code) {
            std::vector<Word> words;

            int size = code.size();
            int b = 0;
            auto last = '\n';
            auto is_comment = false;
            auto is_str = false;
            auto escape = false;

            int line = 1;
            int column = 1;
            TextPosition position(path, line, column);

            int i;
            for (i = 0; i < size; i++) {
                char c = code[i];

                if (!is_comment) {
                    if (!is_str) {
                        if (c == '#') {
                            if (b < i) words.push_back(Word(code.substr(b, i-b), position));
                            b = i+1;
                            is_comment = true;
                        } else if (std::isspace(c)) {
                            if (b < i) words.push_back(Word(code.substr(b, i-b), position));
                            b = i+1;
                        } else if (c == ',' || c == '\\' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}') {
                            if (b < i) words.push_back(Word(code.substr(b, i-b), position));
                            words.push_back(Word(std::string(1,c), position));
                            b = i+1;
                        } else if (c == '\"') {
                            if (b < i) words.push_back(Word(code.substr(b, i-b), position));
                            b = i;
                            is_str = true;
                        } else if (is_operator(last) && !is_operator(c) && b < i) {
                            words.push_back(Word(code.substr(b, i-b), position));
                            b = i;
                        } else if (is_number(last) && !is_number(c) && b < i) {
                            words.push_back(Word(code.substr(b, i-b), position));
                            b = i;
                        } else if (is_alphanum(last) && !is_alphanum(c) && b < i) {
                            words.push_back(Word(code.substr(b, i-b), position));
                            b = i;
                        }
                    } else {
                        if (!escape) {
                            if (c == '\"') {
                                words.push_back(Word(code.substr(b, i-b+1), position));
                                b = i+1;
                                is_str = false;
                            } else if (c == '\\') {
                                escape = true;
                            }
                        } else escape = false;
                    }
                } else {
                    if (c == '\n') is_comment = false;
                    b = i+1;
                }

                last = c;
                if (c == '\n') {
                    line++;
                    column = 1;
                } else column++;
                position = TextPosition(path, line, column);
            }
            if (b < i && !is_comment) words.push_back(Word(code.substr(b, i-b), position));

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
            for (int i = 0; i < (int) a.size() && i < (int) b.size(); i++) {
                if (get_char_priority(a[i]) < get_char_priority(b[i])) return 1;
                if (get_char_priority(a[i]) > get_char_priority(b[i])) return -1;
            }
            if (a.size() < b.size()) return 1;
            else if (a.size() > b.size()) return -1;
            else return 0;
        }

        bool comparator(std::vector<std::string> const& a, std::vector<std::string> const& b) {
            return compare_operators(a[0], b[0]) >= 0;
        }


        ParserError::ParserError(std::string const& message, TextPosition const& position): message(message), position(position) {}

        std::shared_ptr<Expression> expressions_to_expression(std::vector<ParserError> & errors, std::vector<std::shared_ptr<Expression>> expressions, std::shared_ptr<Expression> expression, bool is_function, std::vector<std::shared_ptr<Expression>> const& escaped) {
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
                        for (unsigned long i = 1; i < expressions.size(); i++)
                            tuple->objects.push_back(expressions[i]);
                        function_call->arguments = tuple;
                    } else function_call->arguments = expressions[1];

                    return function_call;
                } else {
                    std::vector<std::vector<std::string>> operators;
                    for (auto const& expr : expressions) {
                        if (auto symbol = std::dynamic_pointer_cast<Symbol>(expr)) {
                            if (is_operator(symbol->name) && std::find(escaped.begin(), escaped.end(), symbol) == escaped.end()) {
                                bool put = false;
                                for (auto & op : operators) {
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

                    std::sort(operators.begin(), operators.end(), comparator);

                    for (auto const& op : operators) {
                        for (auto it = expressions.begin(); it != expressions.end();) {
                            if (auto symbol = std::dynamic_pointer_cast<Symbol>(*it)) {
                                if (std::find(op.begin(), op.end(), symbol->name) != op.end() && std::find(escaped.begin(), escaped.end(), symbol) == escaped.end()) {
                                    if (expressions.begin() <= it-1 && it+1 < expressions.end()) {
                                        auto function_call = std::make_shared<FunctionCall>();
                                        function_call->position = symbol->position;
                                        function_call->function = *it;
                                        auto tuple = std::make_shared<Tuple>();
                                        tuple->position = function_call->position;
                                        tuple->objects.push_back(*(it-1));
                                        tuple->objects.push_back(*(it+1));
                                        function_call->arguments = tuple;

                                        *(it-1) = function_call;
                                        it = expressions.erase(it);
                                        it = expressions.erase(it);
                                    } else errors.push_back(ParserError("operator " + symbol->name + " must be placed between two expressions", *std::static_pointer_cast<TextPosition>(symbol->position)));
                                } else it++;
                            } else it++;
                        }
                    }

                    return expressions[0];
                }
            }
        }

        std::shared_ptr<Expression> get_expression(std::vector<ParserError> & errors, std::vector<Word> const& words, unsigned long &i, std::vector<std::shared_ptr<Expression>> & escaped, bool in_tuple, bool in_function, bool in_operator, bool priority) {
            std::shared_ptr<Expression> expression = nullptr;

            if (words.at(i).word == "(") {
                ++i;
                expression = get_expression(errors, words, i, escaped, false, false, false, true);
                escaped.push_back(expression);
                if (words.at(i).word == ")") ++i;
                else errors.push_back(ParserError(") expected", words.at(i).position));
            } else if (words.at(i).word == ")") {
                expression = std::make_shared<Tuple>();
                if (words[i-1].word != "(") {
                    errors.push_back(ParserError(") unexpected", words.at(i).position));
                    return expression;
                }
                escaped.push_back(expression);
                expression->position = std::make_shared<TextPosition>(words[i-1].position);
            } else if (words.at(i).word == "[") {
                ++i;
                expression = get_expression(errors, words, i, escaped, false, false, false, true);
                escaped.push_back(expression);
                if (words.at(i).word == "]") ++i;
                else errors.push_back(ParserError("] expected", words.at(i).position));
            } else if (words.at(i).word == "]") {
                expression = std::make_shared<Tuple>();
                if (words[i-1].word != "[") {
                    errors.push_back(ParserError("] unexpected", words.at(i).position));
                    return expression;
                }
                escaped.push_back(expression);
                expression->position = std::make_shared<TextPosition>(words[i-1].position);
            } else if (words.at(i).word == "{") {
                ++i;
                expression = get_expression(errors, words, i, escaped, false, false, false, true);
                escaped.push_back(expression);
                if (words.at(i).word == "}") ++i;
                else errors.push_back(ParserError("} expected", words.at(i).position));
            } else if (words.at(i).word == "}") {
                expression = std::make_shared<Tuple>();
                if (words[i-1].word != "{") {
                    errors.push_back(ParserError("} unexpected", words.at(i).position));
                    return expression;
                }
                escaped.push_back(expression);
                expression->position = std::make_shared<TextPosition>(words[i-1].position);
            } else {
                std::shared_ptr<Symbol> symbol = std::make_shared<Symbol>();
                symbol->position = std::make_shared<TextPosition>(words.at(i).position);
                symbol->name = words.at(i).word;
                expression = symbol;
                if (is_system(words.at(i).word))
                    errors.push_back(ParserError(words.at(i).word + " is reserved", words.at(i).position));
                ++i;
            }

            auto is_function = false;
            std::vector<std::shared_ptr<Expression>> expressions;
            while (i < words.size()) {

                if (words.at(i).word == ")") break;
                if (words.at(i).word == "]") break;
                if (words.at(i).word == "}") break;

                if (words.at(i).word == "->") {
                    while (words.at(i).word == "->") {
                        auto property = std::make_shared<Property>();
                        property->position = std::make_shared<TextPosition>(words.at(i).position);
                        property->object = expression;
                        ++i;
                        property->name = words.at(i).word;
                        expression = property;
                        if (is_system(words.at(i).word))
                            errors.push_back(ParserError(words.at(i).word + " is reserved", words.at(i).position));
                        ++i;
                    }
                    continue;
                }
                if (words.at(i).word == ",") {
                    if (!in_tuple && priority) {
                        auto tuple = std::make_shared<Tuple>();
                        tuple->position = std::make_shared<TextPosition>(words.at(i).position);
                        tuple->objects.push_back(expressions_to_expression(errors, expressions, expression, is_function, escaped));
                        while (words.at(i).word == ",") {
                            ++i;
                            tuple->objects.push_back(get_expression(errors, words, i, escaped, true, false, false, true));
                        }
                        return tuple;
                    } else break;
                }
                if (words.at(i).word == "\\") {
                    if (priority) {
                        ++i;
                        auto filter = get_expression(errors, words, i, escaped, false, false, false, false);
                        if (words.at(i).word == "|->") {
                            ++i;
                            auto function_definition = std::make_shared<FunctionDefinition>();
                            function_definition->position = std::make_shared<TextPosition>(words[i-1].position);
                            function_definition->parameters = expression;
                            function_definition->filter = filter;
                            function_definition->body = get_expression(errors, words, i, escaped, in_tuple, in_function, in_operator, false);
                            expression = function_definition;
                            continue;
                        } else {
                            errors.push_back(ParserError("|-> expected", words.at(i).position));
                            continue;
                        }
                    } else break;
                }
                if (words.at(i).word == "|->") {
                    if (priority) {
                        ++i;
                        auto function_definition = std::make_shared<FunctionDefinition>();
                        function_definition->position = std::make_shared<TextPosition>(words[i-1].position);
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
                if (is_operator(words.at(i).word)) {
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
                            symbol->position = std::make_shared<TextPosition>(words.at(i).position);
                            symbol->name = words.at(i).word;
                            expressions.push_back(symbol);
                            ++i;
                            if (i >= words.size() || words.at(i).word == ")" || words.at(i).word == "]" || words.at(i).word == "}")
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

        void find_symbols(std::shared_ptr<Expression> expression, std::shared_ptr<Expression> parent) {
            if (expression == nullptr) return;

            if (parent != nullptr)
                expression->symbols = parent->symbols;

            if (auto function_call = std::dynamic_pointer_cast<FunctionCall>(expression)) {
                find_symbols(function_call->function, expression);
                find_symbols(function_call->arguments, expression);
            } else if (auto function_definition = std::dynamic_pointer_cast<FunctionDefinition>(expression)) {
                find_symbols(function_definition->parameters, expression);
                find_symbols(function_definition->filter, expression);
                find_symbols(function_definition->body, expression);
            } else if (auto property = std::dynamic_pointer_cast<Property>(expression)) {
                find_symbols(property->object, expression);
            } else if (auto symbol = std::dynamic_pointer_cast<Symbol>(expression)) {
                expression->symbols.push_back(symbol->name);
            } else if (auto tuple = std::dynamic_pointer_cast<Tuple>(expression)) {
                for (auto & ex : tuple->objects)
                    find_symbols(ex, expression);
            }

            if (parent != nullptr && !std::dynamic_pointer_cast<FunctionDefinition>(expression))
                for (auto const& name : expression->symbols)
                    if (std::find(parent->symbols.begin(), parent->symbols.end(), name) == parent->symbols.end())
                        parent->symbols.push_back(name);
        }

        std::shared_ptr<Expression> get_tree(std::vector<ParserError> & errors, std::string const& path, std::string const& code, std::vector<std::string> symbols) {
            try {
                auto words = get_words(path, code);
                unsigned long i = 0;
                std::vector<std::shared_ptr<Expression>> escaped;
                auto expression = get_expression(errors, words, i, escaped, false, false, false, true);

                expression->symbols = symbols;
                find_symbols(expression, nullptr);

                return expression;
            } catch (std::out_of_range const& e) {
                throw IncompleteCode();
            }
        }

        std::shared_ptr<Expression> get_tree(std::string const& code, std::string const& path, std::vector<std::string> symbols) {
            std::vector<ParserError> errors;
            auto tree = get_tree(errors, path, code, symbols);
            //std::cout << tree->to_string() << std::endl;
            if (errors.empty()) {
                return tree;
            } else {
                for (auto & e : errors)
                    std::cerr << e.message << " in file \"" << e.position.path << "\" at line " << e.position.line << ", column " << e.position.column << "." << std::endl;
                return nullptr;
            }
        }

    }

}
