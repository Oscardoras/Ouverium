#pragma once

// IWYU pragma: private; include "Interpreter.hpp"

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>

#include <ouverium/parser/Expressions.hpp>

#include "Data.hpp"
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

        std::map<std::shared_ptr<Parser::Expression>, Data> tuples;

        Context(std::shared_ptr<Parser::Expression> caller);
        Context(Context const&) = delete;
        Context(Context&&) = delete;

        Context& operator=(Context const&) = delete;
        Context& operator=(Context&&) = delete;

        [[nodiscard]] virtual Context& get_parent() = 0;
        [[nodiscard]] virtual GlobalContext& get_global() = 0;
        [[nodiscard]] virtual unsigned get_recurion_level() const = 0;

        [[nodiscard]] std::set<std::string> get_symbols() const;
        [[nodiscard]] bool has_symbol(std::string const& symbol) const;
        void add_symbol(std::string symbol, IndirectReference indirect_reference);
        void add_symbol(std::string symbol, Data data);
        IndirectReference operator[](std::string const& symbol);
        [[nodiscard]] auto begin() const { return symbols.begin(); }
        [[nodiscard]] auto end() const { return symbols.end(); }

        virtual ~Context() = default;

    };

    class GlobalContext : public Context {

    public:

        Data system;

        std::map<std::filesystem::path, std::shared_ptr<Parser::Expression>> sources;
        unsigned recursion_limit = 100;

        GlobalContext(std::shared_ptr<Parser::Expression> expression);

        [[nodiscard]] GlobalContext& get_global() override {
            return *this;
        }

        [[nodiscard]] Context& get_parent() override {
            return *this;
        }

        [[nodiscard]] unsigned get_recurion_level() const override {
            return 0;
        }

    };

    class FunctionContext : public Context {

    protected:

        Context& parent;
        unsigned recursion_level;

    public:

        FunctionContext(Context& parent, std::shared_ptr<Parser::Expression> caller);

        [[nodiscard]] GlobalContext& get_global() override {
            return parent.get_global();
        }

        [[nodiscard]] Context& get_parent() override {
            return parent;
        }

        [[nodiscard]] unsigned get_recurion_level() const override {
            return recursion_level;
        }

    };

}
