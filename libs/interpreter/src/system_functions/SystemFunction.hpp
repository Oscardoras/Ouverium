#ifndef __INTERPRETER_SYSTEMFUNCTION_HPP__
#define __INTERPRETER_SYSTEMFUNCTION_HPP__

#include <any>
#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <ouverium/types.h>

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/parser/Expressions.hpp>


namespace Interpreter::SystemFunctions {

    void init(GlobalContext& context);

    ObjectPtr get_object(IndirectReference const& reference);

    template<typename Arg>
    [[nodiscard]] Arg get_arg(FunctionContext& /*context*/, Data const& data) {
        return data.get<ObjectPtr>()->c_obj.get<Arg>();
    }
    template<>
    [[nodiscard]] inline bool get_arg<bool>(FunctionContext& /*context*/, Data const& data) {
        return data.get<bool>();
    }
    template<>
    [[nodiscard]] inline char get_arg<char>(FunctionContext& /*context*/, Data const& data) {
        return data.get<char>();
    }
    template<>
    [[nodiscard]] inline OV_INT get_arg<OV_INT>(FunctionContext& /*context*/, Data const& data) {
        return data.get<OV_INT>();
    }
    template<>
    [[nodiscard]] inline OV_FLOAT get_arg<OV_FLOAT>(FunctionContext& /*context*/, Data const& data) {
        return data.get<OV_FLOAT>();
    }
    template<>
    [[nodiscard]] inline ObjectPtr get_arg<ObjectPtr>(FunctionContext& /*context*/, Data const& data) {
        return data.get<ObjectPtr>();
    }
    template<>
    [[nodiscard]] inline std::string get_arg<std::string>(FunctionContext& /*context*/, Data const& data) {
        return data.get<ObjectPtr>()->to_string();
    }
    template<>
    [[nodiscard]] inline Data get_arg<Data>(FunctionContext& /*context*/, Data const& data) {
        return data;
    }

    template<size_t I, typename Arg>
    [[nodiscard]] std::remove_cvref_t<Arg> get_arg(FunctionContext& context) {
        try {
            return get_arg<std::remove_cvref_t<Arg>>(context, context["arg" + std::to_string(I)].to_data(context));
        } catch (Data::BadAccess const&) {
            throw FunctionArgumentsError();
        } catch (std::bad_any_cast const&) {
            throw FunctionArgumentsError();
        }
    }
    template<size_t I>
    [[nodiscard]] IndirectReference get_arg(FunctionContext& context) {
        return context["arg" + std::to_string(I)];
    }

    template<size_t... I, typename... Args>
    [[nodiscard]] Reference eval(Reference(*function)(Args...), FunctionContext& context, std::index_sequence<I...> /*unused*/) {
        return function(get_arg<I, Args>(context)...);
    }

    template<size_t... I>
    [[nodiscard]] std::shared_ptr<Parser::Expression> get_parameters(std::index_sequence<I...> /*unused*/) {
        return std::make_shared<Parser::Tuple>(Parser::Tuple({ std::make_shared<Parser::Symbol>("arg" + std::to_string(I))..., }));
    }
    template<>
    [[nodiscard]] inline std::shared_ptr<Parser::Expression> get_parameters(std::index_sequence<0> /*unused*/) {
        return std::make_shared<Parser::Symbol>("arg" + std::to_string(0));
    }

    template<typename... Args>
    void add_function(IndirectReference const& reference, Reference(*function)(Args...)) {
        auto pointer = [function](FunctionContext& context) -> Reference {
            return eval(function, context, std::index_sequence_for<Args...>{});
        };
        auto parameters = get_parameters(std::index_sequence_for<Args...>{});
        get_object(reference)->functions.emplace_front(SystemFunction{ parameters, pointer });
    }

    inline void add_function(IndirectReference const& reference, std::shared_ptr<Parser::Expression> parameters, Reference(*function)(FunctionContext&)) {
        get_object(reference)->functions.emplace_front(SystemFunction{ .parameters = std::move(parameters), .pointer = function });
    }

}

#endif
