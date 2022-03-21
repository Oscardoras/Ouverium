#ifndef INTERPRETER_CONTEXT_HPP_
#define INTERPRETER_CONTEXT_HPP_

#include <map>
#include <list>
#include <string>

#include "Reference.hpp"

struct GlobalContext;


struct Context {

    std::map<std::string, Reference> symbols;

    virtual GlobalContext* getGlobal() = 0;
    virtual Context* getParent() = 0;

    Object* addObject(Object* object);
    void collect(Object* current);

    void addSymbol(std::string const& symbol, Reference const& reference);
    virtual Reference getSymbol(std::string const& symbol, bool const& create = true) = 0;
    bool hasSymbol(std::string const& symbol) const;

};

struct GlobalContext: public Context {

    std::list<Object*> objects;

    virtual GlobalContext* getGlobal();
    virtual Context* getParent();

    virtual Reference getSymbol(std::string const& symbol, bool const& create = true);

    ~GlobalContext();

};

struct FunctionContext: public Context {

    Context* parent;

    std::shared_ptr<Expression> parameters;
    std::shared_ptr<Expression> arguments;

    FunctionContext(Context const& parent);
    FunctionContext(Context const& parent, std::shared_ptr<Expression> parameters, std::shared_ptr<Expression> arguments);

    virtual GlobalContext* getGlobal();
    virtual Context* getParent();

    virtual Reference getSymbol(std::string const& symbol, bool const& create = true);

};


#endif