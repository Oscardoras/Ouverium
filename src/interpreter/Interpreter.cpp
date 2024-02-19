#include "Interpreter.hpp"

#include "system_functions/Array.hpp"
#include "system_functions/Base.hpp"
#include "system_functions/Dll.hpp"
#include "system_functions/Math.hpp"
#include "system_functions/System.hpp"
#include "system_functions/Types.hpp"
#include "system_functions/UI.hpp"


namespace Interpreter {

    GlobalContext::GlobalContext(std::shared_ptr<Parser::Expression> expression) :
        Context(expression) {
        system = new_object();
        SystemFunctions::Base::init(*this);
        SystemFunctions::Dll::init(*this);
        SystemFunctions::Array::init(*this);
        SystemFunctions::Math::init(*this);
        SystemFunctions::System::init(*this);
        SystemFunctions::Types::init(*this);
        SystemFunctions::UI::init(*this);
    }


    Exception::Exception(Context& context, Reference const& reference, std::shared_ptr<Parser::Expression> expression) :
        reference(reference) {
        if (expression->position != nullptr) {
            positions.push_back(expression->position);
            Context* old_c = nullptr;
            Context* c = &context;
            while (c != old_c) {
                if (c->expression->position != nullptr)
                    positions.push_back(c->expression->position);
                old_c = c;
                c = &c->get_parent();
            }
        }
    }

    Exception::Exception(Context& context, std::string const& message, Data const& type, std::shared_ptr<Parser::Expression> expression) :
        Exception(context, [&context, &message, &type]() {
        auto object = context.new_object(Object(message));
        auto types = context.new_object();
        types->array.push_back(type);
        object->properties["_types"] = types;
        return Data(object);
    }(), expression) {}

    void Exception::print_stack_trace(Context& context) const {
        if (!positions.empty()) {
            std::ostringstream oss;
            oss << "An exception occured: " << Interpreter::string_from(context, reference.to_data(context));
            positions.front()->notify_error(oss.str());
            for (auto const& p : positions)
                p->notify_position();
        }
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

        Reference compute(Context& context, Arguments const& arguments) {
            if (auto expression = std::get_if<ParserExpression>(&arguments))
                return operator[](*expression) = Interpreter::execute(context, *expression);
            else if (auto reference = std::get_if<Reference>(&arguments))
                return *reference;
            else return *((Reference*) nullptr);
        }

    };

