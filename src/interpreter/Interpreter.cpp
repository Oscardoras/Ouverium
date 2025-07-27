#include <cstddef>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <variant>

#include <ouverium/types.h>

#include "Interpreter.hpp"

#include "../parser/Expressions.hpp"

#include "../Types.hpp"
#include "GC.hpp"


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
    class Computed : public std::map<ParserExpression, Arguments> {

    public:

        using std::map<ParserExpression, Arguments>::map;

        [[nodiscard]] Arguments get(Arguments const& arguments) const {
            if (auto const* expression = std::get_if<ParserExpression>(&arguments)) {
                auto it = find(*expression);
                if (it != end())
                    return it->second;
            }
            return arguments;
        }

        Reference compute(Context& context, Arguments const& arguments) {
            if (auto const* expression = std::get_if<ParserExpression>(&arguments)) {
                if (*expression == nullptr)
                    throw Interpreter::FunctionArgumentsError();

                auto r = Interpreter::execute(context, *expression);
                operator[](*expression) = r;
                return r;
            } else if (auto const* reference = std::get_if<Reference>(&arguments)) {
                return *reference;
            } else if (auto const* vector = std::get_if<std::vector<Arguments>>(&arguments)) {
                return call_function(context, nullptr, context.get_global()["setter"], std::vector<Arguments>{ GC::new_reference(), * vector });
            } else
                return {};
        }

    };

    bool check_tuple(Context& context, FunctionContext& function_context, std::shared_ptr<Parser::Tuple> const& parameters, size_t args) {
        if (parameters->objects.size() < args) return false;

        for (size_t i = args; i < parameters->objects.size(); ++i) {
            auto p_function = std::dynamic_pointer_cast<Parser::FunctionCall>(parameters->objects[i]);
            if (!p_function) return false;

            auto r = execute(function_context, p_function->function).to_data(context, p_function);
            if (r != context["setter"].get_data()) return false;

            auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(p_function->arguments);
            if (!tuple || tuple->objects.size() != 2) return false;
        }

        return true;
    }

    std::optional<std::pair<std::string, Arguments>> is_explicit(Context& context, std::shared_ptr<Parser::Expression> const& parameters, Arguments const& argument) {
        auto const* e = std::get_if<ParserExpression>(&argument);
        if (!e) return std::nullopt;

        auto a_function = std::dynamic_pointer_cast<Parser::FunctionCall>(*e);
        if (!a_function) return std::nullopt;

        auto a_symbol = std::dynamic_pointer_cast<Parser::Symbol>(a_function->function);
        if (!a_symbol) return std::nullopt;

        auto r = execute(context, a_symbol).to_data(context, parameters);
        if (r != context["setter"].get_data()) return std::nullopt;

        auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(a_function->arguments);
        if (!tuple || tuple->objects.size() != 2) return std::nullopt;

        auto a_property = std::dynamic_pointer_cast<Parser::Property>(tuple->objects[0]);
        if (!a_property) return std::nullopt;

        if (!std::get_if<std::weak_ptr<Parser::Expression>>(&a_property->object)) return std::nullopt;

        return std::pair<std::string, Arguments>{ a_property->name, tuple->objects[1] };
    }

    void set_arguments(Context& context, FunctionContext& function_context, std::set<std::string> const& symbols, Computed& computed, std::shared_ptr<Parser::Expression> const& parameters, Arguments const& argument) {
        auto arguments = computed.get(argument);

        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            if (auto e = is_explicit(context, parameters, arguments); e && symbols.contains(e->first)) {
                function_context.add_symbol(e->first, computed.compute(context, e->second));
            } else {
                auto reference = computed.compute(context, arguments);

                if (function_context.has_symbol(symbol->name)) {
                    if (reference != Reference(function_context[symbol->name]))
                        throw Interpreter::FunctionArgumentsError();
                } else {
                    function_context.add_symbol(symbol->name, reference);
                }
            }
        } else if (auto p_tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            if (auto* expression = std::get_if<ParserExpression>(&arguments)) {
                if (auto a_tuple = std::dynamic_pointer_cast<Parser::Tuple>(*expression)) {
                    if (check_tuple(context, function_context, p_tuple, a_tuple->objects.size())) {
                        for (size_t i = 0; i < p_tuple->objects.size(); ++i) {
                            auto const& arg = i < a_tuple->objects.size() ? a_tuple->objects[i] : nullptr;
                            set_arguments(context, function_context, symbols, computed, p_tuple->objects[i], arg);
                        }

                        std::vector<Arguments> cache;
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
                    set_arguments(context, function_context, symbols, computed, parameters, computed.compute(context, arguments));
                }
            } else if (auto* reference = std::get_if<Reference>(&arguments)) {
                try {
                    try {
                        if (p_tuple->objects.size() > 0) {
                            if (auto p_function = std::dynamic_pointer_cast<Parser::FunctionCall>(p_tuple->objects[0])) {
                                auto r = execute(function_context, p_function->function).to_data(context, p_function);
                                if (r == context["setter"].get_data()) {
                                    if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(p_function->arguments); tuple && tuple->objects.size() == 2)
                                        set_arguments(context, function_context, symbols, computed, p_tuple, std::vector<Arguments>{ *reference });
                                    else
                                        throw Interpreter::FunctionArgumentsError();
                                } else
                                    throw Interpreter::FunctionArgumentsError();
                            } else
                                throw Interpreter::FunctionArgumentsError();
                        } else
                            throw Interpreter::FunctionArgumentsError();
                    } catch (Interpreter::FunctionArgumentsError const&) {
                        auto object = reference->to_data(function_context, parameters).get<ObjectPtr>();
                        if (check_tuple(context, function_context, p_tuple, object->array.size())) {
                            for (size_t i = 0; i < p_tuple->objects.size(); ++i) {
                                auto const& arg = i < object->array.size() ? Arguments(Data(object).get_at(i)) : ParserExpression(nullptr);
                                set_arguments(context, function_context, symbols, computed, p_tuple->objects[i], arg);
                            }
                        } else
                            throw Interpreter::FunctionArgumentsError();
                    }
                } catch (Data::BadAccess const&) {
                    throw Interpreter::FunctionArgumentsError();
                }
            } else if (auto* vector = std::get_if<std::vector<Arguments>>(&arguments)) {
                if (check_tuple(context, function_context, p_tuple, vector->size())) {
                    for (size_t i = 0; i < p_tuple->objects.size(); ++i) {
                        auto const& arg = i < (*vector).size() ? (*vector)[i] : ParserExpression(nullptr);
                        set_arguments(context, function_context, symbols, computed, p_tuple->objects[i], arg);
                    }
                } else
                    throw Interpreter::FunctionArgumentsError();
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
                        function_definition->body = std::make_shared<Parser::Symbol>("#cached");

                        object->functions.emplace_front(CustomFunction{ function_definition });
                        object->functions.back().extern_symbols.emplace("#cached", computed.compute(context, it->second));
                    }
                } else if (auto* reference = std::get_if<Reference>(&arguments)) {
                    function_definition->body = std::make_shared<Parser::Symbol>("#cached");

                    object->functions.emplace_front(CustomFunction{ function_definition });
                    object->functions.back().extern_symbols.emplace("#cached", *reference);
                }

                function_context.add_symbol(symbol->name, Data(object));
            } else {
                auto r = execute(function_context, p_function->function).to_data(context, parameters);

                if (r == context["setter"].get_data()) {
                    if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(p_function->arguments); tuple && tuple->objects.size() == 2) {
                        if (auto* ptr = std::get_if<std::shared_ptr<Parser::Expression>>(&arguments)) {
                            if (*ptr == nullptr) {
                                set_arguments(context, function_context, symbols, computed, tuple->objects[0], tuple->objects[1]);
                            } else if (auto empty_tuple = std::dynamic_pointer_cast<Parser::Tuple>(*ptr); empty_tuple && empty_tuple->objects.empty()) {
                                set_arguments(context, function_context, symbols, computed, tuple->objects[0], tuple->objects[1]);
                            } else {
                                set_arguments(context, function_context, symbols, computed, tuple->objects[0], arguments);
                            }
                        } else {
                            set_arguments(context, function_context, symbols, computed, tuple->objects[0], arguments);
                        }
                    } else
                        throw Interpreter::FunctionArgumentsError();
                } else {
                    Arguments args;
                    if (auto* expression = std::get_if<ParserExpression>(&arguments)) {
                        auto it = computed.find(*expression);
                        args = it != computed.end() ? it->second : arguments;
                    } else {
                        args = arguments;
                    }

                    auto const result = try_call_function(context, p_function, r, args);
                    if (auto const* reference = std::get_if<Reference>(&result)) {
                        set_arguments(context, function_context, symbols, computed, p_function->arguments, *reference);
                    } else
                        throw Interpreter::FunctionArgumentsError();
                }
            }
        } else if (auto p_property = std::dynamic_pointer_cast<Parser::Property>(parameters)) {
            auto reference = computed.compute(context, arguments);

            if (auto* property_reference = std::get_if<PropertyReference>(&reference)) {
                if (p_property->name == property_reference->name || p_property->name == ".") {
                    if (auto* ptr = std::get_if<std::shared_ptr<Parser::Expression>>(&p_property->object)) {
                        set_arguments(context, function_context, symbols, computed, *ptr, Reference(property_reference->parent));
                    } else
                        throw Interpreter::FunctionArgumentsError();
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
        auto func_data = func.to_data(context, caller);
        if (auto const* obj = get_if<ObjectPtr>(&func_data)) {
            functions = (*obj)->functions;
        }

        if (functions.empty()) {
            auto function_getter = call_function(context, caller, context.get_global()["function_getter"], func).to_data(context);
            if (auto const* obj = get_if<ObjectPtr>(&function_getter)) {
                functions = (*obj)->functions;
            }
        }

        Computed computed;

        for (auto const& function : functions) {
            try {
                FunctionContext function_context(context, caller);
                for (auto const& symbol : function.extern_symbols)
                    function_context.add_symbol(symbol.first, symbol.second);

                if (auto const* custom_function = std::get_if<CustomFunction>(&function)) {
                    std::set<std::string> symbols;
                    for (auto const& symbol : (*custom_function)->parameters->symbols)
                        symbols.emplace(symbol);

                    set_arguments(context, function_context, symbols, computed, (*custom_function)->parameters, arguments);

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
                    set_arguments(context, function_context, {}, computed, system_function->parameters, arguments);

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
            if (auto* ptr = std::get_if<std::shared_ptr<Parser::Expression>>(&property->object)) {
                auto data = execute(context, *ptr).to_data(context, expression);
                return data.get_property(property->name);
            } else if (auto* ptr = std::get_if<std::weak_ptr<Parser::Expression>>(&property->object)) {
                auto it = context.tuples.find(ptr->lock());
                if (it != context.tuples.end())
                    return it->second.get_property(property->name);
                else
                    return Data(GC::new_object()).get_property(property->name);
            }

            return {};
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
            auto object = GC::new_object();

            auto& t = context.tuples[tuple];
            t = Data(object);
            for (auto const& e : tuple->objects) {
                auto child = execute(context, e);
                if (auto* property = std::get_if<PropertyReference>(&child)) {
                    if (property->parent == t) {
                        continue;
                    }
                }
                object->array.push_back(child.to_data(context));
            }
            context.tuples.erase(tuple);

            return Data(object);
        } else
            return {};
    }


    Reference set(Context& context, Reference const& var, Reference const& data) {
        return call_function(context, nullptr, context.get_global()["setter"], std::vector<Arguments>{ var, data });
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
