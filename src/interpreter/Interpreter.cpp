#include <cstddef>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <variant>

#include <ouverium/types.h>

#include "Interpreter.hpp"

#include "../parser/Expressions.hpp"

#include "../Types.hpp"


namespace Interpreter {

    Exception::Exception(Context& context, std::shared_ptr<Parser::Expression> const& thrower, Reference reference) :
        reference(std::move(reference)) {
        if (thrower)
            positions.push_back(thrower->position);
        Context* old_c = nullptr;
        Context* c = &context;
        while (c != old_c) {
            if (auto* function_context = dynamic_cast<FunctionContext*>(c); (function_context != nullptr) && function_context->caller)
                positions.push_back(function_context->caller->position);
            old_c = c;
            c = &c->get_parent();
        }
    }

    Exception::Exception(Context& context, std::shared_ptr<Parser::Expression> const& thrower, std::string const& message) :
        Exception(context, thrower, Data(GC::new_object(Object(message)))) {}

    void Exception::print_stack_trace(Context& context) const {
        if (!positions.empty()) {
            std::cerr << "An exception occured: ";
            try {
                std::cerr << reference.to_data(context).get<ObjectPtr>()->to_string();
            } catch (Exception const&) {
            } catch (Data::BadAccess const&) {}
            std::cerr << std::endl;
            for (auto const& p : positions)
                std::cerr << "\tin " << p << std::endl;
        }
    }


    using ParserExpression = std::shared_ptr<Parser::Expression>;
    class Computed : public std::map<ParserExpression, Reference> {

    public:

        using std::map<ParserExpression, Reference>::map;

        [[nodiscard]] Arguments get(Arguments const& arguments) const {
            if (auto const* expression = std::get_if<ParserExpression>(&arguments)) {
                auto it = find(*expression);
                if (it != end())
                    return it->second;
            }
            return arguments;
        }

        Reference compute(Context& context, Arguments const& arguments) {
            if (auto const* expression = std::get_if<ParserExpression>(&arguments))
                return operator[](*expression) = Interpreter::execute(context, *expression);
            else if (auto const* reference = std::get_if<Reference>(&arguments))
                return *reference;
            else
                return {};
        }

    };

