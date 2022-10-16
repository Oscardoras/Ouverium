#ifndef __COMPILER_JAVASCRIPT_STRUCTURE_JINSTRUCTION_HPP__
#define __COMPILER_JAVASCRIPT_STRUCTURE_JINSTRUCTION_HPP__

#include <memory>


struct JInstruction {

    enum Type {
        Array,
        Assignment,
        Delete,
        FuncEvel,
        Function,
        If,
        Else,
        Property,
        Return,
        Ternary,
        Undefined,
        Variable,
        While
    } type;

};

struct JExpression: public JInstruction {};


#endif
