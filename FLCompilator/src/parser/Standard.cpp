#include <algorithm>

#include "expression/Condition.hpp"
#include "expression/ConditionAlternative.hpp"
#include "expression/ConditionRepeat.hpp"
#include "expression/Expression.hpp"
#include "expression/FunctionCall.hpp"
#include "expression/FunctionDefinition.hpp"
#include "expression/Property.hpp"
#include "expression/Symbol.hpp"
#include "expression/Tuple.hpp"

#include "Standard.hpp"


std::vector<std::string> systemChars = {"\"", "\\", "->", ".", ",", "(", ")", "[", "]", "{", "}", "like", "|->", "if", "then", "else", "while", "repeat"};

bool issys(std::string const& w) {
    return std::find(systemChars.begin(), systemChars.end(), w) != systemChars.end();
}

bool isalphanum(char const& c) {
    return std::isalnum(c) || c == '_' || c == '\'' || c == '`';
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
            } else if (!isalphanum(last) && isalphanum(c) && !(std::isdigit(last) && c == '.') && b < i) {
                words.push_back(code.substr(b, i-b));
                b = i;
            } else if (isalphanum(last) && !isalphanum(c) && !(last == '.' && std::isdigit(c)) && b < i) {
                words.push_back(code.substr(b, i-b));
                b = i;
            } else if (c == ',' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == '\\') {
                if (b < i) words.push_back(code.substr(b, i-b));
                std::string s(1,c);
                words.push_back(s);
                b = i+1;
            } else if (c == '"') {
                is_str = true;
                b = i;
            }
        } else {
            if (!escape) {
                if (c == '"') {
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

std::shared_ptr<Expression> expressionsToExpression(std::vector<std::shared_ptr<Expression>> expressions, std::shared_ptr<Expression> expression) {
    expressions.push_back(expression);

    std::vector<std::vector<std::string>> operators;
    for (std::shared_ptr<Expression> expr : expressions) {
        if (expr->getType() == "Symbol") {
            std::shared_ptr<Symbol> symbol = std::static_pointer_cast<Symbol>(expr);
            if (!isalphanum(symbol->name[0]) && symbol->name[0] != '.' && symbol->name[0] != '"') {
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
            if ((*it)->getType() == "Symbol") {
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

std::shared_ptr<Expression> getExpression(std::vector<std::string> const& words, int &i, bool const& isTuple, bool const& priority) {
    std::shared_ptr<Expression> expression = nullptr;

    if (words[i] == "(") {
        i++;
        expression = getExpression(words, i, false, true);
        if (words[i] == ")") i++;
        else throw "Error";
    } else if (words[i] == ")") {
        if (words[i-1] == "(") expression = std::make_shared<Tuple>();
        else throw "Error";
    } else if (words[i] == "[") {
        i++;
        expression = getExpression(words, i, false, true);
        if (words[i] == "]") i++;
        else throw "Error";
    } else if (words[i] == "]") {
        if (words[i-1] == "[") expression = std::make_shared<Tuple>();
        else throw "Error";
    } else if (words[i] == "{") {
        i++;
        expression = getExpression(words, i, false, true);
        if (words[i] == "}") i++;
        else throw "Error";
    } else if (words[i] == "}") {
        if (words[i-1] == "{") expression = std::make_shared<Tuple>();
        else throw "Error";
    } else if (words[i] == "if") {
        i++;
        std::shared_ptr<Expression> c = getExpression(words, i, false, false);
        if (words[i] == "then") {
            i++;
            std::shared_ptr<Expression> o = getExpression(words, i, isTuple, false);
            if (words[i] == "else") {
                i++;
                std::shared_ptr<Expression> a = getExpression(words, i, isTuple, false);
                std::shared_ptr<ConditionAlternative> condition = std::make_shared<ConditionAlternative>();
                condition->condition = c;
                condition->object = o;
                condition->alternative = a;
                expression = condition;
            } else {
                std::shared_ptr<Condition> condition = std::make_shared<Condition>();
                condition->condition = c;
                condition->object = o;
                expression = condition;
            }
        } else throw "Error";
    } else if (words[i] == "while") {
        i++;
        std::shared_ptr<ConditionRepeat> conditionRepeat = std::make_shared<ConditionRepeat>();
        conditionRepeat->condition = getExpression(words, i, false, false);
        if (words[i] == "repeat") {
            i++;
            conditionRepeat->object = getExpression(words, i, isTuple, false);
            expression = conditionRepeat;
        } else throw "Error";
    } else if (!issys(words[i]) || words[i][0] == '.' || words[i][0] == '"') {
        std::shared_ptr<Symbol> symbol = std::make_shared<Symbol>();
        symbol->name = words[i];
        expression = symbol;
        i++;
    } else throw "Error";

    std::vector<std::shared_ptr<Expression>> expressions;
    while (i < (int) words.size()) {

        if (words[i] == "then" || words[i] == "else" || words[i] == "repeat") {
            return expressionsToExpression(expressions, expression);
        }
        if (words[i] == "(") {
            i++;
            std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
            functioncall->function = expression;
            functioncall->object = getExpression(words, i, false, true);
            if (words[i] == ")") i++;
            else throw "Error";
            expression = functioncall;
            continue;
        }
        if (words[i] == "[") {
            i++;
            std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
            functioncall->function = expression;
            functioncall->object = getExpression(words, i, false, true);
            if (words[i] == "]") i++;
            else throw "Error";
            expression = functioncall;
            continue;
        }
        if (words[i] == "{") {
            i++;
            std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
            functioncall->function = expression;
            functioncall->object = getExpression(words, i, false, true);
            if (words[i] == "}") i++;
            else throw "Error";
            expression = functioncall;
            continue;
        }
        if (words[i] == ")") {
            return expressionsToExpression(expressions, expression);
        }
        if (words[i] == "]") {
            return expressionsToExpression(expressions, expression);
        }
        if (words[i] == "}") {
            return expressionsToExpression(expressions, expression);
        }
        if (words[i] == "->") {
            while (words[i] == "->") {
                i++;
                if (!issys(words[i])) {
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
            if (!isTuple && priority) {
                std::shared_ptr<Tuple> tuple = std::make_shared<Tuple>();
                tuple->objects.push_back(expressionsToExpression(expressions, expression));
                while (words[i] == ",") {
                    i++;
                    tuple->objects.push_back(getExpression(words, i, true, true));
                }
                expressions.clear();
                expression = tuple;
                continue;
            } else return expressionsToExpression(expressions, expression);
        }
        if (words[i] == "like") {
            i++;
            std::shared_ptr<Expression> like = getExpression(words, i, false, false);
            if (words[i] == "|->") {
                i++;
                std::shared_ptr<FunctionDefinition> functionDefinition = std::make_shared<FunctionDefinition>();
                functionDefinition->parameters = expression;
                functionDefinition->filter = like;
                functionDefinition->object = getExpression(words, i, isTuple, false);
                expression = functionDefinition;
                continue;
            } else throw "Error";
        }
        if (words[i] == "|->") {
            if (priority) {
                i++;
                std::shared_ptr<FunctionDefinition> functionDefinition = std::make_shared<FunctionDefinition>();
                functionDefinition->parameters = expression;
                functionDefinition->filter = nullptr;
                functionDefinition->object = getExpression(words, i, isTuple, false);
                expression = functionDefinition;
                continue;
            } else return expressionsToExpression(expressions, expression);
        }
        if (expression->getType() == "Symbol") {
            std::shared_ptr<Symbol> symbol = std::static_pointer_cast<Symbol>(expression);

            if (!isalphanum(symbol->name[0])) {
                std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
                functioncall->function = symbol;
                functioncall->object = getExpression(words, i, isTuple, false);
                if (functioncall->object != nullptr)
                    expression = functioncall;
                continue;
            }
        }
        if (!isalphanum(words[i][0]) && !issys(words[i])) {
            if (priority) {
                expressions.push_back(expression);
                std::shared_ptr<Symbol> symbol = std::make_shared<Symbol>();
                symbol->name = words[i];
                i++;
                expressions.push_back(symbol);
                expression = getExpression(words, i, isTuple, false);
                continue;
            } else return expressionsToExpression(expressions, expression);
        }

    }

    return expressionsToExpression(expressions, expression);
}

void findSymbols(std::shared_ptr<Expression> & expression, std::shared_ptr<Expression> & parent, std::vector<std::string> & symbols, bool newScope) {
    if (expression == nullptr) return;

    std::string type = expression->getType();
    if (type == "Condition") {
        std::shared_ptr<Condition> condition = std::static_pointer_cast<Condition>(expression);

        findSymbols(condition->condition, expression, symbols, false);
        findSymbols(condition->object, expression, symbols, false);
    } else if (type == "ConditionAlternative") {
        std::shared_ptr<ConditionAlternative> alternativeCondition = std::static_pointer_cast<ConditionAlternative>(expression);

        findSymbols(alternativeCondition->condition, expression, symbols, false);
        findSymbols(alternativeCondition->object, expression, symbols, false);
        findSymbols(alternativeCondition->alternative, expression, symbols, false);
    } else if (type == "ConditionRepeat") {
        std::shared_ptr<ConditionRepeat> conditionRepeat = std::static_pointer_cast<ConditionRepeat>(expression);

        findSymbols(conditionRepeat->condition, expression, symbols, false);
        findSymbols(conditionRepeat->object, expression, symbols, false);
    } else if (type == "FunctionCall") {
        std::shared_ptr<FunctionCall> functionCall = std::static_pointer_cast<FunctionCall>(expression);

        findSymbols(functionCall->function, expression, symbols, false);
        findSymbols(functionCall->object, expression, symbols, false);
    } else if (type == "FunctionDefinition") {
        std::shared_ptr<FunctionDefinition> functionDefinition = std::static_pointer_cast<FunctionDefinition>(expression);

        std::shared_ptr<Expression> expr = std::make_shared<Expression>();
        std::vector<std::string> symbolsCopy = symbols;
        findSymbols(functionDefinition->parameters, expr, symbolsCopy, false);
        findSymbols(functionDefinition->filter, expr, symbolsCopy, false);
        findSymbols(functionDefinition->object, expr, symbolsCopy, true);
    } else if (type == "Symbol") {
        std::shared_ptr<Symbol> symbol = std::static_pointer_cast<Symbol>(expression);

        expression->usedSymbols = {symbol->name};
    } else if (type == "Tuple") {
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
    std::shared_ptr<Expression> expression = getExpression(words, i, false, true);
    std::shared_ptr<Expression> expr = std::make_shared<Expression>();
    findSymbols(expression, expr, symbols, true);
    return expression;
}