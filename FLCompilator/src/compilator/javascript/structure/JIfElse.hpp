#ifndef COMPILATOR_JAVASCRIPT_STRUCTURE_JIFELSE_HPP_
#define COMPILATOR_JAVASCRIPT_STRUCTURE_JIFELSE_HPP_

#include "JIf.hpp"


class JIfElse: public JIf {

public:

    std::vector<std::shared_ptr<JInstruction>> alternative;

    virtual std::string getType() const {
        return "IfElse";
    }

};


#endif