#include <algorithm>
#include <iostream>

#include "expression/Expression.hpp"
#include "expression/FunctionCall.hpp"
#include "expression/FunctionDefinition.hpp"
#include "expression/Property.hpp"
#include "expression/Symbol.hpp"
#include "expression/Tuple.hpp"

#include "Standard.hpp"


namespace StandardParser {

    bool is_operator(char c) {
        return !std::isalnum(c) && c != '_' && c != '\'' && c != '`' && c != '\"' && c != '.'
        && c != '(' && c != '[' && c != '{' && c != ')' && c != ']' && c != '}';
    }

    bool is_number(char c) {
        return std::isdigit(c) || c == '.';
    }

    bool is_alnum(char c) {
        return std::isalnum(c) || c == '_' || c == '\'' || c == '`';
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
            if (!is_alnum(c))
                return false;
        return true;
    }

    std::vector<std::string> systemChars = {"->", ",", "(", ")", "[", "]", "{", "}", "?", "|->"};

    bool is_system(std::string const& str) {
        return std::find(systemChars.begin(), systemChars.end(), str) != systemChars.end();
    }


    TextPosition::TextPosition(std::string const& path, int line, int column): line(line), column(column) {
        this->path = path;
    }

    void TextPosition::notify() {
        std::cerr << "Arguments given to the function don't match in file \"" << path << "\" at line " << line << ", column " << column << "." << std::endl;
    }

    Word::Word(std::string const& word, TextPosition position): word(word), position(position) {}

    ParserError::ParserError(std::string const& message, TextPosition position): message(message), position(position) {}

    std::vector<Word> getWords(std::string const& path, std::string const& code) {
        int size = code.size();
        std::vector<TextPosition> chars;
        {
            int line = 1;
            int column = 1;
            for (int i = 0; i < size; i++) {
                chars.push_back(TextPosition(path, line, column));

                char c = code[i];
                if (c == '\n') {
                    line++;
                    column = 1;
                } else column++;
            }
        }

        std::vector<Word> words;
        int b = 0;
        auto last = '\n';
        auto is_str = false;
        auto escape = false;

        int i;
        for (i = 0; i < size; i++) {
            char c = code[i];

            if (!is_str) {
                if (std::isspace(c)) {
                    if (b < i) words.push_back(Word(code.substr(b, i-b), chars[b]));
                    b = i+1;
                } else if (c == ',' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}') {
                    if (b < i) words.push_back(Word(code.substr(b, i-b), chars[b]));
                    std::string s(1,c);
                    words.push_back(Word(s, chars[b]));
                    b = i+1;
                } else if (c == '\"') {
                    if (b < i) words.push_back(Word(code.substr(b, i-b), chars[b]));
                    b = i;
                    is_str = true;
                } else if (is_operator(last) && !is_operator(c) && b < i) {
                    words.push_back(Word(code.substr(b, i-b), chars[b]));
                    b = i;
                } else if (is_number(last) && !is_number(c) && b < i) {
                    words.push_back(Word(code.substr(b, i-b), chars[b]));
                    b = i;
                } else if (is_alnum(last) && !is_alnum(c) && b < i) {
                    words.push_back(Word(code.substr(b, i-b), chars[b]));
                    b = i;
                }
            } else {
                if (!escape) {
                    if (c == '\"') {
                        words.push_back(Word(code.substr(b, i-b+1), chars[b]));
                        b = i+1;
                        is_str = false;
                    } else if (c == '\\') {
                        escape = true;
                    }
                } else escape = false;
            }

            last = c;
        }
        if (b < i) words.push_back(Word(code.substr(b, i-b), chars[b]));

