#include <algorithm>

#include "expression/Expression.hpp"
#include "expression/FunctionCall.hpp"
#include "expression/FunctionDefinition.hpp"
#include "expression/Property.hpp"
#include "expression/Symbol.hpp"
#include "expression/Tuple.hpp"

#include "Standard.hpp"


std::vector<std::string> systemChars = {"->", ",", "(", ")", "[", "]", "{", "}", "like", "|->"};

bool is_sys(std::string const& w) {
    return std::find(systemChars.begin(), systemChars.end(), w) != systemChars.end();
}

bool is_alphanum(char const& c) {
    return std::isalnum(c) || c == '_' || c == '\'' || c == '`';
}

std::vector<std::string> getWords(std::string const& code) {
    std::vector<std::string> words;

    int size = code.size();
    int b = 0;
    char last = '\n';
    bool is_str = false;
    bool escape = false;

    for (int i = 0; i < size; i++) {
        char c = code[i];

        if (!is_str) {
            if (std::isspace(c)) {
                if (b < i) words.push_back(code.substr(b, i-b));
                b = i+1;
            } else if (c == ',' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}') {
                if (b < i) words.push_back(code.substr(b, i-b));
                std::string s(1,c);
                words.push_back(s);
                b = i+1;
            } else if (c == '\"') {
                if (b < i) words.push_back(code.substr(b, i-b));
                b = i;
                is_str = true;
            } else if (!is_alphanum(last) && is_alphanum(c) && !(std::isdigit(last) && c == '.') && b < i) {
                words.push_back(code.substr(b, i-b));
                b = i;
            } else if (is_alphanum(last) && !is_alphanum(c) && !(last == '.' && std::isdigit(c)) && b < i) {
                words.push_back(code.substr(b, i-b));
                b = i;
            }
        } else {
            if (!escape) {
                if (c == '\"') {
                    words.push_back(code.substr(b, i-b+1));
                    b = i+1;
                    is_str = false;
                } else if (c == '\\') {
                    escape = true;
                }
            } else escape = false;
        }

        last = c;
    }

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
    expressions.push_back(expression);
    if (isFunction) {
        std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
        functioncall->function = expressions[0];

        if (expressions.size() != 2) {
            std::shared_ptr<Tuple> tuple = std::make_shared<Tuple>();
            for (unsigned int i = 1; i < expressions.size(); i++)
                tuple->objects.push_back(expressions[i]);
            functioncall->object = tuple;
        } else functioncall->object = expressions[1];

        return functioncall;
    } else {
        std::vector<std::vector<std::string>> operators;
        for (std::shared_ptr<Expression> expr : expressions) {
            if (expr->type == Expression::Symbol) {
                std::shared_ptr<Symbol> symbol = std::static_pointer_cast<Symbol>(expr);
                if (!is_alphanum(symbol->name[0]) && symbol->name[0] != '.' && symbol->name[0] != '\"') {
                    for (std::vector<std::string> & op : operators)
                        if (compareOperators(symbol->name, op[0]) == 0)
                            op.push_back(symbol->name);
                    std::vector<std::string> op;
                    op.push_back(symbol->name);
                    operators.push_back(op);
                }
            }
        }

        std::sort(operators.begin(), operators.end(), comparator);

        for (std::vector<std::string> op : operators) {
            for (std::vector<std::shared_ptr<Expression>>::iterator it = expressions.begin(); it != expressions.end();) {
                if ((*it)->type == Expression::Symbol) {
                    std::shared_ptr<Symbol> symbol = std::static_pointer_cast<Symbol>(*it);
                    if (std::find(op.begin(), op.end(), symbol->name) != op.end()) {
                        std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
                        functioncall->function = symbol;
                        std::shared_ptr<Tuple> tuple = std::make_shared<Tuple>();
                        tuple->objects.push_back(*(it-1));
                        tuple->objects.push_back(*(it+1));
                        functioncall->object = tuple;
                        
                        *(it-1) = functioncall;
                        it = expressions.erase(it);
                        it = expressions.erase(it);
                    } else it++;
                } else it++;
            }
        }

        return expressions[0];
    }
}

