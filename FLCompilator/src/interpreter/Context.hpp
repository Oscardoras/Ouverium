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

    void addReference(Object* object);

    void removeReference(Object* object);

    void finalize(Object * object);

    void free(Object* object);

    void unuse(Reference & reference);

};

struct GlobalContext: public Context {

    GlobalContext();

    virtual Reference getSymbol(std::string const& symbol, bool const& create = true);

    virtual void addSymbol(std::string const& symbol, Reference const& reference);

    ~GlobalContext();

};

struct FunctionContext: public Context {

    FunctionContext(Context const& parent);

    virtual Reference getSymbol(std::string const& symbol, bool const& create = true);

    virtual void addSymbol(std::string const& symbol, Reference const& reference);

    void freeContext();

    void freeContext(Reference & result);

private:

    int checkObject(Object* & object, Reference & result);

    void freeContext(Object* object, Reference & result);

};


#endif