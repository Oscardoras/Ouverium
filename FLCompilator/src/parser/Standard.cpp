#include <algorithm>

#include "expression/Condition.hpp"
#include "expression/ConditionAlternative.hpp"
#include "expression/ConditionRepeat.hpp"
#include "expression/Expression.hpp"
#include "expression/FunctionCall.hpp"
#include "expression/FunctionDefinition.hpp"
#include "expression/PropertyCall.hpp"
#include "expression/Record.hpp"
#include "expression/Tuple.hpp"
#include "expression/Variable.hpp"


bool isalphanum(char const& c) {
    return std::isalnum(c) || c == '_' || c == '\'' || c == '`';
}

int getCharPriority(char const& c) {
    if (c == '^') return 1;
    if (c == '*' || c == '/' || c == '%') return 2;
    if (c == '+' || c == '-') return 3;
    if (c == '&' || c == '|') return 4;
    //if (c == '!' || c == '=' || c == '<' || c == '>') return 5;
    if (c == ':') return 6;
    if (c == ';') return 7;
    return 5;
}

std::vector<std::string> systemChars = {"\"", "(", ")", ",", ".", "[", "]", "\\", "{", "}", "like", "->", "|->", "if", "then", "else", "while", "repeat"};

bool issys(std::string const& w) {
    return std::find(systemChars.begin(), systemChars.end(), w) != systemChars.end();
}

std::vector<std::string> getWords(std::string const& code) {
    std::vector<std::string> words;
    int size = code.size();
    int b = 0;
    char last = '\n';
    for (int i = 0; i < size; i++) {
        char c = code[i];

        if (std::isspace(c)) {
            if (b < i) words.push_back(code.substr(b, i-b));
            b = i+1;
        } else if (isalphanum(c) && !isalphanum(last) && b < i) {
            words.push_back(code.substr(b, i-b));
            b = i;
        } else if (!isalphanum(c) && isalphanum(last) && b < i) {
            words.push_back(code.substr(b, i-b));
            b = i;
        } else if (c == ',' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == '\\') {
            if (b < i) words.push_back(code.substr(b, i-b));
            std::string s(1,c);
            words.push_back(s);
            b = i+1;
        }

        last = c;
    }

    return words;
}

