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
        std::map<std::string, Reference> symbols;

        virtual GlobalContext& get_global() = 0;
        virtual Context& get_parent() = 0;

        Object* new_object();
        Object* new_object(Object const& object);
        Object* new_object(void* ptr);
        Object* new_object(bool b);
        Object* new_object(long i);
        Object* new_object(double f);
        Object* new_object(char c);
        Object* new_object(size_t tuple_size);
        Object* new_object(std::string const& str);

        bool has_symbol(std::string const& symbol) const;
        void add_symbol(std::string const& symbol, Reference const& reference);
        Reference get_symbol(std::string const& symbol);

    };

    struct GlobalContext: public Context {

        std::map<std::string, std::shared_ptr<Expression>> files;
        std::list<Object> objects;
        std::list<Object*> references;
        std::list<void*> c_pointers;

        GlobalContext();

        virtual GlobalContext& get_global();
        virtual Context& get_parent();

        ~GlobalContext();

    };

    struct FunctionContext: public Context {

        std::reference_wrapper<Context> parent;

        FunctionContext(Context & parent, std::shared_ptr<Parser::Position> position);

        virtual GlobalContext& get_global();
        virtual Context& get_parent();

    };

}


#endif
