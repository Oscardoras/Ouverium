#ifndef __INTERPRETER_INTERPRETER_HPP__
#define __INTERPRETER_INTERPRETER_HPP__

#include "Context.hpp"


namespace Interpreter {

    class Exception {

    public:

        Reference reference;
        std::vector<std::shared_ptr<Parser::Position>> positions;

        Exception(Context& context, Reference const& reference, std::shared_ptr<Parser::Expression> expression);
        Exception(Context& context, std::string const& message, Data const& type, std::shared_ptr<Parser::Expression> expression);

        void print_stack_trace(Context& context) const;

    };
    class FunctionArgumentsError {};

    using Arguments = std::variant<std::shared_ptr<Parser::Expression>, Reference>;

    Reference call_function(Context& context, std::shared_ptr<Parser::Expression> expression, Reference const& func, Arguments const& arguments);
    Reference execute(Context& context, std::shared_ptr<Parser::Expression> expression);

    Reference set(Context& context, Reference const& var, Reference const& data);
    std::string string_from(Context& context, Reference const& data);


    inline Object* insert(GlobalContext& context, std::string const& symbol) {
        auto& data = std::get<SymbolReference>(context[symbol]).get();

        if (data == Data{})
            data = Data(context.new_object());

        return data.get<Object*>();
    }

    inline Object* insert(GlobalContext& context, Object* obj, std::string const& property) {
        auto& data = obj->properties[property];

        if (data == Data{})
            data = Data(context.new_object());

        return data.get<Object*>();
    }

}

#endif
