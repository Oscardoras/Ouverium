#include "../../parser/Standard.cpp"
#include "structure/JArray.hpp"
#include "structure/JAssignment.hpp"
#include "structure/JDelete.hpp"
#include "structure/JFuncEval.hpp"
#include "structure/JFunction.hpp"
#include "structure/JIf.hpp"
#include "structure/JIfElse.hpp"
#include "structure/JReturn.hpp"
#include "structure/JTernary.hpp"
#include "structure/JUndefined.hpp"
#include "structure/JVariable.hpp"
#include "structure/JWhile.hpp"


void append(std::vector<std::shared_ptr<JInstruction>> & vec, std::vector<std::shared_ptr<JInstruction>> const& add) {
    vec.insert(vec.end(), add.begin(), add.end());
}

class Instr {

public:

    std::vector<std::string> variables;
    std::vector<std::shared_ptr<JInstruction>> instructions;

    void addVariable(std::string variable) {
        variables.push_back(variable);
    }

    void addInstruction(std::shared_ptr<JInstruction> instruction) {
        instructions.push_back(instruction);
    }

    void append(Instr const& instr) {
        this->instructions.insert(this->instructions.end(), instr.instructions.begin(), instr.instructions.end());
        this->variables.insert(this->variables.end(), instr.variables.begin(), instr.variables.end());
    }

    void appendInstructionsTo(std::vector<std::shared_ptr<JInstruction>> & instructions) {
        instructions.insert(instructions.end(), this->instructions.begin(), this->instructions.end());
    }

};

class Expr: public Instr {

public:
    std::shared_ptr<JExpression> expression;

};

std::string charConvertor(char c) {
    if (c == '!') return "__a33";
    else if (c == '#') return "__a35";
    else if (c == '$') return "__a36";
    else if (c == '%') return "__a37";
    else if (c == '&') return "__a38";
    else if (c == '*') return "__a42";
    else if (c == '+') return "__a43";
    else if (c == '-') return "__a45";
    else if (c == '/') return "__a47";
    else if (c == ':') return "__a58";
    else if (c == ';') return "__a59";
    else if (c == '<') return "__a60";
    else if (c == '=') return "__a61";
    else if (c == '>') return "__a62";
    else if (c == '?') return "__a63";
    else if (c == '@') return "__a64";
    else if (c == '^') return "__a94";
    else if (c == '|') return "__a124";
    else if (c == '~') return "__a126";
    else {
        std::string s(1, c);
        return s;
    }
}

Instr getInstructions(std::shared_ptr<Expression>);

