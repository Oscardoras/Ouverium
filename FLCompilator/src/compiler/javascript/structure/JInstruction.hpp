#ifndef COMPILER_JAVASCRIPT_STRUCTURE_JINSTRUCTION_HPP_
#define COMPILER_JAVASCRIPT_STRUCTURE_JINSTRUCTION_HPP_

#include <string>
#include <memory>


class JInstruction {

public:

    virtual std::string getType() const = 0;

};


#endif