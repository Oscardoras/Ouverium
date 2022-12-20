#include <algorithm>
#include <map>
#include <stdexcept>

#include "Analyzer.hpp"


namespace Analyzer {

    ObjectPtr& Object::get_property(Context & context, std::string name) {
        auto & field = properties[name];
        if (field == nullptr) field = context.new_object();
        return field;
    }

    Reference::Reference(ObjectPtr ptr):
    std::variant<ObjectPtr, std::reference_wrapper<ObjectPtr>, std::vector<Reference>>(ptr) {}

    Reference::Reference(std::reference_wrapper<ObjectPtr> reference):
    std::variant<ObjectPtr, std::reference_wrapper<ObjectPtr>, std::vector<Reference>>(reference) {}

    Reference::Reference(std::vector<Reference> const& tuple):
    std::variant<ObjectPtr, std::reference_wrapper<ObjectPtr>, std::vector<Reference>>(tuple) {}

    ObjectPtr Reference::to_object(Context & context) const {
        if (auto reference = std::get_if<std::reference_wrapper<ObjectPtr>>(this))
            return reference->get();
        else if (auto tuple = std::get_if<std::vector<Reference>>(this)) {
            std::vector<ObjectPtr> array;
            for (auto o : *tuple)
                array.push_back(o.to_object(context));
            return context.new_object(array);
        } else return nullptr;
    }

    ObjectPtr& Reference::to_reference(Context & context) const {
        if (auto reference = std::get_if<std::reference_wrapper<ObjectPtr>>(this))
            return reference->get();
        else if (auto tuple = std::get_if<std::vector<Reference>>(this)) {
            return context.new_reference(to_object(context));
        } else return *((ObjectPtr*) nullptr);
    }

    Context::Context(GlobalContext& global): global(global) {}

    GlobalContext& Context::get_global() const {
        return this->global.get();
    }

    ObjectPtr Context::new_object() {
        auto & objects = get_global().objects;
        objects.push_back(Object());
        return ObjectPtr(&objects.back());
    }

    ObjectPtr Context::new_object(Data const& data) {
        auto & objects = get_global().objects;
        objects.push_back(Object());
        objects.back().data = data;
        return ObjectPtr(&objects.back());
    }

    ObjectPtr& Context::new_reference(ObjectPtr object) {
        get_global().references.push_back(object);
        return get_global().references.back();
    }

