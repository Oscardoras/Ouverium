#ifndef __INTERPRETER_CONTEXT_HPP__
#define __INTERPRETER_CONTEXT_HPP__

#include <filesystem>
#include <set>

#include "Object.hpp"


namespace Interpreter {

    class GlobalContext;

    class Context {

    protected:

        std::map<std::string, IndirectReference> symbols;

    public:

        std::shared_ptr<Parser::Expression> expression;

        Context(std::shared_ptr<Parser::Expression> expression) :
            expression(expression) {}

        virtual Context& get_parent() = 0;
        virtual GlobalContext& get_global() = 0;
        virtual unsigned get_recurion_level() = 0;

        Object* new_object(Object const& object = {});
        Data& new_reference(Data const& data = {});
        void GC_collect();

        std::set<std::string> get_symbols() const;
        bool has_symbol(std::string const& symbol) const;
        void add_symbol(std::string const& symbol, Reference const& reference);
        IndirectReference operator[](std::string const& symbol);
        auto begin() const { return symbols.begin(); }
        auto end() const { return symbols.end(); }

    };

    class GlobalContext : public Context {

    protected:

        std::list<Object> objects;
        std::list<Data> references;

    public:

        std::map<std::filesystem::path, std::shared_ptr<Parser::Expression>> sources;
        Object* system;
        unsigned recursion_limit = 50;

        GlobalContext(std::shared_ptr<Parser::Expression> expression);

        virtual GlobalContext& get_global() override {
            return *this;
        }

        virtual Context& get_parent() override {
            return *this;
        }

        virtual unsigned get_recurion_level() override {
            return 0;
        }

        auto& get_function(std::string const& symbol) {
            return (*this)[symbol].to_data(*this).get<Object*>()->functions;
        }

        ~GlobalContext();

        friend Context;

    };

    class FunctionContext : public Context {

    protected:

        Context& parent;
        unsigned recursion_level;

    public:

        FunctionContext(Context& parent, std::shared_ptr<Parser::Expression> expression) :
            Context(expression), parent(parent), recursion_level(parent.get_recurion_level() + 1) {}

        virtual GlobalContext& get_global() override {
            return parent.get_global();
        }

        virtual Context& get_parent() override {
            return parent;
        }

        virtual unsigned get_recurion_level() override {
            return recursion_level;
        }

    };

}


#endif
