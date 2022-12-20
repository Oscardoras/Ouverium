#ifndef __COMPILER_C_STRUCTURES_HPP__
#define __COMPILER_C_STRUCTURES_HPP__

#include <memory>
#include <string>
#include <vector>


namespace CTranslator {

    namespace Structures {

        struct Expression {};
        struct RValue: public Expression {};
        struct Instruction {};


        struct If: public Instruction {
            std::shared_ptr<Expression> condition;
            std::vector<std::shared_ptr<Instruction>> body;
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
            std::shared_ptr<RValue> r_value;
            std::shared_ptr<Expression> value;
        };

        struct VariableCall: public RValue {
            std::string name;
        };

        struct FunctionCall: public Expression, public Instruction {
            std::shared_ptr<Expression> function;
            std::vector<std::shared_ptr<Expression>> parameters;
        };

        struct FunctionDefinition: public Expression {
            std::string name;
        };

        struct Property: public RValue {
            std::shared_ptr<Expression> object;
            std::string name;
            bool pointer;
        };

        struct List: public Expression {
            std::vector<std::shared_ptr<Expression>> objects;
        };

        struct Array: public Instruction {
            std::string type;
            std::string name;
            std::shared_ptr<List> list;
        };

    }

}


#endif
