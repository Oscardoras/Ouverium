#include <algorithm>
#include <map>
#include <stdexcept>

#include "Translator.hpp"


namespace CTranslator {

    void iterate(std::shared_ptr<Expression> const& expression, std::vector<FunctionEnvironment> & functions, std::vector<std::string> defined_symbols) {
        if (expression->type == Expression::FunctionDefinition) {
            auto function_definition = std::static_pointer_cast<FunctionDefinition>(expression);

            functions.push_back(FunctionEnvironment {});
            auto & function = functions.back();

            function.expression = function_definition;

            for (auto const& s : function_definition->parameters->symbols)
                if (std::find(defined_symbols.begin(), defined_symbols.end(), s) == defined_symbols.end()) {
                    function.parameters.push_back(Symbol{ .name = s });
                    defined_symbols.push_back(s);
                }

            if (function_definition->filter != nullptr)
                for (auto const& s : function_definition->filter->symbols)
                    if (std::find(defined_symbols.begin(), defined_symbols.end(), s) == defined_symbols.end()) {
                        function.local_variables.push_back(Symbol{ .name = s });
                        defined_symbols.push_back(s);
                    }

            for (auto const& s : function_definition->object->symbols)
                if (std::find(defined_symbols.begin(), defined_symbols.end(), s) == defined_symbols.end()) {
                    function.local_variables.push_back(Symbol{ .name = s });
                    defined_symbols.push_back(s);
                }
        } else if (expression->type == Expression::FunctionCall) {
            auto function_call = std::static_pointer_cast<FunctionCall>(expression);

            iterate(function_call->function, functions, defined_symbols);
            iterate(function_call->object, functions, defined_symbols);
        } else if (expression->type == Expression::Property) {
            auto property = std::static_pointer_cast<Property>(expression);

            iterate(property->object, functions, defined_symbols);
        } else if (expression->type == Expression::Property) {
            auto property = std::static_pointer_cast<Property>(expression);

            iterate(property->object, functions, defined_symbols);
        } else if (expression->type == Expression::Tuple) {
            auto tuple = std::static_pointer_cast<Tuple>(expression);

            for (auto const& o : tuple->objects) {
                iterate(o, functions, defined_symbols);
            }
        }
    }

    GlobalEnvironment get_environment(std::shared_ptr<Expression> tree) {
        GlobalEnvironment env;

        for (auto const& s : tree->symbols) {
            env.global_variables.push_back(Symbol{ .name = s });
        }

        auto defined_symbols = tree->symbols;
        iterate(tree, env.functions, defined_symbols);

        return env;
    }

    void set_references(Context & context, FunctionContext & function_context, std::map<std::shared_ptr<Expression>, Reference> & computed, std::shared_ptr<Expression> parameters, std::shared_ptr<Expression> arguments) {
        if (parameters->type == Expression::Symbol) {
            auto symbol = std::static_pointer_cast<Symbol>(parameters);

            auto it = computed.find(arguments);
            auto reference = it != computed.end() ? it->second : (computed[arguments] = Interpreter::execute(context, arguments));
            function_context.add_symbol(symbol->name, reference);
        } else if (parameters->type == Expression::Tuple) {
            auto p_tuple = std::static_pointer_cast<Tuple>(parameters);

            if (arguments->type == Expression::Tuple) {
                auto a_tuple = std::static_pointer_cast<Tuple>(arguments);

                if (p_tuple->objects.size() == a_tuple->objects.size()) {
                    size_t size = p_tuple->objects.size();
                    for (size_t i = 0; i < size; i++)
                        set_references(context, function_context, computed, p_tuple->objects[i], a_tuple->objects[i]);
                } else throw Interpreter::FunctionArgumentsError();
            } else {
                auto it = computed.find(arguments);
                auto reference = it != computed.end() ? it->second : (computed[arguments] = Interpreter::execute(context, arguments));
                Interpreter::set_references(function_context, parameters, reference);
            }
        } else if (parameters->type == Expression::FunctionCall) {
            auto p_function = std::static_pointer_cast<FunctionCall>(parameters);

            if (p_function->function->type == Expression::Symbol) {
                auto it = computed.find(arguments);
                if (it != computed.end())
                    throw Interpreter::FunctionArgumentsError();

                auto symbol = std::static_pointer_cast<Symbol>(p_function->function);

                auto function_definition = std::make_shared<FunctionDefinition>();
                function_definition->parameters = p_function->object;
                function_definition->object = arguments;

                auto object = context.new_object();
                auto f = std::make_unique<CustomFunction>(function_definition);
                for (auto symbol : function_definition->object->symbols)
                    f->extern_symbols[symbol] = context.get_symbol(symbol);
                object->functions.push_front(std::move(f));

                function_context.add_symbol(symbol->name, Reference(object));
            } else throw Interpreter::FunctionArgumentsError();
        } else throw Interpreter::FunctionArgumentsError();
    }