    ObjectPtr& Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end()) {
            symbols.emplace(symbol, new_reference(new_object()));
        }

        return symbols.at(symbol);
    }

    bool Context::has_symbol(std::string const& symbol) {
        return symbols.find(symbol) != symbols.end();
    }


    void set_references(Context & function_context, std::shared_ptr<Expression> parameters, Reference const& reference) {
        if (auto symbol = std::dynamic_pointer_cast<Symbol>(parameters)) {
            function_context[symbol->name] = reference.to_reference(function_context);
        } else if (auto tuple = std::dynamic_pointer_cast<Tuple>(parameters)) {
            if (auto reference_tuple = std::get_if<std::vector<Reference>>(&reference)) {
                if (reference_tuple->size() == tuple->objects.size())
                    for (size_t i = 0; i < tuple->objects.size(); i++)
                        set_references(function_context, tuple->objects[i], (*reference_tuple)[i]);
                else throw FunctionArgumentsError();
            } else {
                auto object = reference.to_object(function_context);
                if (auto data_tuple = std::get_if<std::vector<ObjectPtr>>(&object->data)) {
                    if (data_tuple->size() == tuple->objects.size()) {
                        for (size_t i = 0; i < tuple->objects.size(); i++)
                            set_references(function_context, tuple->objects[i], (*data_tuple)[i]);
                    } else throw FunctionArgumentsError();
                } else throw FunctionArgumentsError();
            }
        } else throw FunctionArgumentsError();
    }

    void set_references(Context & context, Context & function_context, std::map<std::shared_ptr<Expression>, Reference> & computed, std::shared_ptr<Expression> parameters, std::shared_ptr<Expression> arguments) {
        if (auto symbol = std::dynamic_pointer_cast<Symbol>(parameters)) {
            auto it = computed.find(arguments);
            auto reference = it != computed.end() ? it->second : (computed[arguments] = execute(context, arguments));
            function_context[symbol->name] = reference.to_reference(function_context);
        } else if (auto p_tuple = std::dynamic_pointer_cast<Tuple>(parameters)) {
            if (auto a_tuple = std::dynamic_pointer_cast<Tuple>(arguments)) {
                if (p_tuple->objects.size() == a_tuple->objects.size()) {
                    for (size_t i = 0; i < p_tuple->objects.size(); i++)
                        set_references(context, function_context, computed, p_tuple->objects[i], a_tuple->objects[i]);
                } else throw FunctionArgumentsError();
            } else {
                auto it = computed.find(arguments);
                auto reference = it != computed.end() ? it->second : (computed[arguments] = execute(context, arguments));
                set_references(function_context, parameters, reference);
            }
        } else if (auto p_function = std::dynamic_pointer_cast<FunctionCall>(parameters)) {
            if (auto symbol = std::dynamic_pointer_cast<Symbol>(p_function->function)) {
                auto it = computed.find(arguments);
                if (it != computed.end())
                    throw FunctionArgumentsError();

                auto function_definition = std::make_shared<FunctionDefinition>();
                function_definition->parameters = p_function->arguments;
                function_definition->body = arguments;

                auto object = context.new_object();
                auto f = std::make_shared<Function>(function_definition);
                for (auto symbol : function_definition->body->symbols)
                    f->extern_symbols[symbol] = context[symbol];
                object->functions.push_front(f);

                function_context[symbol->name] = context.new_reference(object);
            } else throw FunctionArgumentsError();
        } else throw FunctionArgumentsError();
    }

    Reference call_function(Context & context, std::shared_ptr<Parser::Position> position, std::list<std::shared_ptr<Function>> const& functions, std::shared_ptr<Expression> arguments) {
        std::map<std::shared_ptr<Expression>, Reference> computed;

        for (auto const& function : functions) {
            try {
                Context function_context(context);
                for (auto & symbol : function->extern_symbols)
                    function_context[symbol.first] = symbol.second;

                if (auto custom = std::get_if<std::shared_ptr<FunctionDefinition>>(&function->ptr)) {
                    set_references(context, function_context, computed, (*custom)->parameters, arguments);

                    ObjectPtr filter;
                    if ((*custom)->filter != nullptr)
                        filter = execute(function_context, (*custom)->filter).to_object(context);
                    else filter = nullptr;

                    bool* r;
                    if (filter != nullptr || ((r = std::get_if<bool>(&filter->data)) && r))
                        return execute(function_context, (*custom)->body);
                    else throw FunctionArgumentsError();
                } else if (auto system = std::get_if<SystemFunction>(&function->ptr)) {
                    set_references(context, function_context, computed, (*system).parameters, arguments);

                    return (*system).pointer(function_context);
                }

            } catch (FunctionArgumentsError & e) {}
        }

        if (position != nullptr)
            position->notify_error("The arguments given to the function don't match");
    }

    Reference execute(Context & context, std::shared_ptr<Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<FunctionCall>(expression)) {
            auto func = execute(context, function_call->function).to_object(context);
            return call_function(context, function_call->position, func->functions, function_call->arguments);
        } else if (auto function_definition = std::dynamic_pointer_cast<FunctionDefinition>(expression)) {
            auto object = context.new_object();
            auto f = std::make_shared<Function>(function_definition);
            for (auto symbol : function_definition->body->symbols)
                if (context.has_symbol(symbol))
                    f->extern_symbols[symbol] = context[symbol];
            if (function_definition->filter != nullptr)
                for (auto symbol : function_definition->filter->symbols)
                    if (context.has_symbol(symbol))
                        f->extern_symbols[symbol] = context[symbol];
            object->functions.push_front(f);

            return object;
        } else if (auto property = std::dynamic_pointer_cast<Property>(expression)) {
            auto object = execute(context, property->object).to_object(context);
            return std::reference_wrapper<ObjectPtr>(object->get_property(context, property->name));
        } else if (auto symbol = std::dynamic_pointer_cast<Symbol>(expression)) {
            if (symbol->name[0] == '\"') {
                std::string str;

                bool escape = false;
                bool first = true;
                for (char c : symbol->name) if (!first) {
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
            if (symbol->name == "true") return context.new_object(true);
            if (symbol->name == "false") return context.new_object(false);
            try {
                return context.new_object(std::stol(symbol->name));
            } catch (std::invalid_argument const& ex1) {
                try {
                    return context.new_object(std::stod(symbol->name));
                } catch (std::invalid_argument const& ex2) {
                    return std::reference_wrapper(context[symbol->name]);
                }
            }
        } else if (auto tuple = std::dynamic_pointer_cast<Tuple>(expression)) {
            std::vector<Reference> v;
            for (auto e : tuple->objects)
                v.push_back(execute(context, e));
            return v;
        } else return context.new_object();
    }

    /*
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
    */

}
