#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include "Context.hpp"


namespace Interpreter {

    class Exception {

    public:

        Reference reference;
        std::vector<std::string> positions;

        Exception(Context& context, Reference const& reference, std::shared_ptr<Parser::Expression> expression);
        Exception(Context& context, std::string const& message, std::shared_ptr<Parser::Expression> expression);

        void print_stack_trace(Context& context) const;

    };
    class FunctionArgumentsError {};

    using Arguments = std::variant<std::shared_ptr<Parser::Expression>, Reference>;

    Reference call_function(Context& context, std::shared_ptr<Parser::Expression> expression, Reference const& func, Arguments const& arguments);
    Reference execute(Context& context, std::shared_ptr<Parser::Expression> expression);

    Reference set(Context& context, Reference const& var, Reference const& data);
    std::string string_from(Context& context, Reference const& data);


    class SystemExpression : public Parser::Expression {
    public:
        SystemExpression(std::string const& position) {
            Parser::Expression::position = position;
        }
        std::set<std::string> get_symbols() const override {
            return {};
        }
        std::set<std::string> compute_symbols(std::set<std::string>&) override {
            return {};
        }
        std::string to_string(unsigned = 0) const override {
            return {};
        }
    };

    inline Object* get_object(GlobalContext& context, IndirectReference const& reference) {
        if (auto symbol_reference = std::get_if<SymbolReference>(&reference)) {
            auto& data = symbol_reference->get();
            if (data == Data{})
                data = context.new_object();
            return data.get<Object*>();
        } else if (auto property_reference = std::get_if<PropertyReference>(&reference)) {
            auto& data = property_reference->parent.get<Object*>()->properties[property_reference->name];
            if (data == Data{})
                data = context.new_object();
            return data.get<Object*>();
        } else if (auto array_reference = std::get_if<ArrayReference>(&reference)) {
            auto& data = array_reference->array.get<Object*>()->array[array_reference->i];
            if (data == Data{})
                data = context.new_object();
            return data.get<Object*>();
        } else return nullptr;
    }

    inline Object* get_object(GlobalContext& context, Data& data) {
        if (data == Data{})
            data = context.new_object();
        return data.get<Object*>();
    }

}

#endif
