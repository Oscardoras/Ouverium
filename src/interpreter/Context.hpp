#ifndef __INTERPRETER_CONTEXT_HPP__
#define __INTERPRETER_CONTEXT_HPP__

#include <map>
#include <set>
#include <string>

#include "Reference.hpp"

#include "../parser/Position.hpp"

#include "../Expressions.hpp"


namespace Interpreter {

    struct GlobalContext;

    struct Context {

        std::shared_ptr<Parser::Position> position;
        std::map<std::string, SymbolReference> symbols;

        inline Context(std::shared_ptr<Parser::Position> position = nullptr):
        position(position) {}

        virtual GlobalContext & get_global() = 0;
        virtual Context & get_parent() = 0;

        Object* new_object();
        Object* new_object(Object const& object);
        Object* new_object(std::string const& str);

        Data & new_reference(Data data = Data{});

        bool has_symbol(std::string const& symbol) const;
        Data & add_symbol(std::string const& symbol, Reference const& reference);
        Data & operator[](std::string const& symbol);

    };

    struct GlobalContext: public Context {

        std::map<std::string, std::shared_ptr<Expression>> files;
        std::list<Object> objects;
        std::list<Data> references;
        std::list<void*> c_pointers;

        GlobalContext();

        virtual GlobalContext & get_global();
        virtual Context & get_parent();

        ~GlobalContext();

    };

    struct FunctionContext: public Context {

        std::reference_wrapper<Context> parent;

        inline FunctionContext(Context & parent, std::shared_ptr<Parser::Position> position = nullptr):
        Context(position), parent(parent) {}

        virtual GlobalContext & get_global();
        virtual Context & get_parent();

    };

}


#endif
