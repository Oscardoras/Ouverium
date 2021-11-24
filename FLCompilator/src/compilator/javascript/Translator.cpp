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

struct Expr {
    std::vector<std::shared_ptr<JInstruction>> instructions;
    std::shared_ptr<JExpression> expression;
};

/*
Expr translateFunction(std::shared_ptr<FunctionCall> functionCall) {
    Expr expr;
    Expr function = getExpression(functionCall->function);
    //Expr parameters = getExpression(functionCall->object);
    append(expr.instructions, function.instructions);
    //append(expr.instructions, parameters.instructions);
    std::string type = function.expression->getType();
    
    if (type == "JVariable") {
        std::string variableName = std::static_pointer_cast<JVariable>(function.expression)->variableName;

        if (variableName == ":=") {
            std::shared_ptr<JAssignment> jAssignment = std::make_shared<JAssignment>();
            if (functionCall->object->getType() == "Tuple") {
                std::shared_ptr<JArray> jArray = std::make_shared<JArray>();

                jAssignment->value = parameters.expression;
            }
            
            if (definition->variables->getType() == "Variable") {
                assignment->variable = getJVariable(std::static_pointer_cast<Variable>(definition->variables));
                instructions.push_back(assignment);
            } else {
                assignment->variable = std::make_shared<JVariable>();
                assignment->variable->variableName = "temp";
                instructions.push_back(assignment);

                append(instructions, assign(std::static_pointer_cast<Tuple>(definition->variables), assignment->variable));
            }
        }

        if (variable->variableName == "console" && variable->next != nullptr && variable->next->variableName == "print") {
            Expr expr;
            std::shared_ptr<JVariable> var1 = std::make_shared<JVariable>();
            var1->variableName = variable->variableName;
            std::shared_ptr<JVariable> var2 = std::make_shared<JVariable>();
            var2->variableName = "info";
            var1->next = var2;
            expr.expression = var1;
        }
    }

    std::shared_ptr<JFuncEval> jfunceval = std::make_shared<JFuncEval>();
    Expr func = getExpression(functionCall->function);
    jfunceval->function = func.expression;
    append(instructions, func.instructions);

    return getExpression(function);
}

std::vector<std::shared_ptr<JInstruction>> assign(std::shared_ptr<Tuple> variables, std::shared_ptr<JVariable> temp) {
    std::vector<std::shared_ptr<JInstruction>> assignments;
    int i = 0;
    for (std::shared_ptr<Expression> expr : variables->objects) {
        std::shared_ptr<JVariable> tab = std::make_shared<JVariable>(*temp.get());
        tab->index.push_back(i);
        if (expr->getType() == "VariableCall") {
            std::shared_ptr<JAssignment> assignment = std::make_shared<JAssignment>();
            assignment->variable = getJVariable(std::static_pointer_cast<VariableCall>(expr));

            assignments.push_back(assignment);
        } else {
            append(assignments, assign(std::static_pointer_cast<Tuple>(expr), tab));
        }
        i++;
    }
    return assignments;
}

std::vector<std::shared_ptr<JInstruction>> del(std::shared_ptr<Expression> variables) {
    std::vector<std::shared_ptr<JInstruction>> instructions;

    if (variables->getType() == "Variable") {
        std::shared_ptr<JDelete> dele = std::make_shared<JDelete>();
        dele->variable = getJVariable(std::static_pointer_cast<Variable>(variables));
        instructions.push_back(dele);
    } else {
        for (std::shared_ptr<Expression> ex : std::static_pointer_cast<Tuple>(variables)->objects)
            append(instructions, del(ex));
    }

    return instructions;
}
*/

std::string charConvertor(char c) {
    if (c == '!') return "a65";
    else if (c == '#') return "__a67";
    else if (c == '$') return "__a68";
    else if (c == '%') return "__a69";
    else if (c == '&') return "__a70";
    else if (c == '*') return "__a74";
    else if (c == '+') return "__a75";
    else if (c == '-') return "__a77";
    else if (c == '.') return "__a78";
    else if (c == '/') return "__a79";
    else if (c == ':') return "__a90";
    else if (c == ';') return "__a91";
    else if (c == '<') return "__a92";
    else if (c == '=') return "__a93";
    else if (c == '>') return "__a94";
    else if (c == '?') return "__a95";
    else if (c == '@') return "__a96";
    else if (c == '~') return "__a158";
    else return ""+c;
}