Expr getExpression(std::shared_ptr<Expression> expression) {
    Expr expr;

    std::string type = expression->getType();
    if (type == "Condition") {
        std::shared_ptr<Condition> condition = std::static_pointer_cast<Condition>(expression);

        std::shared_ptr<JTernary> ternary = std::make_shared<JTernary>();
        Expr co = getExpression(condition->condition);
        expr.append(co);
        ternary->condition = co.expression;
        Expr ex = getExpression(condition->object);
        expr.append(ex);
        ternary->expression = ex.expression;
        ternary->alternative = std::make_shared<JUndefined>();

        expr.expression = ternary;
    } else if (type == "ConditionAlternative") {
        std::shared_ptr<ConditionAlternative> alternativeCondition = std::static_pointer_cast<ConditionAlternative>(expression);

        std::shared_ptr<JTernary> ternary = std::make_shared<JTernary>();
        Expr co = getExpression(alternativeCondition->condition);
        expr.append(co);
        ternary->condition = co.expression;
        Expr ex = getExpression(alternativeCondition->object);
        expr.append(ex);
        ternary->expression = ex.expression;
        Expr al = getExpression(alternativeCondition->alternative);
        expr.append(al);
        ternary->alternative = al.expression;

        expr.expression = ternary;
    } else if (type == "ConditionRepeat") {
        std::shared_ptr<ConditionRepeat> conditionRepeat = std::static_pointer_cast<ConditionRepeat>(expression);

        std::shared_ptr<JWhile> jWhile = std::make_shared<JWhile>();
        Expr co = getExpression(conditionRepeat->condition);
        expr.append(co);
        jWhile->condition = co.expression;
        getInstructions(conditionRepeat->object).appendInstructionsTo(jWhile->instructions);

        expr.addInstruction(jWhile);
        expr.expression = std::make_shared<JUndefined>();
    } else if (type == "FunctionCall") {
        std::shared_ptr<FunctionCall> functionCall = std::static_pointer_cast<FunctionCall>(expression);

        if (functionCall->function->getType() == "Variable" && std::static_pointer_cast<Variable>(functionCall->function)->variableName == ";" && functionCall->object->getType() == "Tuple") {
            std::shared_ptr<Tuple> tuple = std::static_pointer_cast<Tuple>(functionCall->object);
            expr.append(getInstructions(tuple->objects[0]));
            Expr second = getExpression(tuple->objects[1]);
            expr.append(second);

            expr.expression = second.expression;
        } else {
            std::shared_ptr<JFuncEval> jFuncEval = std::make_shared<JFuncEval>();
            Expr func = getExpression(functionCall->function);
            expr.append(func);
            jFuncEval->function = func.expression;
            Expr param = getExpression(functionCall->object);
            expr.append(param);
            jFuncEval->parameters.push_back(param.expression);
            
            expr.expression = jFuncEval;
        }
    } else if (type == "FunctionDefinition") {
        std::shared_ptr<FunctionDefinition> functionDefinition = std::static_pointer_cast<FunctionDefinition>(expression);

        std::shared_ptr<JFunction> jFunction = std::make_shared<JFunction>();

        std::shared_ptr<JVariable> parameter = std::make_shared<JVariable>();
        parameter->variableName = "__parameters";
        jFunction->parameters.push_back(parameter);
        std::shared_ptr<FunctionCall> assignation = std::make_shared<FunctionCall>();
        std::shared_ptr<Variable> function = std::make_shared<Variable>();
        function->variableName = ":=";
        assignation->function = function;
        std::shared_ptr<Tuple> tuple = std::make_shared<Tuple>();
        tuple->objects.push_back(functionDefinition->parameters);
        std::shared_ptr<Variable> variable = std::make_shared<Variable>();
        variable->variableName = "__parameters";
        tuple->objects.push_back(functionDefinition->parameters);
        tuple->objects.push_back(variable);
        assignation->object = tuple;
        getExpression(assignation).appendInstructionsTo(jFunction->instructions);

        Expr body = getExpression(functionDefinition->object);
        body.appendInstructionsTo(jFunction->instructions);

        if (body.expression->getType() != "JUndefined") {
            std::shared_ptr<JReturn> jReturn = std::make_shared<JReturn>();
            jReturn->value = body.expression;
            jFunction->instructions.push_back(jReturn);
        }
        
        expr.expression = jFunction;
    } else if (type == "Tuple") {
        std::shared_ptr<Tuple> tuple = std::static_pointer_cast<Tuple>(expression);

        std::shared_ptr<JArray> array = std::make_shared<JArray>();
        for (std::shared_ptr<Expression> ex : tuple->objects) {
            Expr exp = getExpression(ex);
            expr.append(exp);
            array->objects.push_back(exp.expression);
        }
        
        expr.expression = array;
    } else if (type == "Variable") {
        std::shared_ptr<Variable> variable = std::static_pointer_cast<Variable>(expression);
        
        std::shared_ptr<JVariable> jvariable = std::make_shared<JVariable>();
        jvariable->variableName = "";
        for (size_t i = 0; i < variable->variableName.length(); i++)
            jvariable->variableName += charConvertor(variable->variableName[i]);

        expr.expression = jvariable;

        expr.addVariable(jvariable->variableName);
    }

    return expr;
}

Instr getInstructions(std::shared_ptr<Expression> expression) {
    Instr instr;

    std::string type = expression->getType();
    if (type == "Condition") {
        std::shared_ptr<Condition> condition = std::static_pointer_cast<Condition>(expression);

        std::shared_ptr<JIf> jif = std::make_shared<JIf>();
        Expr co = getExpression(condition->condition);
        instr.append(co);
        jif->condition = co.expression;
        getInstructions(condition->object).appendInstructionsTo(jif->instructions);
        instr.addInstruction(jif);
    } else if (type == "ConditionAlternative") {
        std::shared_ptr<ConditionAlternative> conditionAlternative = std::static_pointer_cast<ConditionAlternative>(expression);

        std::shared_ptr<JIfElse> jifelse = std::make_shared<JIfElse>();
        Expr co = getExpression(conditionAlternative->condition);
        instr.append(co);
        jifelse->condition = co.expression;
        getInstructions(conditionAlternative->object).appendInstructionsTo(jifelse->instructions);
        getInstructions(conditionAlternative->alternative).appendInstructionsTo(jifelse->alternative);
        instr.addInstruction(jifelse);
    } else if (type == "ConditionRepeat") {
        std::shared_ptr<ConditionRepeat> conditionRepeat = std::static_pointer_cast<ConditionRepeat>(expression);

        std::shared_ptr<JWhile> jwhile = std::make_shared<JWhile>();
        Expr co = getExpression(conditionRepeat->condition);
        instr.append(co);
        jwhile->condition = co.expression;
        getInstructions(conditionRepeat->object).appendInstructionsTo(jwhile->instructions);
        instr.addInstruction(jwhile);
    } else {
        Expr expr = getExpression(expression);
        instr.append(expr);
        instr.addInstruction(expr.expression);
    }

    return instr;
}

