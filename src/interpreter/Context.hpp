#ifndef INTERPRETER_CONTEXT_HPP_
#define INTERPRETER_CONTEXT_HPP_

#include <map>
#include <list>
#include <string>

#include "Reference.hpp"

#include "../parser/Position.hpp"

struct GlobalContext;


struct Context {

    std::shared_ptr<Position> position;
    std::map<std::string, Reference> symbols;

    virtual GlobalContext* getGlobal() = 0;
    virtual Context* getParent() = 0;

    Object* newObject();
    Object* newObject(Object const& object);
    Object* newObject(void* ptr);
    Object* newObject(bool b);
    Object* newObject(long i);
    Object* newObject(double f);
    Object* newObject(char c);
    Object* newObject(size_t tuple_size);
    Object* newObject(std::string const& str);

    void addSymbol(std::string const& symbol, Reference const& reference);
    Reference getSymbol(std::string const& symbol);
    bool hasSymbol(std::string const& symbol) const;

};

struct GlobalContext: public Context {

    std::list<std::string> files;
    std::list<Object> objects;
    std::list<Object*> references;
    std::list<void*> cpointers;

    virtual GlobalContext* getGlobal();
    virtual Context* getParent();

    ~GlobalContext();

};

struct FunctionContext: public Context {

    Context* parent;

    FunctionContext(Context & parent, std::shared_ptr<Position> position);

    virtual GlobalContext* getGlobal();
    virtual Context* getParent();

};


#endif