    Reference execute(Context & context, std::shared_ptr<Expression> expression, Links & links) {
        if (expression->type == Expression::FunctionCall) {
            auto function_call = std::static_pointer_cast<FunctionCall>(expression);

            auto func = execute(context, function_call->function, links).to_object(context);

            std::map<std::shared_ptr<Expression>, Reference> computed;

            for (auto const& function : func->functions) {
                try {
                    FunctionContext function_context(context, function_call->position);
                    if (function_call->position != nullptr)
                        function_context.add_symbol("system_position", context.new_object(function_call->position->path));
                    for (auto & symbol : function->extern_symbols)
                        function_context.add_symbol(symbol.first, symbol.second);

                    if (function->type == Function::Custom) {
                        set_references(context, function_context, computed, ((CustomFunction*) function.get())->pointer->parameters, function_call->object);

                        Object* filter;
                        if (((CustomFunction*) function.get())->pointer->filter != nullptr)
                            filter = execute(function_context, ((CustomFunction*) function.get())->pointer->filter, links).to_object(context);
                        else filter = nullptr;

                        if (filter == nullptr || (filter->type == Object::Bool && filter->data.b)) {
                            auto r = execute(function_context, ((CustomFunction*) function.get())->pointer->object, links);
                            links[function_call] = ((CustomFunction*) function.get())->pointer;
                            return r;
                        } else throw Interpreter::FunctionArgumentsError();
                    } else {
                        set_references(context, function_context, computed, ((SystemFunction*) function.get())->parameters, function_call->object);

                        auto r = ((SystemFunction*) function.get())->pointer(function_context);
                        links[function_call] = ((SystemFunction*) function.get())->pointer;
                        return r;
                    }

                } catch (Interpreter::FunctionArgumentsError & e) {}
            }

            if (function_call->position != nullptr)
                function_call->position->notify_error();

            throw Interpreter::Error();
        } else if (expression->type == Expression::FunctionDefinition) {
            auto function_definition = std::static_pointer_cast<FunctionDefinition>(expression);

            auto object = context.new_object();
            auto f = std::make_unique<CustomFunction>(function_definition);
            for (auto symbol : function_definition->object->symbols)
                if (context.has_symbol(symbol))
                    f->extern_symbols[symbol] = context.get_symbol(symbol);
            if (function_definition->filter != nullptr)
                for (auto symbol : function_definition->filter->symbols)
                    if (context.has_symbol(symbol))
                        f->extern_symbols[symbol] = context.get_symbol(symbol);
            object->functions.push_front(std::move(f));

            return Reference(object);
        } else if (expression->type == Expression::Property) {
            auto property = std::static_pointer_cast<Property>(expression);

            auto object = execute(context, property->object, links).to_object(context);
            return Reference(object, &object->get_property(property->name, context));
        } else if (expression->type == Expression::Symbol) {
            auto symbol = std::static_pointer_cast<Symbol>(expression)->name;

            if (symbol[0] == '\"') {
                std::string str;

                bool escape = false;
                bool first = true;
                for (char c : symbol) if (!first) {
                    if (!escape) {
                        if (c == '\"') break;
                        else if (c == '\\') escape = true;
                        else str += c;
                    } else {
                        escape = false;
                        if (c == 'b') str += '\b';
                        if (c == 'e') str += '\e';
                        if (c == 'f') str += '\f';
                        if (c == 'n') str += '\n';
                        if (c == 'r') str += '\r';
                        if (c == 't') str += '\t';
                        if (c == 'v') str += '\v';
                        if (c == '\\') str += '\\';
                        if (c == '\'') str += '\'';
                        if (c == '\"') str += '\"';
                        if (c == '?') str += '\?';
                    }
                } else first = false;

                return context.new_object(str);
            }
            if (symbol == "true") return Reference(context.new_object(true));
            if (symbol == "false") return Reference(context.new_object(false));
            try {
                return Reference(context.new_object(std::stol(symbol)));
            } catch (std::invalid_argument const& ex1) {
                try {
                    return Reference(context.new_object(std::stod(symbol)));
                } catch (std::invalid_argument const& ex2) {
                    return context.get_symbol(symbol);
                }
            }
        } else if (expression->type == Expression::Tuple) {
            auto tuple = std::static_pointer_cast<Tuple>(expression);

            auto n = tuple->objects.size();
            Reference reference;
            if (n > 0) {
                reference = Reference(n);
                for (int i = 0; i < (int) n; i++)
                    reference.tuple[i] = execute(context, tuple->objects[i], links);
            } else reference = Reference(context.new_object());
            return reference;
        } else return Reference();
    }