std::string toJavaScript(std::shared_ptr<JInstruction> tree) {
    std::string code = "";
    std::string type = tree->getType();

    if (type == "JArray") {
        std::shared_ptr<JArray> jArray = std::static_pointer_cast<JArray>(tree);
        code += "[";
        bool first = true;
        for (std::shared_ptr<JExpression> expression : jArray->objects) {
            if (!first) code += ", ";
            first = false;
            code += toJavaScript(expression);
        }
        code += "]";
    } else if (type == "JAssignment") {
        std::shared_ptr<JAssignment> jAssignment = std::static_pointer_cast<JAssignment>(tree);
        if (jAssignment) code += "var ";
        code += jAssignment->variable->variableName;
        code += " = ";
        code += toJavaScript(jAssignment->value);
    } else if (type == "JFuncEval") {
        std::shared_ptr<JFuncEval> jFuncEval = std::static_pointer_cast<JFuncEval>(tree);
        code += "(";
        code += toJavaScript(jFuncEval->function) + ".value";
        code += ")(";
        bool first = true;
        for (std::shared_ptr<JExpression> expression : jFuncEval->parameters) {
            if (!first) code += ", ";
            first = false;
            code += toJavaScript(expression);
        }
        code += ")";
    } else if (type == "JFunction") {
        std::shared_ptr<JFunction> jFunction = std::static_pointer_cast<JFunction>(tree);
        code += "function(";
        bool first = true;
        for (std::shared_ptr<JExpression> expression : jFunction->parameters) {
            if (!first) {
                code += ", ";
                first = false;
            }
            code += toJavaScript(expression);
        }
        code += ") {\n";
        for (std::shared_ptr<JInstruction> instruction : jFunction->instructions)
            code += toJavaScript(instruction) + ";\n";
        code += "}\n";
    } else if (type == "JIf") {
        std::shared_ptr<JIf> jIf = std::static_pointer_cast<JIf>(tree);
        code += "if (";
        code += toJavaScript(jIf->condition);
        code += ") {\n";
        for (std::shared_ptr<JInstruction> instruction : jIf->instructions)
            code += toJavaScript(instruction) + ";\n";
        code += "}\n";
    } else if (type == "JIfElse") {
        std::shared_ptr<JIfElse> jIfElse = std::static_pointer_cast<JIfElse>(tree);
        code += "if (";
        code += toJavaScript(jIfElse->condition);
        code += ") {\n";
        for (std::shared_ptr<JInstruction> instruction : jIfElse->instructions)
            code += toJavaScript(instruction) + ";\n";
        code += "} else {\n";
        for (std::shared_ptr<JInstruction> instruction : jIfElse->alternative)
            code += toJavaScript(instruction) + ";\n";
        code += "}\n";
    } else if (type == "JReturn") {
        std::shared_ptr<JReturn> jReturn = std::static_pointer_cast<JReturn>(tree);
        code += "return ";
        code += toJavaScript(jReturn->value);
    } else if (type == "JTernary") {
        std::shared_ptr<JTernary> jTernary = std::static_pointer_cast<JTernary>(tree);
        code += toJavaScript(jTernary->condition);
        code += " ? ";
        code += toJavaScript(jTernary->expression);
        code += " : ";
        code += toJavaScript(jTernary->alternative);
    } else if (type == "JUndefined") {
        code += "undefined";
    } else if (type == "JVariable") {
        std::shared_ptr<JVariable> jVariable = std::static_pointer_cast<JVariable>(tree);
        code +=  jVariable->variableName;
    } else if (type == "JWhile") {
        std::shared_ptr<JWhile> jWhile = std::static_pointer_cast<JWhile>(tree);
        code += "while (";
        code += toJavaScript(jWhile->condition);
        code += ") {\n";
        for (std::shared_ptr<JInstruction> instruction : jWhile->instructions)
            code += toJavaScript(instruction) + ";\n";
        code += "}\n";
    }

    return code;
}

std::string getJavaScript(std::shared_ptr<Expression> expression) {
    std::string code = "";
    Instr instr = getInstructions(expression);
    for (std::shared_ptr<JInstruction> instruction : instr.instructions)
        code += toJavaScript(instruction) + ";\n";
    return code;
}