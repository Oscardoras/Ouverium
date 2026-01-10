#pragma once

#include <any>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include <ouverium/interpreter/Interpreter.hpp>

#include <ouverium/parser/Expressions.hpp>

#include <ouverium/types.h>


namespace Interpreter::SystemFunctions {

    void init(GlobalContext& context);

    ObjectPtr get_object(IndirectReference const& reference);

    template<typename Arg>
    [[nodiscard]] std::optional<std::conditional_t<std::is_reference_v<Arg>, std::reference_wrapper<std::remove_reference_t<Arg>>, std::remove_reference_t<Arg>>> get_arg(Data const& data) {
        if constexpr (!std::is_reference_v<Arg>) {
            return std::optional<Arg>(data.get<Arg>());
        } else {
            using T = std::remove_reference_t<Arg>;
            if (auto* t = get_if<std::reference_wrapper<T>>(&data))
                return std::optional<std::reference_wrapper<T>>(*t);
            else if (auto* t = get_if<std::shared_ptr<T>>(&data))
                return std::optional<std::reference_wrapper<T>>(**t);
            else
                return std::optional<std::reference_wrapper<T>>(std::nullopt);
        }
    }

    template<>
    [[nodiscard]] inline std::optional<Data> get_arg<Data>(Data const& data) {
        return data;
    }

    template<>
    [[nodiscard]] inline std::optional<std::string> get_arg<std::string>(Data const& data) {
        if (auto const* obj = get_if<ObjectPtr>(&data))
            return (*obj)->to_string();

        return std::nullopt;
    }

    template<size_t I, typename Arg>
    [[nodiscard]] decltype(auto) get_arg(FunctionContext& context) {
        try {
            using T = std::conditional_t<std::is_reference_v<Arg>&& std::is_const_v<std::remove_reference_t<Arg>>, std::remove_cvref_t<Arg>, Arg>;

            auto opt = get_arg<T>(context["arg" + std::to_string(I)].to_data(context));
            if (opt.has_value())
                if constexpr (std::is_reference_v<T>)
                    return opt->get();
                else
                    return T(*opt);
            else
                throw FunctionArgumentsError();
        } catch (std::exception const&) {
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

    template<size_t... I, typename... Args>
    [[nodiscard]] Reference eval(Reference(*function)(FunctionContext& context, Args...), FunctionContext& context, std::index_sequence<I...> /*unused*/) {
        return function(context, get_arg<I, Args>(context)...);
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

    template<typename... Args>
    void add_function(IndirectReference const& reference, Reference(*function)(FunctionContext& context, Args...)) {
        auto pointer = [function](FunctionContext& context) -> Reference {
            return eval(function, context, std::index_sequence_for<Args...>{});
        };
        auto parameters = get_parameters(std::index_sequence_for<Args...>{});
        get_object(reference)->functions.emplace_front(SystemFunction{ parameters, pointer });
    }

    inline void add_function(IndirectReference const& reference, std::shared_ptr<Parser::Expression> parameters, Reference(*function)(FunctionContext&)) {
        get_object(reference)->functions.emplace_front(SystemFunction{ .parameters = std::move(parameters), .pointer = function });
    }

    template<typename T>
    Reference check_type(Data const& data) {
        try {
            return Data(get_arg<T>(data).has_value());
        } catch (Data::BadAccess const&) {
        } catch (std::bad_any_cast const&) {}

        return Data(false);
    }

}
