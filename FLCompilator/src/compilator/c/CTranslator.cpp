#include "Tree.cpp"


struct CVariable {
    std::string name;
    std::vector<std::string> types;
};

struct CType {
    std::string name;
    std::vector<CVariable> variables;
};

struct CFunction {
    std::string name;
    std::string code;
};

struct CGlobal {
    std::vector<CType> types;
    std::vector<CFunction> functions;
};

struct CCode {
    std::string code;
    std::vector<std::string> types;
};

CCode toC(std::shared_ptr<Expression> tree, std::shared_ptr<CGlobal> global, std::vector<std::shared_ptr<CVariable>> definedVariables) {
    std::string code;
    std::string type = tree->getType();

    if (type == "Definition") {
        std::shared_ptr<Definition> definition = std::static_pointer_cast<Definition>(tree);
        CCode ccode = toC(definition->object, global, definedVariables);
        for (Variable & variable : definition->variables) {
            std::shared_ptr<CVariable> definedVariable = nullptr;
            for (std::shared_ptr<CVariable> cVariable : definedVariables) {
                if (variable.variableName == cVariable->name) {
                    definedVariable = cVariable;
                    break;
                }
            }
            if (definedVariable == nullptr) {
                std::shared_ptr<CVariable> cVariable = std::make_shared<CVariable>();
                cVariable->name = variable.variableName;
                definedVariables.push_back(cVariable);
                definedVariable = cVariable;
            }
            definedVariable->types = ccode.types;
            if (definition->object->getType() == "FunctionDefinition") {
                CFunction func = { .name = variable.variableName, .code = ccode.code };
                global->functions.push_back(func);
            }
        }
    } else if (type == "Condition") {
        std::shared_ptr<Condition> condition = std::static_pointer_cast<Condition>(tree);
        code += "if ( " + toC(condition->condition) + " ) {\n" + toC(condition->object) + "\n}\n";
    } else if (type == "AlternativeCondition") {
        std::shared_ptr<AlternativeCondition> alternativeCondition = std::static_pointer_cast<AlternativeCondition>(tree);
        code += "if ( " + toC(alternativeCondition->condition) + " ) {\n"
            + toC(alternativeCondition->object)
        + "\n} else {\n"
            + toC(alternativeCondition->object)
        + "\n}\n";
    } else if (type == "LoopCondition") {
        std::shared_ptr<LoopCondition> loopCondition = std::static_pointer_cast<LoopCondition>(tree);
        code += "while ( " + toC(loopCondition->condition) + " ) {\n"
            + toC(loopCondition->object)
        + "\n}\n";
    }

    return code;
}