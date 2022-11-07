#ifndef __COMPILER_C_STRUCTURES_HPP__
#define __COMPILER_C_STRUCTURES_HPP__


namespace CStructures {

    struct Expression {

    };

    struct Instruction {

    };


    struct If: public Instruction {

    };

    struct IfElse: public If {

    };

    struct While: public Instruction {

    };

    struct Declaration: public Instruction {

    };

    struct Affectation: public Expression, public Instruction {

    };

    struct FunctionCall: public Expression, public Instruction {

    };

}


#endif
