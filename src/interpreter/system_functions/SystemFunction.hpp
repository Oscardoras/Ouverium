#ifndef __INTERPRETER_SYSTEMFUNCTION_HPP__
#define __INTERPRETER_SYSTEMFUNCTION_HPP__

#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    void init(GlobalContext& context);

    ObjectPtr get_object(IndirectReference const& reference);

    template<typename Arg>
    Arg get_arg(FunctionContext&, Data const& data) {
        return data.get<ObjectPtr>()->c_obj.get<Arg>();
    }
    template<>
    inline bool get_arg<bool>(FunctionContext&, Data const& data) {
        return data.get<bool>();
    }
    template<>
    inline char get_arg<char>(FunctionContext&, Data const& data) {
        return data.get<char>();
    }
    template<>
    inline OV_INT get_arg<OV_INT>(FunctionContext&, Data const& data) {
        return data.get<OV_INT>();
    }
    template<>
    inline OV_FLOAT get_arg<OV_FLOAT>(FunctionContext&, Data const& data) {
        return data.get<OV_FLOAT>();
    }
    template<>
    inline ObjectPtr get_arg<ObjectPtr>(FunctionContext&, Data const& data) {
        return data.get<ObjectPtr>();
    }
    template<>
    inline std::string get_arg<std::string>(FunctionContext&, Data const& data) {
        return data.get<ObjectPtr>()->to_string();
    }
    template<>
    inline Data get_arg<Data>(FunctionContext&, Data const& data) {
        return data;
    }

    template<size_t I, typename Arg>
    std::remove_reference_t<Arg> get_arg(FunctionContext& context) {
        try {
            return get_arg<std::remove_reference_t<Arg>>(context, context["arg" + I].to_data(context));
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        } catch (std::bad_any_cast const&) {
            throw FunctionArgumentsError();
        }
    }
    template<size_t I>
    IndirectReference get_arg(FunctionContext& context) {
        return context["arg" + I];
    }

    template<size_t... I, typename... Args>
    Reference eval(Reference(*function)(Args...), FunctionContext& context, std::index_sequence<I...>) {
        return function(get_arg<I, Args>(context)...);
    }

    template<size_t... I>
    std::shared_ptr<Parser::Expression> get_parameters(std::index_sequence<I...>) {
        return std::make_shared<Parser::Tuple>(Parser::Tuple({ std::make_shared<Parser::Symbol>("arg" + I)..., }));
    }
    template<>
    inline std::shared_ptr<Parser::Expression> get_parameters(std::index_sequence<0>) {
        return std::make_shared<Parser::Symbol>("arg" + 0);
    }

    template<typename... Args>
    void add_function(IndirectReference const& reference, Reference(*function)(Args...)) {
        auto pointer = [function](FunctionContext& context) -> Reference {
            return eval(function, context, std::index_sequence_for<Args...>{});
        };
        auto parameters = get_parameters(std::index_sequence_for<Args...>{});
        get_object(reference)->functions.push_front(SystemFunction{ parameters, pointer });
    }

    inline void add_function(IndirectReference const& reference, std::shared_ptr<Parser::Expression> parameters, Reference(*function)(FunctionContext&)) {
        get_object(reference)->functions.push_front(SystemFunction{ parameters, function });
    }

}

#endif
