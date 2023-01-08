#include <algorithm>
#include <map>
#include <stdexcept>

#include "Analyzer.hpp"


namespace Analyzer {

    M<std::reference_wrapper<Data>> Object::get_property(Context & context, std::string name) {
        auto & field = properties[name];
        if (field.empty())
            field.push_back(context.new_object());

        return M<std::reference_wrapper<Data>>(field);
    }

    Data Reference::to_data(Context & context) const {
        if (auto object_ptr = std::get_if<Data>(this))
            return *object_ptr;
        else if (auto reference = std::get_if<std::reference_wrapper<Data>>(this))
            return reference->get();
        else if (auto tuple = std::get_if<std::vector<M<Reference>>>(this)) {
            std::vector<M<Data>> array;
            for (auto o : *tuple)
                array.push_back(Analyzer::to_data(o, context));
            return Data(context.new_object(array));
        } else return Data(nullptr);
    }

    Data& Reference::to_reference(Context & context) const {
        if (auto data = std::get_if<Data>(this))
            return context.new_reference(*data);
        else if (auto reference = std::get_if<std::reference_wrapper<Data>>(this))
            return reference->get();
        else if (auto tuple = std::get_if<std::vector<M<Reference>>>(this)) {
            return context.new_reference(to_data(context));
        } else return *((Data*) nullptr);
    }

    Context::Context(Context& parent): parent(parent) {}

    Context& Context::get_parent() {
        return this->parent.get();
    }

    GlobalContext& Context::get_global() {
        return this->parent.get().get_global();
    }

    Object* Context::new_object() {
        auto & objects = get_global().objects;
        objects.push_back(Object());
        return &objects.back();
    }

    Object* Context::new_object(std::vector<M<Data>> const& array) {
        auto & objects = get_global().objects;
        objects.push_back(Object());
        objects.back().array = array;
        return &objects.back();
    }

    Data& Context::new_reference(Data data) {
        get_global().references.push_back(data);
        return get_global().references.back();
    }

    M<std::reference_wrapper<Data>> Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            symbols.emplace(symbol, new_reference(Data(new_object())));