        return words;
    }

    int getCharPriority(char const& c) {
        if (c == '^') return 1;
        if (c == '*' || c == '/' || c == '%') return 2;
        if (c == '+' || c == '-') return 3;
        //if (c == '!' || c == '=' || c == '<' || c == '>') return 4;
        if (c == '&' || c == '|') return 5;
        if (c == ':') return 6;
        if (c == ';') return 7;
        return 4;
    }

    int compareOperators(std::string const& a, std::string const& b) {
        for (int i = 0; i < (int) a.size() && i < (int) b.size(); i++) {
            if (getCharPriority(a[i]) < getCharPriority(b[i])) return 1;
            if (getCharPriority(a[i]) > getCharPriority(b[i])) return -1;
        }
        if (a.size() < b.size()) return 1;
        else if (a.size() > b.size()) return -1;
        else return 0;
    }

    bool comparator(std::vector<std::string> const& a, std::vector<std::string> const& b) {
        return compareOperators(a[0], b[0]) >= 0;
    }

    std::shared_ptr<Expression> expressionsToExpression(std::vector<std::shared_ptr<Expression>> expressions, std::shared_ptr<Expression> expression, bool isFunction) {
        if (expressions.empty()) {
            return expression;
        } else {
            expressions.push_back(expression);
            if (isFunction) {
                auto functioncall = std::make_shared<FunctionCall>();
                functioncall->position = expressions[0]->position;
                functioncall->function = expressions[0];

                if (expressions.size() != 2) {
                    auto tuple = std::make_shared<Tuple>();
                    tuple->position = functioncall->position;
                    for (unsigned long i = 1; i < expressions.size(); i++)
                        tuple->objects.push_back(expressions[i]);
                    functioncall->object = tuple;
                } else functioncall->object = expressions[1];

                return functioncall;
            } else {
                std::vector<std::vector<std::string>> operators;
                for (auto expr : expressions) {
                    if (expr->type == Expression::Symbol) {
                        auto symbol = std::static_pointer_cast<Symbol>(expr);
                        if (!symbol->escaped && is_operator(symbol->name)) {
                            bool put = false;
                            for (auto & op : operators) {
                                if (compareOperators(symbol->name, op[0]) == 0) {
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

                for (auto op : operators) {
                    for (auto it = expressions.begin(); it != expressions.end();) {
                        if ((*it)->type == Expression::Symbol) {
                            auto symbol = std::static_pointer_cast<Symbol>(*it);
                            if (!symbol->escaped && std::find(op.begin(), op.end(), symbol->name) != op.end()) {
                                if (expressions.begin() <= it-1 && it+1 < expressions.end()) {
                                    auto functioncall = std::make_shared<FunctionCall>();
                                    functioncall->position = symbol->position;
                                    functioncall->function = symbol;
                                    auto tuple = std::make_shared<Tuple>();
                                    tuple->position = functioncall->position;
                                    tuple->objects.push_back(*(it-1));
                                    tuple->objects.push_back(*(it+1));
                                    functioncall->object = tuple;
                                    
                                    *(it-1) = functioncall;
                                    it = expressions.erase(it);
                                    it = expressions.erase(it);
                                } else throw ParserError("operator " + symbol->name + " must be placed between two expressions", *std::static_pointer_cast<TextPosition>(symbol->position));
                            } else it++;
                        } else it++;
                    }
                }

                return expressions[0];
            }
        }
    }

    std::shared_ptr<Expression> getExpression(std::vector<Word> const& words, int &i, bool inTuple, bool inFunction, bool inOperator, bool priority) {
        if (i >= (int) words.size()) throw IncompleteError();
        
        std::shared_ptr<Expression> expression = nullptr;

        if (words[i].word == "(") {
            i++;
            expression = getExpression(words, i, false, false, false, true);
            expression->escaped = true;
            if (words[i].word == ")") i++;
            else throw ParserError(") expected", words[i].position);
        } else if (words[i].word == ")") {
            if (words[i-1].word == "(") expression = std::make_shared<Tuple>();
            else throw ParserError(") unexpected", words[i].position);
            expression->escaped = true;
            expression->position = std::make_shared<TextPosition>(words[i-1].position);
        } else if (words[i].word == "[") {
            i++;
            expression = getExpression(words, i, false, false, false, true);
            expression->escaped = true;
            if (words[i].word == "]") i++;
            else throw ParserError("] expected", words[i].position);
        } else if (words[i].word == "]") {
            if (words[i-1].word == "[") expression = std::make_shared<Tuple>();
            else throw ParserError("] unexpected", words[i].position);
            expression->escaped = true;
            expression->position = std::make_shared<TextPosition>(words[i-1].position);
        } else if (words[i].word == "{") {
            i++;
            expression = getExpression(words, i, false, false, false, true);
            expression->escaped = true;
            if (words[i].word == "}") i++;
            else throw ParserError("} expected", words[i].position);
        } else if (words[i].word == "}") {
            if (words[i-1].word == "{") expression = std::make_shared<Tuple>();
            else throw ParserError("} unexpected", words[i].position);
            expression->escaped = true;
            expression->position = std::make_shared<TextPosition>(words[i-1].position);
        } else if (!is_system(words[i].word)) {
            std::shared_ptr<Symbol> symbol = std::make_shared<Symbol>();
            symbol->position = std::make_shared<TextPosition>(words[i].position);
            symbol->name = words[i].word;
            expression = symbol;
            i++;
        } else throw ParserError(words[i].word + " is reserved", words[i].position);

        auto isFunction = false;
        std::vector<std::shared_ptr<Expression>> expressions;
        while (i < (int) words.size()) {

            if (words[i].word == ")") break;
            if (words[i].word == "]") break;
            if (words[i].word == "}") break;

            if (words[i].word == "->") {
                while (words[i].word == "->") {
                    i++;
                    if (!is_system(words[i].word)) {
                        auto property = std::make_shared<Property>();
                        property->position = std::make_shared<TextPosition>(words[i-1].position);
                        property->object = expression;
                        property->name = words[i].word;
                        expression = property;
                        i++;
                    } else throw ParserError(words[i].word + " is reserved", words[i].position);
                }
                continue;
            }
            if (words[i].word == ",") {
                if (!inTuple && priority) {
                    auto tuple = std::make_shared<Tuple>();
                    tuple->position = std::make_shared<TextPosition>(words[i].position);
                    tuple->objects.push_back(expressionsToExpression(expressions, expression, isFunction));
                    while (words[i].word == ",") {
                        i++;
                        tuple->objects.push_back(getExpression(words, i, true, false, false, true));
                    }
                    return tuple;
                } else break;
            }
            if (words[i].word == "?") {
                if (priority) {
                    i++;
                    auto like = getExpression(words, i, false, false, false, false);
                    if (words[i].word == "|->") {
                        i++;
                        auto functionDefinition = std::make_shared<FunctionDefinition>();
                        functionDefinition->position = std::make_shared<TextPosition>(words[i-1].position);
                        functionDefinition->parameters = expression;
                        functionDefinition->filter = like;
                        functionDefinition->object = getExpression(words, i, inTuple, inFunction, inOperator, false);
                        expression = functionDefinition;
                        continue;
                    } else throw ParserError("|-> expected", words[i].position);
                } else break;
            }
            if (words[i].word == "|->") {
                if (priority) {
                    i++;
                    auto functionDefinition = std::make_shared<FunctionDefinition>();
                    functionDefinition->position = std::make_shared<TextPosition>(words[i-1].position);
                    functionDefinition->parameters = expression;
                    functionDefinition->filter = nullptr;
                    functionDefinition->object = getExpression(words, i, inTuple, inFunction, inOperator, false);
                    expression = functionDefinition;
                    continue;
                } else break;
            }
            if (expression->type == Expression::Symbol) {
                auto symbol = std::static_pointer_cast<Symbol>(expression);

                if (!symbol->escaped && is_operator(symbol->name)) {
                    auto functioncall = std::make_shared<FunctionCall>();
                    functioncall->position = symbol->position;

                    functioncall->function = symbol;
                    functioncall->object = getExpression(words, i, inTuple, inFunction, inOperator, false);
                    
                    expression = functioncall;
                    continue;
                }
            }
            if (is_operator(words[i].word) && !is_system(words[i].word)) {
                if (priority) {
                    if (inOperator) break;
                    else {
                        if (isFunction) {
                            expression = expressionsToExpression(expressions, expression, isFunction);
                            expressions.clear();
                            isFunction = false;
                        }
                        expressions.push_back(expression);
                        auto symbol = std::make_shared<Symbol>();
                        symbol->position = std::make_shared<TextPosition>(words[i].position);
                        symbol->name = words[i].word;
                        i++;
                        expressions.push_back(symbol);
                        expression = getExpression(words, i, inTuple, inFunction, true, false);
                        continue;
                    }
                } else break;
            }
            if (!inFunction) {
                isFunction = true;
                expressions.push_back(expression);
                expression = getExpression(words, i, inTuple, true, inOperator, false);
                continue;
            } else break;

        }

        return expressionsToExpression(expressions, expression, isFunction);
    }

    void findSymbols(std::shared_ptr<Expression> & expression, std::shared_ptr<Expression> & parent, std::vector<std::string> & symbols, bool newScope) {
        if (expression == nullptr) return;

        auto type = expression->type;
        if (type == Expression::FunctionCall) {
            auto functionCall = std::static_pointer_cast<FunctionCall>(expression);

            findSymbols(functionCall->function, expression, symbols, false);
            findSymbols(functionCall->object, expression, symbols, false);
        } else if (type == Expression::FunctionDefinition) {
            auto functionDefinition = std::static_pointer_cast<FunctionDefinition>(expression);

            auto symbolsCopy = symbols;
            findSymbols(functionDefinition->parameters, expression, symbolsCopy, true);
            findSymbols(functionDefinition->filter, expression, symbolsCopy, true);
            findSymbols(functionDefinition->object, expression, symbolsCopy, true);
        } else if (type == Expression::Property) {
            auto property = std::static_pointer_cast<Property>(expression);

            findSymbols(property->object, expression, symbols, false);
        } else if (type == Expression::Symbol) {
            auto symbol = std::static_pointer_cast<Symbol>(expression);

            expression->usedSymbols = {symbol->name};
            symbols.push_back(symbol->name);
        } else if (type == Expression::Tuple) {
            auto tuple = std::static_pointer_cast<Tuple>(expression);

            for (auto ex : tuple->objects)
                findSymbols(ex, expression, symbols, false);
        }

        for (auto name : expression->usedSymbols)
            if (std::find(parent->usedSymbols.begin(), parent->usedSymbols.end(), name) == parent->usedSymbols.end())
                if (!newScope || std::find(symbols.begin(), symbols.end(), name) != symbols.end())
                    parent->usedSymbols.push_back(name);
    }

    std::shared_ptr<Expression> getTree(std::string const& path, std::string const& code, std::vector<std::string> symbols) {
        auto words = getWords(path, code);
        int i = 0;
        auto expression = getExpression(words, i, false, false, false, true);
        auto expr = std::make_shared<Expression>();
        findSymbols(expression, expr, symbols, true);
        return expression;
    }

}