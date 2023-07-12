#include <stdexcept>
#include <iostream>

#include "Function.hpp"
#include "Interpreter.hpp"

#include "system_functions/Array.hpp"
#include "system_functions/ArrayList.hpp"
#include "system_functions/Base.hpp"
#include "system_functions/Math.hpp"
#include "system_functions/Streams.hpp"
#include "system_functions/String.hpp"
#include "system_functions/Types.hpp"

#include "../parser/Standard.hpp"

#include "../Utils.hpp"


namespace Interpreter {

    auto getter_args = std::make_shared<Parser::Symbol>("var");
    Reference getter(FunctionContext & context) {
        return context["var"] = context.new_object();
    }

    Object* init_getter(GlobalContext & context) {
        auto object = context.new_object();
        context["getter"] = object;
        object->functions.push_front(SystemFunction{getter_args, getter});
        return object;
    }

    GlobalContext::GlobalContext(std::shared_ptr<Parser::Expression> expression):
        Context(expression), getter(init_getter(*this)) {
        Array::init(*this);
        //ArrayList::init(*this);
        Base::init(*this);
        Math::init(*this);
        Streams::init(*this);
        //String::init(*this);
        Types::init(*this);
    }


    using ParserExpression = std::shared_ptr<Parser::Expression>;
    class Computed : public std::map<ParserExpression, Reference> {

    public:

        using std::map<ParserExpression, Reference>::map;

        Arguments get(Arguments const& arguments) const {
            if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                auto it = find(*expression);
                if (it != end())
                    return it->second;
            }
            return arguments;
        }

