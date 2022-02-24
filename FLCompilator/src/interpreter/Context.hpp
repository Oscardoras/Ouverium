#ifndef INTERPRETER_CONTEXT_HPP_
#define INTERPRETER_CONTEXT_HPP_

#include <map>
#include <string>

#include "Reference.hpp"

struct GlobalContext;


struct Context {

    GlobalContext* global;

    std::map<std::string, Reference> symbols;

    virtual Reference getSymbol(std::string const& symbol, bool const& create = true) = 0;

    virtual void addSymbol(std::string const& symbol, Reference const& reference) = 0;

    virtual bool hasSymbol(std::string const& symbol);

};

struct GlobalContext: public Context {

    GlobalContext();

    virtual Reference getSymbol(std::string const& symbol, bool const& create = true);

    virtual void addSymbol(std::string const& symbol, Reference const& reference);

    ~GlobalContext();

};

struct FunctionContext: public Context {

    FunctionContext(Context & parent);

    virtual Reference getSymbol(std::string const& symbol, bool const& create = true);

    virtual void addSymbol(std::string const& symbol, Reference const& reference);

    void free();

    void free(Reference & result);

};


#endif