    void set_arguments(Context& context, FunctionContext& function_context, Computed& computed, std::shared_ptr<Parser::Expression> const& parameters, Arguments const& argument) {
        auto arguments = computed.get(argument);

        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            auto reference = computed.compute(context, arguments);

            if (function_context.has_symbol(symbol->name)) {
                if (reference != Reference(function_context[symbol->name]))
                    throw Interpreter::FunctionArgumentsError();
            } else {
                function_context.add_symbol(symbol->name, reference.to_indirect_reference(context, parameters));
            }
        } else if (auto p_tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            if (auto* expression = std::get_if<ParserExpression>(&arguments)) {
                if (auto a_tuple = std::dynamic_pointer_cast<Parser::Tuple>(*expression)) {
                    if (p_tuple->objects.size() == a_tuple->objects.size()) {
                        for (size_t i = 0; i < p_tuple->objects.size(); ++i)
                            set_arguments(context, function_context, computed, p_tuple->objects[i], a_tuple->objects[i]);

                        TupleReference cache;
                        for (auto const& o : a_tuple->objects) {
                            auto it = computed.find(o);
                            if (it != computed.end())
                                cache.push_back(it->second);
                            else
                                return;
                        }
                        computed[a_tuple] = cache;
                    } else
                        throw Interpreter::FunctionArgumentsError();
                } else {
                    set_arguments(context, function_context, computed, parameters, computed.compute(context, arguments));
                }
            } else if (auto* reference = std::get_if<Reference>(&arguments)) {
                if (auto* tuple_reference = std::get_if<TupleReference>(reference)) {
                    if (tuple_reference->size() == p_tuple->objects.size()) {
                        for (size_t i = 0; i < p_tuple->objects.size(); ++i)
                            set_arguments(context, function_context, computed, p_tuple->objects[i], (*tuple_reference)[i]);
                    } else
                        throw Interpreter::FunctionArgumentsError();
                } else {
                    try {
                        auto object = reference->to_data(function_context, parameters).get<ObjectPtr>();
                        if (object->array.capacity() > 0 && object->array.size() == p_tuple->objects.size()) {
                            for (size_t i = 0; i < p_tuple->objects.size(); ++i)
                                set_arguments(context, function_context, computed, p_tuple->objects[i], Data(object).get_at(i));
                        } else
                            throw Interpreter::FunctionArgumentsError();
                    } catch (Data::BadAccess const&) {
                        throw Interpreter::FunctionArgumentsError();
                    }
                }
            }
        } else if (auto p_function = std::dynamic_pointer_cast<Parser::FunctionCall>(parameters)) {
            if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(p_function->function); symbol && !function_context.has_symbol(symbol->name)) {
                ObjectPtr object = GC::new_object();
                auto function_definition = std::make_shared<Parser::FunctionDefinition>();
                function_definition->parameters = p_function->arguments;

                if (auto* expression = std::get_if<ParserExpression>(&arguments)) {
                    auto it = computed.find(*expression);
                    if (it == computed.end()) {
                        function_definition->body = *expression;

                        object->functions.emplace_front(CustomFunction{ function_definition });
                        auto& f = object->functions.back();
                        for (auto symbol : function_definition->body->symbols)
                            f.extern_symbols.emplace(symbol, context[symbol]);
                    } else {
                        if (auto* tuple_reference = std::get_if<TupleReference>(&it->second)) {
                            auto tuple = std::make_shared<Parser::Tuple>();
                            for (size_t i = 0; i < tuple_reference->size(); ++i)
                                tuple->objects.push_back(std::make_shared<Parser::Symbol>("#cached" + std::to_string(i)));
                            function_definition->body = tuple;

                            object->functions.emplace_front(CustomFunction{ function_definition });
                            for (size_t i = 0; i < tuple_reference->size(); ++i)
                                object->functions.back().extern_symbols.emplace("#cached" + std::to_string(i), (*tuple_reference)[i].to_indirect_reference(context));
                        } else {
                            function_definition->body = std::make_shared<Parser::Symbol>("#cached");

                            object->functions.emplace_front(CustomFunction{ function_definition });
                            object->functions.back().extern_symbols.emplace("#cached", it->second.to_indirect_reference(context, parameters));
                        }
                    }

                } else if (auto* reference = std::get_if<Reference>(&arguments)) {
                    if (auto* tuple_reference = std::get_if<TupleReference>(reference)) {
                        auto tuple = std::make_shared<Parser::Tuple>();
                        for (size_t i = 0; i < tuple_reference->size(); ++i)
                            tuple->objects.push_back(std::make_shared<Parser::Symbol>("#cached" + std::to_string(i)));
                        function_definition->body = tuple;

                        object->functions.emplace_front(CustomFunction{ function_definition });
                        for (size_t i = 0; i < tuple_reference->size(); ++i)
                            object->functions.back().extern_symbols.emplace("#cached" + std::to_string(i), (*tuple_reference)[i].to_indirect_reference(context, parameters));
                    } else {
                        function_definition->body = std::make_shared<Parser::Symbol>("#cached");

                        object->functions.emplace_front(CustomFunction{ function_definition });
                        object->functions.back().extern_symbols.emplace("#cached", reference->to_indirect_reference(context, parameters));
                    }
                }

                function_context.add_symbol(symbol->name, Data(object));
            } else {
                auto r = execute(function_context, p_function->function).to_data(context, parameters);

                Arguments args;
                if (auto* expression = std::get_if<ParserExpression>(&arguments)) {
                    auto it = computed.find(*expression);
                    args = it != computed.end() ? it->second : arguments;
                } else {
                    args = arguments;
                }

                auto const result = try_call_function(context, p_function, r, args);
                if (auto const* reference = std::get_if<Reference>(&result)) {
                    set_arguments(context, function_context, computed, p_function->arguments, *reference);
                } else
                    throw Interpreter::FunctionArgumentsError();
            }
        } else if (auto p_property = std::dynamic_pointer_cast<Parser::Property>(parameters)) {
            auto reference = computed.compute(context, arguments);

            if (auto* property_reference = std::get_if<PropertyReference>(&reference)) {
                if (p_property->name == property_reference->name || p_property->name == ".") {
                    set_arguments(context, function_context, computed, p_property->object, Reference(property_reference->parent));
                } else
                    throw Interpreter::FunctionArgumentsError();
            } else
                throw Interpreter::FunctionArgumentsError();
        } else
            throw Interpreter::FunctionArgumentsError();
    }

    std::variant<Reference, Exception> try_call_function(Context& context, std::shared_ptr<Parser::Expression> const& caller, Reference const& func, Arguments const& arguments) {
        if (context.get_recurion_level() >= context.get_global().recursion_limit)
            throw Exception(context, caller, "recursion limit exceeded");

        std::list<Function> functions;
        try {
            functions = func.to_data(context, caller).get<ObjectPtr>()->functions;
        } catch (Data::BadAccess const&) {}

        if (functions.empty()) {
            try {
                functions = call_function(context, caller, context.get_global()["function_getter"], func).to_data(context).get<ObjectPtr>()->functions;
            } catch (Data::BadAccess const&) {}
        }

        Computed computed;

        for (auto const& function : functions) {
            try {
                FunctionContext function_context(context, caller);
                for (auto const& symbol : function.extern_symbols)
                    function_context.add_symbol(symbol.first, symbol.second);

                if (auto const* custom_function = std::get_if<CustomFunction>(&function)) {
                    set_arguments(context, function_context, computed, (*custom_function)->parameters, arguments);

                    Data filter = Data(true);
                    if ((*custom_function)->filter != nullptr)
                        filter = execute(function_context, (*custom_function)->filter).to_data(context, (*custom_function)->filter);

                    try {
                        if (filter.get<bool>())
                            return Interpreter::execute(function_context, (*custom_function)->body);
                        else
                            continue;
                    } catch (Data::BadAccess const&) {
                        throw FunctionArgumentsError();
                    }
                } else if (auto const* system_function = std::get_if<SystemFunction>(&function)) {
                    set_arguments(context, function_context, computed, system_function->parameters, arguments);

                    return system_function->pointer(function_context);
                } else
                    return Reference();
            } catch (FunctionArgumentsError&) {}
        }

        if (functions.empty())
            return Exception(context, caller, "not a function");
        else
            return Exception(context, caller, "incorrect function arguments");
    }

    Reference call_function(Context& context, std::shared_ptr<Parser::Expression> const& caller, Reference const& func, Arguments const& arguments) {
        auto r = try_call_function(context, caller, func, arguments);

        if (auto* reference = std::get_if<Reference>(&r))
            return *reference;
        else
            throw std::move(std::get<Exception>(r));
    }

    Reference execute(Context& context, std::shared_ptr<Parser::Expression> const& expression) {
        if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(expression)) {
            auto reference = execute(context, function_call->function);

            return call_function(context, function_call, reference, function_call->arguments);
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
            auto object = GC::new_object();
            object->functions.emplace_front(CustomFunction{ function_definition });
            auto& f = object->functions.back();

            for (auto symbol : function_definition->captures)
                f.extern_symbols.emplace(symbol, context[symbol]);

            return Data(object);
        } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(expression)) {
            auto data = execute(context, property->object).to_data(context, expression);
            return data.get_property(property->name);
        } else if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(expression)) {
            auto data = get_symbol(symbol->name);
            if (auto* b = std::get_if<bool>(&data)) {
                return Data(*b);
            } else if (auto* l = std::get_if<OV_INT>(&data)) {
                return Data(*l);
            } else if (auto* d = std::get_if<OV_FLOAT>(&data)) {
                return Data(*d);
            } else if (auto* str = std::get_if<std::string>(&data)) {
                return Data(GC::new_object(*str));
            } else {
                return context[symbol->name];
            }
        } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(expression)) {
            TupleReference tuple_reference;
            for (auto const& e : tuple->objects)
                tuple_reference.push_back(execute(context, e));
            return tuple_reference;
        } else
            return {};
    }


    Reference set(Context& context, Reference const& var, Reference const& data) {
        return call_function(context, nullptr, context.get_global()["setter"], TupleReference{ var, data });
    }

    std::string string_from(Context& context, Reference const& data) {
        std::ostringstream oss;

        auto d = call_function(context, nullptr, context.get_global()["string_from"], data).to_data(context, nullptr);
        try {
            oss << d.get<ObjectPtr>()->to_string();
        } catch (Data::BadAccess const&) {}

        return oss.str();
    }

}