        Reference compute(Context & context, Arguments const& arguments) {
            if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                return operator[](*expression) = Interpreter::execute(context, *expression);
            } else if (auto reference = std::get_if<Reference>(&arguments)) {
                return *reference;
            } else return *((Reference*) nullptr);
        }

    };

    void set_arguments(Context & context, FunctionContext & function_context, Computed & computed, std::shared_ptr<Parser::Expression> parameters, Arguments const& argument) {
        auto arguments = computed.get(argument);

        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            auto reference = computed.compute(context, arguments);

            if (function_context.has_symbol(symbol->name)) {
                if (reference.to_data(context) != function_context[symbol->name].to_data(context))
                    throw Interpreter::FunctionArgumentsError();
            } else {
                function_context.add_symbol(symbol->name, reference);
            }
        } else if (auto p_tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                if (auto a_tuple = std::dynamic_pointer_cast<Parser::Tuple>(*expression)) {
                    if (p_tuple->objects.size() == a_tuple->objects.size()) {
                        for (size_t i = 0; i < p_tuple->objects.size(); i++)
                            set_arguments(context, function_context, computed, p_tuple->objects[i], a_tuple->objects[i]);

                        TupleReference cache;
                        for (auto const& o : a_tuple->objects) {
                            auto it = computed.find(o);
                            if (it != computed.end()) {
                                cache.push_back(it->second);
                            } else return;
                        }
                        computed[a_tuple] = cache;
                    } else throw Interpreter::FunctionArgumentsError();
                } else {
                    set_arguments(context, function_context, computed, parameters, computed.compute(context, arguments));
                }
            } else if (auto reference = std::get_if<Reference>(&arguments)) {
                if (auto tuple_reference = std::get_if<TupleReference>(reference)) {
                    if (tuple_reference->size() == p_tuple->objects.size()) {
                        for (size_t i = 0; i < p_tuple->objects.size(); i++)
                            set_arguments(context, function_context, computed, p_tuple->objects[i], tuple_reference[i]);
                    } else throw Interpreter::FunctionArgumentsError();
                } else {
                    auto data = reference->to_data(function_context);
                    try {
                        auto object = data.get<Object*>();
                        if (object->array.size() == p_tuple->objects.size()) {
                            for (size_t i = 0; i < p_tuple->objects.size(); i++)
                                set_arguments(context, function_context, computed, p_tuple->objects[i], ArrayReference{*object, i});
                        } else throw Interpreter::FunctionArgumentsError();
                    } catch (Data::BadAccess const& e) {
                        throw Interpreter::FunctionArgumentsError();
                    }
                }
            }
        } else if (auto p_function = std::dynamic_pointer_cast<Parser::FunctionCall>(parameters)) {
            if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(p_function->function)) {
                if (!function_context.has_symbol(symbol->name)) {
                    Object* object = context.new_object();
                    auto function_definition = std::make_shared<Parser::FunctionDefinition>();
                    function_definition->parameters = p_function->arguments;

                    if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                        auto it = computed.find(*expression);
                        if (it == computed.end()) {
                            function_definition->body = *expression;

                            object->functions.push_front(CustomFunction{function_definition});
                            auto & f = object->functions.back();
                            for (auto symbol : function_definition->body->symbols)
                                f.extern_symbols.emplace(symbol, context[symbol]);
                        } else {
                            function_definition->body = std::make_shared<Parser::Symbol>("#cached");

                            object->functions.push_front(CustomFunction{function_definition});
                            object->functions.back().extern_symbols.emplace("#cached", it->second.to_indirect_reference(context));
                        }

                    } else if (auto reference = std::get_if<Reference>(&arguments)) {
                        function_definition->body = std::make_shared<Parser::Symbol>("#cached");

                        object->functions.push_front(CustomFunction{function_definition});
                        object->functions.back().extern_symbols.emplace("#cached", reference->to_indirect_reference(context));
                    }

                    function_context.add_symbol(symbol->name, Reference(object));
                    return;
                }
            }
            std::list<Function> functions;
            try {
                functions = Interpreter::execute(context, p_function->function).to_data(context).get<Object*>()->functions;
            } catch (Data::BadAccess const& e) {}

            Arguments args;
            if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                auto it = computed.find(*expression);
                args = it != computed.end() ? it->second : arguments;
            } else {
                args = arguments;
            }

            auto reference = call_function(context, p_function, functions, args);
            set_arguments(context, function_context, computed, p_function->arguments, reference);
        } else if (auto p_property = std::dynamic_pointer_cast<Parser::Property>(parameters)) {
            auto reference = computed.compute(context, arguments);

            if (auto property_reference = std::get_if<PropertyReference>(&reference)) {
                if (p_property->name == property_reference->name) {
                    set_arguments(context, function_context, computed, p_property->object, Reference(Data(&property_reference->parent.get())));
                } else throw Interpreter::FunctionArgumentsError();
            } else throw Interpreter::FunctionArgumentsError();
        } else throw Interpreter::FunctionArgumentsError();
    }

    Reference call_function(Context & context, std::shared_ptr<Parser::Expression> expression, std::list<Function> const& functions, Arguments const& arguments) {
        Computed computed;

        for (auto const& function : functions) {
            try {
                FunctionContext function_context(context, expression);
                for (auto & symbol : function.extern_symbols)
                    function_context.add_symbol(symbol.first, symbol.second);

                if (auto custom_function = std::get_if<CustomFunction>(&function)) {
                    set_arguments(context, function_context, computed, (*custom_function)->parameters, arguments);

                    Data filter = true;
                    if ((*custom_function)->filter != nullptr)
                        filter = execute(function_context, (*custom_function)->filter).to_data(context);

                    try {
                        if (filter.get<bool>())
                            return Interpreter::execute(function_context, (*custom_function)->body);
                        else continue;
                    } catch (Data::BadAccess & e) {
                        throw FunctionArgumentsError();
                    }
                } else if (auto system_function = std::get_if<SystemFunction>(&function)) {
                    set_arguments(context, function_context, computed, system_function->parameters, arguments);

                    return system_function->pointer(function_context);
                } else return Reference();
            } catch (FunctionArgumentsError & e) {}
        }

        if (expression->position != nullptr) {
            expression->position->store_stack_trace(context);
            expression->position->notify_error(functions.empty() ? "Error: not a function" : "Error: incorrect function arguments");
        }
        throw Error();
    }

    Reference execute(Context & context, std::shared_ptr<Parser::Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(expression)) {
            auto data = execute(context, function_call->function).to_data(context);
            std::list<Function> functions;
            try {
                functions = data.get<Object*>()->functions;
            } catch (Data::BadAccess const& e) {}
            return call_function(context, function_call, functions, function_call->arguments);
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
            auto object = context.new_object();
            object->functions.push_front(CustomFunction{function_definition});
            auto & f = object->functions.back();

            for (auto symbol : function_definition->symbols)
                if (context.has_symbol(symbol))
                    f.extern_symbols.emplace(symbol, context[symbol]);

            return Data(object);
        } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(expression)) {
            auto data = execute(context, property->object).to_data(context);
            try {
                auto object = data.get<Object*>();
                return (*object)[property->name];
            } catch (Data::BadAccess const& e) {
                return Data(context.new_object());
            }
        } else if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(expression)) {
            auto data = get_symbol(symbol->name);
            if (auto b = std::get_if<bool>(&data)) {
                return Data(*b);
            } else if (auto l = std::get_if<long>(&data)) {
                return Data(*l);
            } else if (auto d = std::get_if<double>(&data)) {
                return Data(*d);
            } else if (auto str = std::get_if<std::string>(&data)) {
                return context.new_object(*str);
            } else {
                return context[symbol->name];
            }
        } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(expression)) {
            TupleReference tuple_reference;
            for (auto e : tuple->objects)
                tuple_reference.push_back(execute(context, e));
            return tuple_reference;
        } else return Reference();
    }


    Reference run(Context & context, std::shared_ptr<Parser::Expression> expression) {
        try {
            return Interpreter::execute(context, expression);
        } catch (Base::Exception & ex) {
            ex.position->notify_error("An exception occured");
            return Reference(context.new_object());
        } catch (Error & e) {
            return Reference(context.new_object());
        }
    }

    bool print(Context & context, std::ostream & stream, Reference const& reference) {
        Data data = reference.to_data(context);

        if (auto c = std::get_if<char>(&data)) {
            std::cout << *c;
            return true;
        } else if (auto f = std::get_if<double>(&data)) {
            std::cout << *f;
            return true;
        } else if (auto i = std::get_if<long>(&data)) {
            std::cout << *i;
            return true;
        } else if (auto b = std::get_if<bool>(&data)) {
            std::cout << *b;
            return true;
        } else if (auto object = std::get_if<Object*>(&data)) {
            bool printed = false;
            for (size_t i = 0; i < (*object)->array.size(); ++i)
                if (print(context, stream, ArrayReference{**object, i})) printed = true;
            return printed;
        } else return false;
    }

}
