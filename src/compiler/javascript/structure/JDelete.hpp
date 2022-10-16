#ifndef COMPILER_JAVASCRIPT_STRUCTURE_JDELETE_HPP_
#define COMPILER_JAVASCRIPT_STRUCTURE_JDELETE_HPP_

#include "JInstruction.hpp"
#include "JVariable.hpp"


class JDelete: public JInstruction {

public:

    std::shared_ptr<JVariable> variable;

    virtual std::string getType() const {
        return "JDelete";
    }

};


#endif