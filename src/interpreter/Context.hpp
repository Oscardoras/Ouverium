#ifndef __INTERPRETER_CONTEXT_HPP__
#define __INTERPRETER_CONTEXT_HPP__

#include <map>
#include <set>
#include <string>

#include "Reference.hpp"

#include "../parser/Parser.hpp"


namespace Interpreter {

    class GlobalContext;

    class Context: public Parser::Context {

    protected:

        std::map<std::string, IndirectReference> symbols;

    public:

        Context(std::shared_ptr<Parser::Position> position = nullptr):
            Parser::Context(position) {}

        virtual GlobalContext & get_global() = 0;

        Object* new_object();
        Object* new_object(Object && object);
        Object* new_object(std::string const& str);

        Data & new_reference(Data const& data = Data{});

        bool has_symbol(std::string const& symbol) const;
        IndirectReference add_symbol(std::string const& symbol, Reference const& reference);
        IndirectReference operator[](std::string const& symbol);
        auto begin() {return symbols.begin();}
        auto end() {return symbols.end();}

        auto & get_function(std::string const& symbol) {
            return static_cast<Data &>((*this)[symbol]).get<Object*>(*this)->functions;
        }

    };

    class GlobalContext: public Context {

    protected:

        std::list<Object> objects;
        std::list<Data> references;

    public:

        std::map<std::string, std::shared_ptr<Parser::Expression>> sources;

        GlobalContext();

        virtual GlobalContext & get_global() override;
        virtual Context & get_parent() override;

        ~GlobalContext();

        friend Object* Context::new_object();
        friend Object* Context::new_object(Object && object);
        friend Object* Context::new_object(std::string const& str);
        friend Data & Context::new_reference(Data const& data);

    };

    class FunctionContext: public Context {

    protected:

        Context & parent;

    public:

        FunctionContext(Context & parent, std::shared_ptr<Parser::Position> position = nullptr):
            Context(position), parent(parent) {}

        virtual GlobalContext & get_global() override;
        virtual Context & get_parent() override;

    };

}


#endif
