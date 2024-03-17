#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include "Context.hpp"


namespace Interpreter {

    namespace SystemFunctions {

        void init(GlobalContext& context);

    }

    class Exception {

    public:

        Reference reference;
        std::vector<Parser::Position> positions;

        Exception(Context& context, std::shared_ptr<Parser::Expression> thrower, Reference const& reference);
        Exception(Context& context, std::shared_ptr<Parser::Expression> thrower, std::string const& message);

        void print_stack_trace(Context& context) const;

    };
    class FunctionArgumentsError {};

    using Arguments = std::variant<std::shared_ptr<Parser::Expression>, Reference>;

    Reference call_function(Context& context, std::shared_ptr<Parser::Expression> caller, Reference const& func, Arguments const& arguments);
    Reference execute(Context& context, std::shared_ptr<Parser::Expression> expression);

    Reference set(Context& context, Reference const& var, Reference const& data);
    std::string string_from(Context& context, Reference const& data);


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
