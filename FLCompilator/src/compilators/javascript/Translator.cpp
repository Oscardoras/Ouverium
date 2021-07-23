#include "../../parser/Standard.cpp"


struct JCode {
    std::string code;
    std::string result;
};

std::string getVariable(std::shared_ptr<Variable> variable) {
    std::string code = variable->variableName;
    variable = variable->next;
    while (variable != nullptr) {
        code += "." + variable->variableName;
        variable = variable->next;
    }
    return code;
}

std::string getFunction(std::shared_ptr<Expression> function) {
    std::string functionType = function->getType();

    if (functionType == "Variable") {
        std::shared_ptr<Variable> variable = std::static_pointer_cast<Variable>(function);

        if (variable->variableName == "console" && variable->next != nullptr && variable->next->variableName == "print") {
            return "console.info";
        }

        return getVariable(variable);
    }
}

JCode toJavaScript(std::shared_ptr<Expression> tree) {
    std::string code;
    std::string type = tree->getType();

    if (type == "Definition") {
        std::shared_ptr<Definition> definition = std::static_pointer_cast<Definition>(tree);
        std::string variablesType = definition->variables->getType();
        JCode jcode = toJavaScript(definition->object);

        if (jcode.result != "") code += jcode.code;

        if (variablesType == "Variable") {
            code += getVariable(std::static_pointer_cast<Variable>(definition->variables)) + " = " + (jcode.result == "" ? jcode.code : jcode.result) + ";";
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
        JCode jcode = toJavaScript(functioncall->object);

        if (jcode.result != "") code += jcode.code;

        if (functionType == "Variable") {
            code += getFunction(std::static_pointer_cast<Variable>(functioncall)) + "(" + (jcode.result == "" ? jcode.code : jcode.result) + ")";
        }
    } else if (type == "Tuple") {
        std::shared_ptr<Tuple> tuple = std::static_pointer_cast<Tuple>(tree);
        code += "[";
        for (std::shared_ptr<Expression> & expression : tuple->objects) {

        }
        code += "]";
    } else if (type == "Condition") {
        std::shared_ptr<Condition> condition = std::static_pointer_cast<Condition>(tree);
        code += "if ( " + toJavaScript(condition->condition) + " ) {\n" + toJavaScript(condition->object) + "\n}\n";
    } else if (type == "AlternativeCondition") {
        std::shared_ptr<AlternativeCondition> alternativeCondition = std::static_pointer_cast<AlternativeCondition>(tree);
        code += "if ( " + toJavaScript(alternativeCondition->condition) + " ) {\n"
            + toJavaScript(alternativeCondition->object)
        + "\n} else {\n"
            + toJavaScript(alternativeCondition->object)
        + "\n}\n";
    } else if (type == "LoopCondition") {
        std::shared_ptr<LoopCondition> loopCondition = std::static_pointer_cast<LoopCondition>(tree);
        code += "while ( " + toJavaScript(loopCondition->condition) + " ) {\n"
            + toJavaScript(loopCondition->object)
        + "\n}\n";
    }

    return code;
}