Expr getExpression(std::shared_ptr<Expression> expression) {
    Expr expr;

    std::string type = expression->getType();
    if (type == "Condition") {
        std::shared_ptr<Condition> condition = std::static_pointer_cast<Condition>(expression);

        std::shared_ptr<JTernary> ternary = std::make_shared<JTernary>();
        Expr co = getExpression(condition->condition);
        ternary->condition = co.expression;
        append(expr.instructions, co.instructions);
        Expr ex = getExpression(condition->object);
        ternary->expression = ex.expression;
        append(expr.instructions, ex.instructions);
        ternary->alternative = std::make_shared<JUndefined>();

        expr.expression = ternary;
    } else if (type == "ConditionAlternative") {
        std::shared_ptr<ConditionAlternative> alternativeCondition = std::static_pointer_cast<ConditionAlternative>(expression);

        std::shared_ptr<JTernary> ternary = std::make_shared<JTernary>();
        Expr co = getExpression(alternativeCondition->condition);
        ternary->condition = co.expression;
        append(expr.instructions, co.instructions);
        Expr ex = getExpression(alternativeCondition->object);
        ternary->expression = ex.expression;
        append(expr.instructions, ex.instructions);
        Expr al = getExpression(alternativeCondition->alternative);
        ternary->alternative = al.expression;
        append(expr.instructions, al.instructions);

        expr.expression = ternary;
    } else if (type == "ConditionRepeat") {
        std::shared_ptr<ConditionRepeat> conditionRepeat = std::static_pointer_cast<ConditionRepeat>(expression);

        std::shared_ptr<JWhile> jWhile = std::make_shared<JWhile>();
        Expr co = getExpression(conditionRepeat->condition);
        jWhile->condition = co.expression;
        append(expr.instructions, co.instructions);
        jWhile->instructions = getInstructions(conditionRepeat->object);

        expr.instructions.push_back(jWhile);
        expr.expression = nullptr;
    } else if (type == "FunctionCall") {
        std::shared_ptr<FunctionCall> functionCall = std::static_pointer_cast<FunctionCall>(expression);

        std::shared_ptr<JFuncEval> jFuncEval = std::make_shared<JFuncEval>();
        Expr func = getExpression(functionCall->function);
        append(expr.instructions, func.instructions);
        jFuncEval->function = func.expression;
        Expr param = getExpression(functionCall->object);
        append(expr.instructions, param.instructions);
        jFuncEval->parameters.push_back(param.expression);
        
        expr.expression = jFuncEval;
    } else if (type == "Tuple") {
        std::shared_ptr<Tuple> tuple = std::static_pointer_cast<Tuple>(expression);

        std::shared_ptr<JArray> array = std::make_shared<JArray>();
        for (std::shared_ptr<Expression> ex : tuple->objects) {
            Expr exp = getExpression(ex);
            append(expr.instructions, exp.instructions);
            array->objects.push_back(exp.expression);
        }
        
        expr.expression = array;
    } else if (type == "Variable") {
        std::shared_ptr<Variable> variable = std::static_pointer_cast<Variable>(expression);
        
        std::shared_ptr<JVariable> jvariable = std::make_shared<JVariable>();
        jvariable->variableName = "";
        for (int i = 0; i < variable->variableName.length(); i++)
            jvariable->variableName += charConvertor(variable->variableName[i]);

        expr.expression = jvariable;
    }

    return expr;
}

std::vector<std::shared_ptr<JInstruction>> getInstructions(std::shared_ptr<Expression> expression) {
    std::vector<std::shared_ptr<JInstruction>> instructions;

    std::string type = expression->getType();
    if (type == "Condition") {
        std::shared_ptr<Condition> condition = std::static_pointer_cast<Condition>(expression);

        std::shared_ptr<JIf> jif = std::make_shared<JIf>();
        Expr co = getExpression(condition->condition);
        jif->condition = co.expression;
        append(instructions, co.instructions);
        jif->instructions = getInstructions(condition->object);
    } else if (type == "ConditionAlternative") {
        std::shared_ptr<ConditionAlternative> conditionAlternative = std::static_pointer_cast<ConditionAlternative>(expression);

        std::shared_ptr<JIfElse> jifelse = std::make_shared<JIfElse>();
        Expr co = getExpression(conditionAlternative->condition);
        jifelse->condition = co.expression;
        append(instructions, co.instructions);
        jifelse->instructions = getInstructions(conditionAlternative->object);
        jifelse->alternative = getInstructions(conditionAlternative->alternative);
    } else if (type == "ConditionRepeat") {
        std::shared_ptr<ConditionRepeat> conditionRepeat = std::static_pointer_cast<ConditionRepeat>(expression);

        std::shared_ptr<JWhile> jwhile = std::make_shared<JWhile>();
        Expr co = getExpression(conditionRepeat->condition);
        jwhile->condition = co.expression;
        append(instructions, co.instructions);
        jwhile->instructions = getInstructions(conditionRepeat->object);
    } else {
        Expr expr = getExpression(expression);
        append(instructions, expr.instructions);
        instructions.push_back(expr.expression);
    }

    return instructions;
}

