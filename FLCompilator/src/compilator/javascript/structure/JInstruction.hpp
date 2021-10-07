#ifndef COMPILATOR_JAVASCRIPT_STRUCTURE_JINSTRUCTION_HPP_
#define COMPILATOR_JAVASCRIPT_STRUCTURE_JINSTRUCTION_HPP_

#include <string>
#include <memory>


class JInstruction {

public:

    virtual std::string getType() const = 0;

};


#endif