std::shared_ptr<Expression> getExpression(std::vector<std::string> const& words, int &i, bool inTuple, bool inFunction, bool inOperator, bool priority) {
    std::shared_ptr<Expression> expression = nullptr;

    if (words[i] == "(") {
        i++;
        expression = getExpression(words, i, false, false, false, true);
        if (words[i] == ")") i++;
        else throw "Error";
    } else if (words[i] == ")") {
        if (words[i-1] == "(") expression = std::make_shared<Tuple>();
        else throw "Error";
    } else if (words[i] == "[") {
        i++;
        expression = getExpression(words, i, false, false, false, true);
        if (words[i] == "]") i++;
        else throw "Error";
    } else if (words[i] == "]") {
        if (words[i-1] == "[") expression = std::make_shared<Tuple>();
        else throw "Error";
    } else if (words[i] == "{") {
        i++;
        expression = getExpression(words, i, false, false, false, true);
        if (words[i] == "}") i++;
        else throw "Error";
    } else if (words[i] == "}") {
        if (words[i-1] == "{") expression = std::make_shared<Tuple>();
        else throw "Error";
    } else if (!is_sys(words[i])) {
        std::shared_ptr<Symbol> symbol = std::make_shared<Symbol>();
        symbol->name = words[i];
        expression = symbol;
        i++;
    } else throw "Error";

    bool isFunction = false;
    std::vector<std::shared_ptr<Expression>> expressions;
    while (i < (int) words.size()) {

        if (words[i] == ")") {
            break;
        }
        if (words[i] == "]") {
            break;
        }
        if (words[i] == "}") {
            break;
        }
        if (words[i] == "->") {
            while (words[i] == "->") {
                i++;
                if (!is_sys(words[i])) {
                    std::shared_ptr<Property> property = std::make_shared<Property>();
                    property->object = expression;
                    property->name = words[i];
                    expression = property;
                    i++;
                } else throw "Symbol name " + words[i] + " is not allowed";
            }
            continue;
        }
        if (words[i] == ",") {
            if (!inTuple && priority) {
                std::shared_ptr<Tuple> tuple = std::make_shared<Tuple>();
                tuple->objects.push_back(expressionsToExpression(expressions, expression, isFunction));
                while (words[i] == ",") {
                    i++;
                    tuple->objects.push_back(getExpression(words, i, true, false, false, true));
                }
                return tuple;
            } break;
        }
        if (words[i] == "like") {
            if (priority) {
                i++;
                std::shared_ptr<Expression> like = getExpression(words, i, false, false, false, false);
                if (words[i] == "|->") {
                    i++;
                    std::shared_ptr<FunctionDefinition> functionDefinition = std::make_shared<FunctionDefinition>();
                    functionDefinition->parameters = expression;
                    functionDefinition->filter = like;
                    functionDefinition->object = getExpression(words, i, inTuple, inFunction, inOperator, false);
                    expression = functionDefinition;
                    continue;
                } else throw "Error";
            } break;
        }
        if (words[i] == "|->") {
            if (priority) {
                i++;
                std::shared_ptr<FunctionDefinition> functionDefinition = std::make_shared<FunctionDefinition>();
                functionDefinition->parameters = expression;
                functionDefinition->filter = nullptr;
                functionDefinition->object = getExpression(words, i, inTuple, inFunction, inOperator, false);
                expression = functionDefinition;
                continue;
            } break;
        }
        if (expression->type == Expression::Symbol) {
            std::shared_ptr<Symbol> symbol = std::static_pointer_cast<Symbol>(expression);

            if (!is_alphanum(symbol->name[0])) {
                std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();

                functioncall->function = symbol;
                functioncall->object = getExpression(words, i, inTuple, inFunction, inOperator, false);
                
                expression = functioncall;
                continue;
            }
        }
        if (!is_alphanum(words[i][0]) && !is_sys(words[i])) {
            if (priority) {
                if (inOperator) break;
                else {
                    if (isFunction) {
                        expression = expressionsToExpression(expressions, expression, isFunction);
                        expressions.clear();
                        isFunction = false;
                    }
                    expressions.push_back(expression);
                    std::shared_ptr<Symbol> symbol = std::make_shared<Symbol>();
                    symbol->name = words[i];
                    i++;
                    expressions.push_back(symbol);
                    expression = getExpression(words, i, inTuple, inFunction, inOperator, false);
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
        std::shared_ptr<FunctionCall> functionCall = std::static_pointer_cast<FunctionCall>(expression);

        findSymbols(functionCall->function, expression, symbols, false);
        findSymbols(functionCall->object, expression, symbols, false);
    } else if (type == Expression::FunctionDefinition) {
        std::shared_ptr<FunctionDefinition> functionDefinition = std::static_pointer_cast<FunctionDefinition>(expression);

        std::shared_ptr<Expression> expr = std::make_shared<Expression>();
        std::vector<std::string> symbolsCopy = symbols;
        findSymbols(functionDefinition->parameters, expr, symbolsCopy, false);
        findSymbols(functionDefinition->filter, expr, symbolsCopy, false);
        findSymbols(functionDefinition->object, expr, symbolsCopy, true);
    } else if (type == Expression::Property) {
        std::shared_ptr<Property> property = std::static_pointer_cast<Property>(expression);

        findSymbols(property->object, expression, symbols, false);
    } else if (type == Expression::Symbol) {
        std::shared_ptr<Symbol> symbol = std::static_pointer_cast<Symbol>(expression);

        expression->usedSymbols = {symbol->name};
    } else if (type == Expression::Tuple) {
        std::shared_ptr<Tuple> tuple = std::static_pointer_cast<Tuple>(expression);

        for (std::shared_ptr<Expression> ex : tuple->objects)
            findSymbols(ex, expression, symbols, false);
    }

    if (newScope)
        for (std::string name : expression->usedSymbols)
            if (std::find(symbols.begin(), symbols.end(), name) == symbols.end()) {
                expression->newSymbols.push_back(name);
                symbols.push_back(name);
            }

    for (std::string name : expression->usedSymbols)
        if (std::find(parent->usedSymbols.begin(), parent->usedSymbols.end(), name) == parent->usedSymbols.end())
            parent->usedSymbols.push_back(name);
}

std::shared_ptr<Expression> StandardParser::getTree(std::string code, std::vector<std::string> symbols) {
    std::vector<std::string> words = getWords(code);
    int i = 0;
    std::shared_ptr<Expression> expression = getExpression(words, i, false, false, false, true);
    std::shared_ptr<Expression> expr = std::make_shared<Expression>();
    findSymbols(expression, expr, symbols, true);
    return expression;
}