std::string toJavaScript(std::shared_ptr<JInstruction> tree, std::vector<std::string> variables = {}) {
    std::string type = tree->getType();

    if (type == "Definition") {
        std::shared_ptr<Definition> definition = std::static_pointer_cast<Definition>(tree);
        std::string variablesType = definition->variables->getType();
        JCode obj = toJavaScript(definition->object);

        if (obj.result != "") jcode.code += obj.code;

        if (variablesType == "Tuple") {
            
        } else if (variablesType == "Variable") {
            std::shared_ptr<Variable> variable = std::static_pointer_cast<Variable>(definition->variables);
            if (!contains(variables, variable->variableName)) {
                code += "var ";
                variables.push(variable->variableName);
            }
            code += getVariable(std::static_pointer_cast<Variable>(definition->variables)) + " = " + (obj.result == "" ? obj.code : obj.result) + ";";
        }
    } else if (type == "FunctionDefinition") {
        std::shared_ptr<FunctionDefinition> functiondefinition = std::static_pointer_cast<FunctionDefinition>(tree);
        std::string variablesType = functiondefinition->variables->getType();

        code += "function(";
        if (variablesType == "Variable") {
            std::shared_ptr<Variable> variable = std::static_pointer_cast<Variable>(functiondefinition->variables);
            code += variable->variableName;
        }
        code + ") {\n"
            + toJavaScript(functiondefinition->object).code
        + "}";
    } else if (type == "FunctionCall") {
        std::shared_ptr<FunctionCall> functioncall = std::static_pointer_cast<FunctionCall>(tree);
        std::string functionType = functioncall->function->getType();
        JCode obj = toJavaScript(functioncall->object);

        if (obj.result != "") code += obj.code;

        if (functionType == "Variable") {
            code += getFunction(std::static_pointer_cast<Variable>(functioncall)) + "(" + (obj.result == "" ? obj.code : obj.result) + ")";
        }
    } else if (type == "Tuple") {
        std::shared_ptr<Tuple> tuple = std::static_pointer_cast<Tuple>(tree);
        code += "[";
        for (std::shared_ptr<Expression> & expression : tuple->objects) {

        }
        code += "]";
    } else if (type == "Condition") {
        std::shared_ptr<Condition> condition = std::static_pointer_cast<Condition>(tree);
        JCode cond = toJavaScript(condition->condition);

        if (cond.result != "") jcode.code += cond.code;
        jcode.code += "if ( " + (cond.result == "" ? cond.code : cond.result) + " ) {\n";
            JCode obj = toJavaScript(condition->object);
            jcode.code += obj.code;
            jcode.result = obj.result;
        jcode.code += "\n}\n";
    } else if (type == "AlternativeCondition") {
        std::shared_ptr<AlternativeCondition> alternativeCondition = std::static_pointer_cast<AlternativeCondition>(tree);
        JCode cond = toJavaScript(alternativeCondition->condition);

        if (cond.result != "") jcode.code += cond.code;
        jcode.code += "var ";
        jcode.code += "if ( " + (cond.result == "" ? cond.code : cond.result) + " ) {\n";
            JCode obj = toJavaScript(alternativeCondition->object);
            jcode.code += obj.code;
            jcode.result = obj.result;
        jcode.code += "\n} else {\n";
            JCode obj = toJavaScript(alternativeCondition->alternative);
            jcode.code += obj.code;
            jcode.result = obj.result;
        jcode.code += "\n}\n";
    } else if (type == "LoopCondition") {
        std::shared_ptr<LoopCondition> loopCondition = std::static_pointer_cast<LoopCondition>(tree);
        code += "while ( " + toJavaScript(loopCondition->condition) + " ) {\n"
            + toJavaScript(loopCondition->object)
        + "\n}\n";
    }

    return code;
}