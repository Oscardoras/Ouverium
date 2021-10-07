#include <algorithm>

#include "expression/Condition.hpp"
#include "expression/ConditionAlternative.hpp"
#include "expression/ConditionRepeat.hpp"
#include "expression/Expression.hpp"
#include "expression/FunctionCall.hpp"
#include "expression/FunctionDefinition.hpp"
#include "expression/Record.hpp"
#include "expression/Tuple.hpp"
#include "expression/VariableCall.hpp"


bool isalphanum(char const& c) {
    return std::isalnum(c) || c == '_' || c == '\'' || c == '`';
}

std::vector<std::string> system = {"\"", "(", ")", ",", ".", "[", "]", "\\", "{", "}", ":=", "like", "->", "|->", "if", "else", "while", "true", "false"};

bool issys(std::string const& w) {
    return std::find(system.begin(), system.end(), w) != system.end();
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
        } else if (c == ',' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == '$' || c == '\\') {
            if (b < i) words.push_back(code.substr(b, i-b));
            words.push_back(""+c);
            b = i+1;
        }

        last = c;
    }

    return words;
}

std::shared_ptr<VariableCall> getVariable(std::vector<std::string> const& words, int &i) {
    std::shared_ptr<VariableCall> variable = std::make_shared<VariableCall>();
    i -= 2;
    do {
        i += 2;
        variable->variableName = words[i];
        if (!issys(variable->variableName)) throw "Variable name " + variable->variableName + " is not allowed";
    } while (words[i+1] == "->");
    i++;
    return variable;
}

std::shared_ptr<Expression> getVariables(std::vector<std::string> const& words, int &i) {
    std::shared_ptr<Expression> expression = nullptr;
    expression = getVariable(words, i);
    i++;
    if (words[i] == ",") {
        std::shared_ptr<Tuple> tuple = std::make_shared<Tuple>();
        tuple->objects.push_back(expression);
        while (words[i] == ",") {
            i++;
            tuple->objects.push_back(getVariable(words, i));
        }
        expression = tuple;
    }
    return expression;
}

std::shared_ptr<Expression> getExpression(std::vector<std::string> const& words, int &i, bool const& priority) {
    std::shared_ptr<Expression> expression = nullptr;

    if (words[i] == "(") {
        i++;
        expression = getExpression(words, i, true);
    } else if (words[i] == "if") {
        i++;
        std::shared_ptr<Expression> c = getExpression(words, i, false);
        if (words[i] == "then") {
            i++;
            std::shared_ptr<Expression> o = getExpression(words, i, false);
            if (words[i] == "else") {
                i++;
                std::shared_ptr<Expression> a = getExpression(words, i, false);
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
        std::shared_ptr<Condition> condition = std::make_shared<Condition>();
        condition->condition = getExpression(words, i, false);
        if (words[i] == "then") {
            i++;
            condition->object = getExpression(words, i, false);
            expression = condition;
        } else throw "Error";
    } else if (!issys(words[i])) {
        std::shared_ptr<Expression> variables = getVariables(words, i);

        std::string word2 = words[i];
        if (word2 == "like") {
            i++;
            std::shared_ptr<Expression> like = getExpression(words, i, false);
            if (words[i] == "|->") {
                std::shared_ptr<FunctionDefinition> functionDefinition = std::make_shared<FunctionDefinition>();
                functionDefinition->variables = variables;
                functionDefinition->filter = like;
                i++;
                functionDefinition->object = getExpression(words, i, false);
                expression = functionDefinition;
            } else throw "Error";
        } else if (variables->getType() == "VariableCall") {
            std::shared_ptr<VariableCall> variable = std::static_pointer_cast<VariableCall>(variables);

            if (!isalphanum(variable->getLastVariable().variableName[0])) {
                std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
                functioncall->function = variable;
                functioncall->object = getExpression(words, i, false);
                i++;
                expression = functioncall;
            } else {
                expression = variable;
            }
        }
    } else throw "Error";

    if (words[i] == ")") {
        i++;
        return expression;
    }

    if (expression == nullptr) throw "Error";

    if (words[i] == "(") {
        i++;
        std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
        functioncall->function = expression;
        functioncall->object = getExpression(words, i, true);
        expression = functioncall;
    } else if (!isalphanum(words[i][0]) && priority) {
        if (words[i] == ",") {
            std::shared_ptr<Tuple> tuple = std::make_shared<Tuple>();
            tuple->objects.push_back(expression);
            while (words[i] == ",") {
                i++;
                tuple->objects.push_back(getExpression(words, i, false));
            }
            i++;
            expression = tuple;
        } else {
            std::shared_ptr<FunctionCall> functioncall = std::make_shared<FunctionCall>();
            functioncall->function = getVariable(words, i);
            i++;
            std::shared_ptr<Tuple> tuple = std::make_shared<Tuple>();
            tuple->objects.push_back(expression);
            tuple->objects.push_back(getExpression(words, i, true));
            functioncall->object = tuple;
            expression = functioncall;
        }
    }

    return expression;
}


std::shared_ptr<Expression> getTree(std::string code) {
    std::vector<std::string> words = getWords(code);
    int i = 0;
    return getExpression(words, i, true);
}