    void set_arguments(Context& context, FunctionContext& function_context, Computed& computed, std::shared_ptr<Parser::Expression> parameters, Arguments const& argument) {
        auto arguments = computed.get(argument);

        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            auto reference = computed.compute(context, arguments);

            if (function_context.has_symbol(symbol->name)) {
                if (reference != Reference(function_context[symbol->name]))
                    throw Interpreter::FunctionArgumentsError();
            } else {
                function_context.add_symbol(symbol->name, reference);
            }
        } else if (auto p_tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                if (auto a_tuple = std::dynamic_pointer_cast<Parser::Tuple>(*expression)) {
                    if (p_tuple->objects.size() == a_tuple->objects.size()) {
                        for (size_t i = 0; i < p_tuple->objects.size(); ++i)
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
                        for (size_t i = 0; i < p_tuple->objects.size(); ++i)
                            set_arguments(context, function_context, computed, p_tuple->objects[i], (*tuple_reference)[i]);
                    } else throw Interpreter::FunctionArgumentsError();
                } else {
                    try {
                        auto object = reference->to_data(function_context).get<Object*>();
                        if (object->array.size() == p_tuple->objects.size()) {
                            for (size_t i = 0; i < p_tuple->objects.size(); ++i)
                                set_arguments(context, function_context, computed, p_tuple->objects[i], ArrayReference{ *object, i });
                        } else throw Interpreter::FunctionArgumentsError();
                    } catch (Data::BadAccess const&) {
                        throw Interpreter::FunctionArgumentsError();
                    }
                }
            }
        } else if (auto p_function = std::dynamic_pointer_cast<Parser::FunctionCall>(parameters)) {
            if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(p_function->function); symbol && !function_context.has_symbol(symbol->name)) {
                Object* object = context.new_object();
                auto function_definition = std::make_shared<Parser::FunctionDefinition>();
                function_definition->parameters = p_function->arguments;

                if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                    auto it = computed.find(*expression);
                    if (it == computed.end()) {
                        function_definition->body = *expression;

                        object->functions.push_front(CustomFunction{ function_definition });
                        auto& f = object->functions.back();
                        for (auto symbol : function_definition->body->symbols)
                            f.extern_symbols.emplace(symbol, context[symbol]);
                    } else {
                        if (auto tuple_reference = std::get_if<TupleReference>(&it->second)) {
                            auto tuple = std::make_shared<Parser::Tuple>();
                            for (size_t i = 0; i < tuple_reference->size(); ++i)
                                tuple->objects.push_back(std::make_shared<Parser::Symbol>("#cached" + std::to_string(i)));
                            function_definition->body = tuple;

                            object->functions.push_front(CustomFunction{ function_definition });
                            for (size_t i = 0; i < tuple_reference->size(); ++i)
                                object->functions.back().extern_symbols.emplace("#cached" + std::to_string(i), (*tuple_reference)[i].to_indirect_reference(context));
                        } else {
                            function_definition->body = std::make_shared<Parser::Symbol>("#cached");

                            object->functions.push_front(CustomFunction{ function_definition });
                            object->functions.back().extern_symbols.emplace("#cached", it->second.to_indirect_reference(context));
                        }
                    }

                } else if (auto reference = std::get_if<Reference>(&arguments)) {
                    if (auto tuple_reference = std::get_if<TupleReference>(reference)) {
                        auto tuple = std::make_shared<Parser::Tuple>();
                        for (size_t i = 0; i < tuple_reference->size(); ++i)
                            tuple->objects.push_back(std::make_shared<Parser::Symbol>("#cached" + std::to_string(i)));
                        function_definition->body = tuple;

                        object->functions.push_front(CustomFunction{ function_definition });
                        for (size_t i = 0; i < tuple_reference->size(); ++i)
                            object->functions.back().extern_symbols.emplace("#cached" + std::to_string(i), (*tuple_reference)[i].to_indirect_reference(context));
                    } else {
                        function_definition->body = std::make_shared<Parser::Symbol>("#cached");

                        object->functions.push_front(CustomFunction{ function_definition });
                        object->functions.back().extern_symbols.emplace("#cached", reference->to_indirect_reference(context));
                    }
                }

                function_context.add_symbol(symbol->name, Reference(Data(object)));
            } else {
                auto r = execute(function_context, p_function->function).to_data(context);

                Arguments args;
                if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                    auto it = computed.find(*expression);
                    args = it != computed.end() ? it->second : arguments;
                } else {
                    args = arguments;
                }

                Reference reference;
                try {
                    reference = call_function(context, p_function, r, args);
                } catch (Exception const&) {
                    throw Interpreter::FunctionArgumentsError();
                }
                set_arguments(context, function_context, computed, p_function->arguments, reference);
            }
        } else if (auto p_property = std::dynamic_pointer_cast<Parser::Property>(parameters)) {
            auto reference = computed.compute(context, arguments);

            if (auto property_reference = std::get_if<PropertyReference>(&reference)) {
                if (p_property->name == property_reference->name || p_property->name == ".") {
                    set_arguments(context, function_context, computed, p_property->object, Reference(Data(&property_reference->parent.get())));
                } else throw Interpreter::FunctionArgumentsError();
            } else throw Interpreter::FunctionArgumentsError();
        } else throw Interpreter::FunctionArgumentsError();
    }

    Reference call_function(Context& context, std::shared_ptr<Parser::Expression> expression, Reference const& func, Arguments const& arguments) {
        if (context.get_recurion_level() >= context.get_global().recursion_limit)
            throw Exception(context, "recursion limit exceeded", context.get_global()["RecursionLimitExceeded"].to_data(context), expression);

        std::list<Function> functions;
        try {
            functions = func.to_data(context).get<Object*>()->functions;
        } catch (Data::BadAccess const&) {}

        try {
            if (functions.empty())
                functions = call_function(context, context.expression, context.get_global()["getter"], func).to_data(context).get<Object*>()->functions;
        } catch (Data::BadAccess const&) {}

        Computed computed;

        for (auto const& function : functions) {
            try {
                FunctionContext function_context(context, expression);
                for (auto& symbol : function.extern_symbols)
                    function_context.add_symbol(symbol.first, symbol.second);

                if (auto custom_function = std::get_if<CustomFunction>(&function)) {
                    set_arguments(context, function_context, computed, (*custom_function)->parameters, arguments);

                    Data filter = Data(true);
                    if ((*custom_function)->filter != nullptr)
                        filter = execute(function_context, (*custom_function)->filter).to_data(context);

                    try {
                        if (filter.get<bool>())
                            return Interpreter::execute(function_context, (*custom_function)->body);
                        else continue;
                    } catch (Data::BadAccess const&) {
                        throw FunctionArgumentsError();
                    }
                } else if (auto system_function = std::get_if<SystemFunction>(&function)) {
                    set_arguments(context, function_context, computed, system_function->parameters, arguments);

                    return system_function->pointer(function_context);
                } else return Reference();
            } catch (FunctionArgumentsError&) {}
        }

        if (functions.empty())
            throw Exception(context, "not a function", context.get_global()["NotAFunction"].to_data(context), expression);
        else
            throw Exception(context, "incorrect function arguments", context.get_global()["IncorrectFunctionArguments"].to_data(context), expression);
    }

    Reference execute(Context& context, std::shared_ptr<Parser::Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(expression)) {
            auto reference = execute(context, function_call->function);

            return call_function(context, function_call, reference, function_call->arguments);
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
            auto object = context.new_object();
            object->functions.push_front(CustomFunction{ function_definition });
            auto& f = object->functions.back();

            for (auto symbol : function_definition->captures)
                f.extern_symbols.emplace(symbol, context[symbol]);

            return Data(object);
        } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(expression)) {
            auto data = execute(context, property->object).to_data(context);
            try {
                auto object = data.get<Object*>();
                return (*object)[property->name];
            } catch (Data::BadAccess const&) {
                return Data{};
            }
        } else if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(expression)) {
            auto data = get_symbol(symbol->name);
            if (auto b = std::get_if<bool>(&data)) {
                return Data(*b);
            } else if (auto l = std::get_if<OV_INT>(&data)) {
                return Data(*l);
            } else if (auto d = std::get_if<OV_FLOAT>(&data)) {
                return Data(*d);
            } else if (auto str = std::get_if<std::string>(&data)) {
                return Data(context.new_object(*str));
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


    Reference set(Context& context, Reference const& var, Reference const& data) {
        return call_function(context, context.expression, context.get_global()["setter"], TupleReference{ var, data });
    }

    std::string string_from(Context& context, Reference const& data) {
        std::ostringstream oss;

        auto d = call_function(context, context.expression, context.get_global()["string_from"], data).to_data(context);
        try {
            oss << d.get<Object*>()->to_string();
        } catch (Data::BadAccess const&) {}

        return oss.str();
    }

}
