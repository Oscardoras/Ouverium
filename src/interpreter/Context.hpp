#ifndef __INTERPRETER_CONTEXT_HPP__
#define __INTERPRETER_CONTEXT_HPP__

#include <filesystem>
#include <map>
#include <set>

#include "Reference.hpp"


namespace Interpreter {

    class Context;
    class GlobalContext;
    class FunctionContext;

    class Context {

    protected:

        std::map<std::string, IndirectReference> symbols;

    public:

        std::shared_ptr<Parser::Expression> caller;

        Context(std::shared_ptr<Parser::Expression> caller);
        Context(Context const&) = delete;
        Context(Context&&) = delete;

        Context& operator=(Context const&) = delete;
        Context& operator=(Context&&) = delete;

        virtual Context& get_parent() = 0;
        virtual GlobalContext& get_global() = 0;
        virtual unsigned get_recurion_level() = 0;

        [[nodiscard]] std::set<std::string> get_symbols() const;
        [[nodiscard]] bool has_symbol(std::string const& symbol) const;
        void add_symbol(std::string const& symbol, IndirectReference const& indirect_reference);
        void add_symbol(std::string const& symbol, Data const& data);
        IndirectReference operator[](std::string const& symbol);
        [[nodiscard]] auto begin() const { return symbols.begin(); }
        [[nodiscard]] auto end() const { return symbols.end(); }

        virtual ~Context();

    };

    class GlobalContext : public Context {

    public:

        Data system;

        std::map<std::filesystem::path, std::shared_ptr<Parser::Expression>> sources;
        unsigned recursion_limit = 100;

        GlobalContext(std::shared_ptr<Parser::Expression> expression);

        GlobalContext& get_global() override {
            return *this;
        }

        Context& get_parent() override {
            return *this;
        }

        unsigned get_recurion_level() override {
            return 0;
        }

    };

    class FunctionContext : public Context {

    protected:

        Context& parent;
        unsigned recursion_level;

    public:

        FunctionContext(Context& parent, std::shared_ptr<Parser::Expression> caller);

        GlobalContext& get_global() override {
            return parent.get_global();
        }

        Context& get_parent() override {
            return parent;
        }

        unsigned get_recurion_level() override {
            return recursion_level;
        }

    };

}


#endif