    std::vector<std::shared_ptr<CStructures::Instruction>> get_instructions(std::shared_ptr<Expression> expression, Types & types, Links & links) {
        if (expression->type == Expression::FunctionCall) {
            auto function_call = std::static_pointer_cast<FunctionCall>(expression);

            auto & functions = links[function_call];
            if (functions.size() == 1) {

            }
        }
    }

    std::shared_ptr<CStructures::Expression> get_expression(std::shared_ptr<Expression> expression, Types & types, Links & links) {
        if (expression->type == Expression::FunctionCall) {
            auto function_call = std::static_pointer_cast<FunctionCall>(expression);

            auto & functions = links[function_call];
            if (functions.size() == 1) {
                try {
                    auto f = std::get<std::shared_ptr<FunctionDefinition>>(functions[0]);
                    return std::make_shared<CStructures::FunctionCall>(CStructures::FunctionCall {
                        .function = get_expression(function_call->function, types, links)
                    });
                } catch (std::bad_variant_access const& e) {

                }
            }
        } else if (expression->type == Expression::FunctionDefinition) {
            return std::make_shared<CStructures::FunctionDefinition>(CStructures::FunctionDefinition {
                .function = get_expression(function_call->function, types, links)
            });
        } else if (expression->type == Expression::Property) {
            auto property = std::static_pointer_cast<Property>(expression);

            auto o = get_expression(property->object, types, links);
            return std::make_shared<CStructures::Property>(CStructures::Property {
                .object = o,
                .name = property->name,
                .pointer = types[property]->pointer
            });
        } else if (expression->type == Expression::Symbol) {
            auto symbol = std::static_pointer_cast<Symbol>(expression);

            return std::make_shared<CStructures::VariableCall>(CStructures::VariableCall {.name = symbol->name});
        } else if (expression->type == Expression::Tuple) {
            auto tuple = std::static_pointer_cast<Tuple>(expression);

            auto list = std::make_shared<CStructures::List>(CStructures::List {});
            for (auto const& o : tuple->objects)
                list->objects.push_back(get_expression(o, types, links));
        }
    }

}
