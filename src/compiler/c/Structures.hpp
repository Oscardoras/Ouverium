#ifndef __COMPILER_C_STRUCTURES_HPP__
#define __COMPILER_C_STRUCTURES_HPP__

#include <memory>
#include <string>
#include <vector>

#include "../Analyzer.hpp"


namespace CTranslator {

    namespace Structures {

        struct Expression {};
        struct LValue: public Expression {};
        struct Instruction {};

        struct Type {
            std::string name;
            // TODO: Data
        };


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
            std::reference_wrapper<Type> type;
            std::string name;
        };

        struct Affectation: public Expression, public Instruction {
            std::shared_ptr<LValue> lvalue;
            std::shared_ptr<Expression> value;
        };

        struct VariableCall: public LValue {
            std::string name;
        };

        struct FunctionCall: public Expression, public Instruction {
            std::shared_ptr<Expression> function;
            std::vector<std::shared_ptr<Expression>> parameters;
        };

        struct Property: public LValue {
            std::shared_ptr<Expression> object;
            std::string name;
            bool pointer;
        };

        struct List: public Expression {
            std::vector<std::shared_ptr<Expression>> objects;
        };

        struct Array: public Instruction {
            std::reference_wrapper<Type> type;
            std::string name;
            std::shared_ptr<List> list;
        };


        struct FunctionDefinition {
            std::reference_wrapper<Type> type;
            std::string name;
            std::vector<Declaration> parameters;
            std::shared_ptr<Expression> body;
        };

    }

}


#endif
