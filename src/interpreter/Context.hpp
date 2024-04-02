#ifndef __INTERPRETER_CONTEXT_HPP__
#define __INTERPRETER_CONTEXT_HPP__

#include <filesystem>
#include <mutex>
#include <set>

#include <boost/asio.hpp>

#include "Object.hpp"


namespace Interpreter {

    class Context;
    class GlobalContext;
    class FunctionContext;

    class Context {

    protected:

        std::map<std::string, IndirectReference> symbols;

    public:

        std::shared_ptr<Parser::Expression> caller;

        Context(std::shared_ptr<Parser::Expression> caller) :
            caller(caller) {}

        virtual Context& get_parent() = 0;
        virtual GlobalContext& get_global() = 0;
        virtual unsigned get_recurion_level() = 0;

        ObjectPtr new_object(Object const& object = {});
        Data& new_reference(Data const& data = {});

        std::set<std::string> get_symbols() const;
        bool has_symbol(std::string const& symbol) const;
        void add_symbol(std::string const& symbol, IndirectReference const& indirect_reference);
        void add_symbol(std::string const& symbol, Data const& data);
        IndirectReference operator[](std::string const& symbol);
        auto begin() const { return symbols.begin(); }
        auto end() const { return symbols.end(); }

        virtual ~Context() = default;

    };

    class GlobalContext : public Context {

    protected:

        std::mutex mutex;
        std::list<ObjectPtr::type> objects;
        unsigned long last_size = 1024;
        std::list<Data> references;
        std::set<Context*> contexts;

    public:

        ObjectPtr system;

        std::map<std::filesystem::path, std::shared_ptr<Parser::Expression>> sources;
        unsigned recursion_limit = 100;

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

        void GC_collect();

        friend Context;
        friend FunctionContext;

    };

    class FunctionContext : public Context {

    protected:

        Context& parent;
        unsigned recursion_level;

    public:

        FunctionContext(Context& parent, std::shared_ptr<Parser::Expression> caller);

        virtual GlobalContext& get_global() override {
            return parent.get_global();
        }

        virtual Context& get_parent() override {
            return parent;
        }

        virtual unsigned get_recurion_level() override {
            return recursion_level;
        }

        ~FunctionContext();

    };

}


#endif
