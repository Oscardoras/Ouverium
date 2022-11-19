#ifndef __COMPILER_C_STRUCTURES_HPP__
#define __COMPILER_C_STRUCTURES_HPP__

#include <memory>
#include <string>
#include <vector>


namespace CStructures {

    struct Expression {};
    struct Instruction {};


    struct If: public Instruction {
        std::shared_ptr<Expression> condition;
        std::vector<std::shared_ptr<Instruction>> body;
    };

    struct IfElse: public If {
        std::vector<std::shared_ptr<Instruction>> alternative;
    };

    struct While: public Instruction {
        std::shared_ptr<Expression> condition;
        std::vector<std::shared_ptr<Instruction>> body;
    };

    struct Declaration: public Instruction {
        std::string type;
        std::string name;
    };

    struct Affectation: public Expression, public Instruction {
        std::string name;
        std::shared_ptr<Expression> value;
    };

    struct VariableCall: public Expression {
        std::string name;
    };

    struct FunctionCall: public Expression, public Instruction {
        std::shared_ptr<Expression> function;
        std::vector<std::shared_ptr<Expression>> parameters;
    };

    struct FunctionDefinition: public Expression {
        std::string name;
    };

    struct Property: public Expression {
        std::shared_ptr<Expression> object;
        std::string name;
        bool pointer;
    };

    struct List: public Expression {
        std::vector<std::shared_ptr<Expression>> objects;
    };

}


#endif