std::shared_ptr<Expression> getExpression(std::vector<std::string> const& words, int &i, bool const& isTuple, bool const& hasLike, bool const& priority) {
    std::shared_ptr<Expression> expression = nullptr;

    if (words[i] == "(") {
        i++;
        expression = getExpression(words, i, false, false, true);
        if (words[i] == ")") i++;
    } else if (words[i] == "if") {
        i++;
        std::shared_ptr<Expression> c = getExpression(words, i, false, false, false);
        if (words[i] == "then") {
            i++;
            std::shared_ptr<Expression> o = getExpression(words, i, false, false, false);
            if (words[i] == "else") {
                i++;
                std::shared_ptr<Expression> a = getExpression(words, i, false, false, false);
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
        conditionRepeat->condition = getExpression(words, i, false, false, false);
        if (words[i] == "repeat") {
            i++;
            conditionRepeat->object = getExpression(words, i, false, false, false);
            expression = conditionRepeat;
        } else throw "Error";
    } else if (!issys(words[i])) {
        std::shared_ptr<Variable> variable = std::make_shared<Variable>();
        variable->variableName = words[i];
        expression = variable;
        i++;
    }

    while (i < (int) words.size()) {

        if (words[i] == "if" || words[i] == "then" || words[i] == "else" || words[i] == "while" || words[i] == "repeat") {
            return expression;
        }
        if (words[i] == ")") {
            return expression == nullptr && words[i-1] == "(" ? std::make_shared<Tuple>() : expression;
        }
        if (words[i] == "->") {
            while (words[i] == "->") {
                i++;
                if (!issys(words[i])) {
                    std::shared_ptr<PropertyCall> propertyCall = std::make_shared<PropertyCall>();
                    propertyCall->object = expression;
                    propertyCall->variableName = words[i];
                    expression = propertyCall;
                    i++;
                } else throw "Variable name " + words[i] + " is not allowed";
            }
        }
        if (words[i] == ",") {
            if (!isTuple) {
                std::shared_ptr<Tuple> tuple = std::make_shared<Tuple>();
                tuple->objects.push_back(expression);
                while (words[i] == ",") {
                    i++;
                    tuple->objects.push_back(getExpression(words, i, true, false, false));
                }
                expression = tuple;
            } else return expression;
        }
        if (words[i] == "like") {
            i++;
            std::shared_ptr<Expression> like = getExpression(words, i, false, true, false);
            if (words[i] == "|->") {
                i++;
                std::shared_ptr<FunctionDefinition> functionDefinition = std::make_shared<FunctionDefinition>();
                functionDefinition->parameters = expression;
                functionDefinition->filter = like;
                functionDefinition->object = getExpression(words, i, false, false, false);
                expression = functionDefinition;
            } else throw "Error";
        }
        if (words[i] == "|->") {
            if (!hasLike) {
                i++;
                std::shared_ptr<FunctionDefinition> functionDefinition = std::make_shared<FunctionDefinition>();
                functionDefinition->parameters = expression;
                functionDefinition->filter = nullptr;
                functionDefinition->object = getExpression(words, i, false, false, false);
                expression = functionDefinition;
            } else return expression;
        }
        if (expression->getType() == "Variable") {
            std::shared_ptr<Variable> variable = std::static_pointer_cast<Variable>(expression);

            if (!isalphanum(variable->variableName[0])) {
                std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
                functioncall->function = variable;
                functioncall->object = getExpression(words, i, false, false, false);
                if (functioncall->object != nullptr) expression = functioncall;
            }
        }
        if (!isalphanum(words[i][0]) && !issys(words[i])) {
            if (priority) {
                std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
                std::shared_ptr<Variable> variable = std::make_shared<Variable>();
                variable->variableName = words[i];
                functioncall->function = variable;
                i++;
                std::shared_ptr<Tuple> tuple = std::make_shared<Tuple>();
                tuple->objects.push_back(expression);
                tuple->objects.push_back(getExpression(words, i, false, false, false));
                functioncall->object = tuple;
                expression = functioncall;
            } else return expression;
        }
        if (words[i] == "(") {
            i++;
            std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
            functioncall->function = expression;
            functioncall->object = getExpression(words, i, false, false, true);
            if (words[i] == ")") i++;
            expression = functioncall;
        }

    }

    return expression;
}


std::shared_ptr<Expression> getTree(std::string code) {
    std::vector<std::string> words = getWords(code);
    int i = 0;
    return getExpression(words, i, false, false, true);
}


std::string tabu(int n) {
    std::string s = "";
    for (int i = 0; i < n; i++) s+= "    ";
    return s;
}

std::string printTree(std::shared_ptr<Expression> const expression, int n = 0) {
    std::string s = "";

    if (expression == nullptr) return "NULL\n";

    std::string type = expression->getType();
    if (type == "Condition") {
        std::shared_ptr<Condition> condition = std::static_pointer_cast<Condition>(expression);

        s += "Condition :\n";
        n++;
        s += tabu(n) + "condition : " + printTree(condition->condition, n);
        s += tabu(n) + "object : " + printTree(condition->object, n);
    } else if (type == "ConditionAlternative") {
        std::shared_ptr<ConditionAlternative> alternativeCondition = std::static_pointer_cast<ConditionAlternative>(expression);

        s += "ConditionAlternative :\n";
        n++;
        s += tabu(n) + "condition : " + printTree(alternativeCondition->condition, n);
        s += tabu(n) + "object : " + printTree(alternativeCondition->object, n);
        s += tabu(n) + "alternative : " + printTree(alternativeCondition->alternative, n);
    } else if (type == "ConditionRepeat") {
        std::shared_ptr<ConditionRepeat> conditionRepeat = std::static_pointer_cast<ConditionRepeat>(expression);

        s += "ConditionRepeat :\n";
        n++;
        s += tabu(n) + "condition : " + printTree(conditionRepeat->condition, n);
        s += tabu(n) + "object : " + printTree(conditionRepeat->object, n);
    } else if (type == "FunctionCall") {
        std::shared_ptr<FunctionCall> functionCall = std::static_pointer_cast<FunctionCall>(expression);

        s += "FunctionCall :\n";
        n++;
        s += tabu(n) + "function : " + printTree(functionCall->function, n);
        s += tabu(n) + "object : " + printTree(functionCall->object, n);
    } else if (type == "FunctionDefinition") {
        std::shared_ptr<FunctionDefinition> functionDefinition = std::static_pointer_cast<FunctionDefinition>(expression);

        s += "FunctionDefinition :\n";
        n++;
        s += tabu(n) + "parameters : " + printTree(functionDefinition->parameters, n);
        s += tabu(n) + "filter : " + printTree(functionDefinition->filter, n);
        s += tabu(n) + "object : " + printTree(functionDefinition->object, n);
    } else if (type == "Tuple") {
        std::shared_ptr<Tuple> tuple = std::static_pointer_cast<Tuple>(expression);

        s += "Tuple :\n";
        n++;
        for (std::shared_ptr<Expression> ex : tuple->objects) s += tabu(n) + printTree(ex, n);
    } else if (type == "Variable") {
        std::shared_ptr<Variable> variable = std::static_pointer_cast<Variable>(expression);
        
        s += "Variable : " + variable->variableName + "\n";
    }

    return s;
}