        return symbols.at(symbol);
    }

    M<Data> to_data(M<Reference> m, Context & context) {
        M<Data> r;
        for (auto ref : m)
            m.push_back(ref.to_data(context));
        return r;
    }

    M<std::reference_wrapper<Data>> to_reference(M<Reference> m, Context & context) {
        M<Data> r;
        for (auto ref : m)
            m.push_back(ref.to_reference(context));
        return r;
    }

    bool Context::has_symbol(std::string const& symbol) {
        return symbols.find(symbol) != symbols.end();
    }

    GlobalContext::GlobalContext():
    Context(*this) {}

    GlobalContext& GlobalContext::get_global() {
        return *this;
    }


    void set_references(Context & function_context, std::shared_ptr<Expression> parameters, M<Reference> const& reference) {
        if (auto symbol = std::dynamic_pointer_cast<Symbol>(parameters)) {
            function_context[symbol->name] = to_reference(reference, function_context);
        } else if (auto tuple = std::dynamic_pointer_cast<Tuple>(parameters)) {
            bool success = false;
            for (auto ref : reference) {
                if (auto reference_tuple = std::get_if<std::vector<M<Reference>>>(&ref)) {
                    if (reference_tuple->size() == tuple->objects.size())
                        for (size_t i = 0; i < tuple->objects.size(); i++)
                            set_references(function_context, tuple->objects[i], (*reference_tuple)[i]);
                    else continue;
                } else {
                    auto object_ptr = ref.to_data(function_context);
                    if (auto object = std::get_if<Object*>(&object_ptr)) {
                        if ((*object)->array.size() == tuple->objects.size()) {
                            for (size_t i = 0; i < tuple->objects.size(); i++) {
                                set_references(function_context, tuple->objects[i], (*object)->array[i]);
                            }
                        } else continue;
                    } else continue;
                }
                success = true;
            }
            if (!success)
                throw FunctionArgumentsError();
        } else throw FunctionArgumentsError();
    }

    void set_references(Context & context, Context & function_context, std::map<std::shared_ptr<Expression>, M<Reference>> & computed, std::shared_ptr<Expression> parameters, std::shared_ptr<Expression> arguments) {
        if (auto symbol = std::dynamic_pointer_cast<Symbol>(parameters)) {
            auto it = computed.find(arguments);
            auto reference = it != computed.end() ? it->second : (computed[arguments] = execute(context, arguments));
            function_context[symbol->name] = to_reference(reference, context);
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
                auto function_definition = std::make_shared<FunctionDefinition>();
                function_definition->parameters = p_function->arguments;
                function_definition->body = arguments;

                auto object = context.new_object();
                auto f = std::make_shared<Function>(function_definition);
                for (auto symbol : function_definition->body->symbols)
                    f->extern_symbols[symbol] = context[symbol];
                object->functions.push_front(f);

                function_context[symbol->name] = M<std::reference_wrapper<Data>>(context.new_reference(object));
            } else throw FunctionArgumentsError();
        } else throw FunctionArgumentsError();
    }

    using It =std::map<std::string, M<std::reference_wrapper<Data>>>::iterator;

    void call_reference(M<Reference> & result, std::shared_ptr<Analyzer::Function> function, Context function_context, It const it, It const end) {
        if (it != end) {
            bool success = false;
            for (auto const& m : it->second) {
                function_context[it->first] = m;
                It copy = it;
                try {
                    call_reference(result, function, function_context, copy++, end);
                    success = true;
                } catch (FunctionArgumentsError & e) {}
            }
            if (!success)
                throw FunctionArgumentsError();
        } else {
            M<Reference> r;

            if (auto custom = std::get_if<std::shared_ptr<FunctionDefinition>>(&function->ptr)) {
                bool filter = false;
                if ((*custom)->filter != nullptr) {
                    auto f = execute(function_context, (*custom)->filter);
                    for (auto reference : f)
                        if (auto c = std::get_if<bool>(&reference.to_data(function_context))) {
                            if (*c) filter = true;
                        } else {
                            if ((*custom)->filter->position != nullptr)
                                (*custom)->filter->position->notify_error("The expression must be a boolean");
                        }
                } else filter = true;

                if (filter)
                    r = execute(function_context, (*custom)->body);
                else throw FunctionArgumentsError();
            } else if (auto system = std::get_if<SystemFunction>(&function->ptr))
                r = (*system).pointer(function_context);

            result.insert(result.end(), r.begin(), r.end());
        }
    }

    M<Reference> call_function(Context & context, std::shared_ptr<Parser::Position> position, std::list<std::shared_ptr<Function>> const& functions, std::shared_ptr<Expression> arguments) {
        std::map<std::shared_ptr<Expression>, M<Reference>> computed;

        for (auto const& function : functions) {
            try {
                Context function_context(context);
                for (auto & symbol : function->extern_symbols)
                    function_context[symbol.first] = symbol.second;

                if (auto custom = std::get_if<std::shared_ptr<FunctionDefinition>>(&function->ptr))
                    set_references(context, function_context, computed, (*custom)->parameters, arguments);
                else if (auto system = std::get_if<SystemFunction>(&function->ptr))
                    set_references(context, function_context, computed, (*system).parameters, arguments);

                M<Reference> result;
                call_reference(result, function, function_context, function_context.begin(), function_context.end());
                return result;
            } catch (FunctionArgumentsError & e) {}
        }

        if (position != nullptr)
            position->notify_error("The arguments given to the function don't match");
    }

    M<Reference> execute(Context & context, std::shared_ptr<Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<FunctionCall>(expression)) {
            M<Reference> m;

            auto f = execute(context, function_call->function);
            if (f.empty())
                f.push_back(Data(context.new_object()));

            for (auto reference : f) {
                auto data = reference.to_data(context);
                std::list<std::shared_ptr<Function>> functions;
                if (auto object = std::get_if<Object*>(&data))
                    functions = (*object)->functions;
                auto result = call_function(context, function_call->position, functions, function_call->arguments);
                m.insert(m.end(), result.begin(), result.end());
            }

            return m;
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

            return Reference(Data(object));
        } else if (auto property = std::dynamic_pointer_cast<Property>(expression)) {
            M<Reference> m;
            auto r = execute(context, property->object);
            for (auto reference : r) {
                auto data = reference.to_data(context);
                if (auto object = std::get_if<Object*>(&data)) {
                    auto field = (*object)->get_property(context, property->name);
                    m.insert(m.end(), field.begin(), field.end());
                } else
                    m.push_back(Data(context.new_object()));
            }

            return m;
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

                return M<Reference>(Data(context.new_object(str)));
            }
            if (symbol->name == "true") return M<Reference>(Data(true));
            if (symbol->name == "false") return M<Reference>(Data(false));
            try {
                return M<Reference>(Data(std::stol(symbol->name)));
            } catch (std::invalid_argument const& ex1) {
                try {
                    return M<Reference>(Data(std::stod(symbol->name)));
                } catch (std::invalid_argument const& ex2) {
                    return context[symbol->name];
                }
            }
        } else if (auto tuple = std::dynamic_pointer_cast<Tuple>(expression)) {
            std::vector<M<Reference>> v;
            for (auto e : tuple->objects)
                v.push_back(execute(context, e));
            return Reference(v);
        } else return Reference(Data(context.new_object()));
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
