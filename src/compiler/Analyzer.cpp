#include <algorithm>
#include <map>
#include <stdexcept>

#include "Analyzer.hpp"

#include "../Utils.hpp"


namespace Analyzer {

    M<Data> M<SymbolReference>::to_data() const {
        M<Data> m;
        for (auto & reference : *this)
            for (auto & data : reference.get())
                m.insert(data);
        return m;
    }

    M<Data> M<Reference>::to_data(Context & context) const {
        M<Data> m;
        for (auto const& e : *this) {
            auto data = e.to_data(context);
            m.insert(data.begin(), data.end());
        }
        return m;
    }

    M<SymbolReference> M<Reference>::to_symbol_reference(Context & context) const {
        M<SymbolReference> m;
        for (auto const& e : *this)
            m.insert(e.to_symbol_reference(context));
        return m;
    }

    std::reference_wrapper<M<Data>> Object::get_property(Context & context, std::string name) {
        auto & field = properties[name];
        if (field.empty())
            field.insert(context.new_object());

        return field;
    }

    M<Data> Reference::to_data(Context & context) const {
        if (auto object_ptr = std::get_if<M<Data>>(this))
            return *object_ptr;
        else if (auto reference = std::get_if<SymbolReference>(this))
            return reference->get();
        else if (auto tuple = std::get_if<std::vector<M<Reference>>>(this)) {
            std::vector<M<Data>> array;
            for (auto o : *tuple)
                array.push_back(o.to_data(context));
            return Data(context.new_object(array));
        } else return Data(nullptr);
    }

    SymbolReference Reference::to_symbol_reference(Context & context) const {
        if (auto data = std::get_if<M<Data>>(this))
            return context.new_reference(*data);
        else if (auto reference = std::get_if<SymbolReference>(this))
            return *reference;
        else if (auto tuple = std::get_if<std::vector<M<Reference>>>(this)) {
            return context.new_reference(to_data(context));
        } else return *((M<Data>*) nullptr);
    }

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

    Object* Context::new_object(std::string const& str) {
        long l = str.length();
        auto & objects = get_global().objects;
        objects.push_back(Object());
        auto object = &objects.back();
        for (auto c : str)
            object->array.push_back(Data(c));
        return object;
    }

    SymbolReference Context::new_reference(M<Data> data) {
        get_global().references.push_back(data);
        return get_global().references.back();
    }

    M<SymbolReference>& Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            symbols.emplace(symbol, new_reference(Data(new_object())));

        return symbols.at(symbol);
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
            auto & lvalue = function_context[symbol->name];
            auto rvalue = reference.to_symbol_reference(function_context);
            lvalue.insert(rvalue.begin(), rvalue.end());
        } else if (auto tuple = std::dynamic_pointer_cast<Tuple>(parameters)) {
            bool success = false;
            for (auto ref : reference) {
                if (auto reference_tuple = std::get_if<std::vector<M<Reference>>>(&ref)) {
                    if (reference_tuple->size() == tuple->objects.size())
                        for (size_t i = 0; i < tuple->objects.size(); i++)
                            set_references(function_context, tuple->objects[i], (*reference_tuple)[i]);
                    else continue;
                } else {
                    auto data = ref.to_data(function_context);
                    for (auto d : data) {
                        if (auto object = std::get_if<Object*>(&d)) {
                            if ((*object)->array.size() == tuple->objects.size()) {
                                for (size_t i = 0; i < tuple->objects.size(); i++) {
                                    set_references(function_context, tuple->objects[i], (*object)->array[i]);
                                }
                            } else continue;
                        } else continue;
                    }
                }
                success = true;
            }
            if (!success)
                throw FunctionArgumentsError();
        } else throw FunctionArgumentsError();
    }

    void set_references(Context & context, bool potential, Context & function_context, std::map<std::shared_ptr<Expression>, M<Reference>> & computed, std::shared_ptr<Expression> parameters, std::shared_ptr<Expression> arguments) {
        if (auto symbol = std::dynamic_pointer_cast<Symbol>(parameters)) {
            auto it = computed.find(arguments);
            auto reference = it != computed.end() ? it->second : (computed[arguments] = execute(context, potential, arguments));
            auto & lvalue = function_context[symbol->name];
            auto rvalue = reference.to_symbol_reference(context);
            lvalue.insert(rvalue.begin(), rvalue.end());
        } else if (auto p_tuple = std::dynamic_pointer_cast<Tuple>(parameters)) {
            if (auto a_tuple = std::dynamic_pointer_cast<Tuple>(arguments)) {
                if (p_tuple->objects.size() == a_tuple->objects.size()) {
                    for (size_t i = 0; i < p_tuple->objects.size(); i++)
                        set_references(context, potential, function_context, computed, p_tuple->objects[i], a_tuple->objects[i]);
                } else throw FunctionArgumentsError();
            } else {
                auto it = computed.find(arguments);
                auto reference = it != computed.end() ? it->second : (computed[arguments] = execute(context, potential, arguments));
                set_references(function_context, parameters, reference);
            }
        } else if (auto p_function = std::dynamic_pointer_cast<FunctionCall>(parameters)) {
            if (auto symbol = std::dynamic_pointer_cast<Symbol>(p_function->function)) {
                auto function_definition = std::make_shared<FunctionDefinition>();
                function_definition->parameters = p_function->arguments;
                function_definition->body = arguments;

                auto object = context.new_object();
                Function f = {function_definition};
                for (auto symbol : function_definition->body->symbols)
                    f.extern_symbols[symbol] = context[symbol];
                object->functions.push_front(f);

                function_context[symbol->name] = SymbolReference(context.new_reference(Data(object)));
            } else throw FunctionArgumentsError();
        } else throw FunctionArgumentsError();
    }

    using It = std::map<std::string, M<SymbolReference>>::iterator;

    void call_reference(M<Reference> & result, bool potential, std::shared_ptr<Analyzer::Function> function, Context function_context, It const it, It const end) {
        if (it != end) {
            bool success = false;
            for (auto const& m1 : it->second) {
                function_context[it->first] = SymbolReference(m1.get());
                It copy = it;
                try {
                    call_reference(result, potential, function, function_context, copy++, end);
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
                    auto f = execute(function_context, potential, (*custom)->filter);
                    for (auto reference : f)
                        for (auto data : reference.to_data(function_context)) {
                            if (auto c = std::get_if<bool>(&data)) {
                                if (*c) filter = true;
                            } else {
                                if ((*custom)->filter->position != nullptr)
                                    (*custom)->filter->position->notify_error("The expression must be a boolean");
                            }
                        }
                } else filter = true;

                if (filter)
                    r = execute(function_context, potential, (*custom)->body);
                else throw FunctionArgumentsError();
            } else if (auto system = std::get_if<SystemFunction>(&function->ptr))
                r = (*system).pointer(function_context, potential);

            result.insert(r.begin(), r.end());
        }
    }

    M<Reference> call_function(Context & context, bool potential, std::shared_ptr<Parser::Position> position, std::list<std::shared_ptr<Function>> const& functions, M<Reference> const& arguments) {
        for (auto const& function : functions) {
            try {
                Context function_context(context);
                for (auto & symbol : function->extern_symbols)
                    function_context[symbol.first] = symbol.second;

                if (auto custom = std::get_if<std::shared_ptr<FunctionDefinition>>(&function->ptr))
                    set_references(context, (*custom)->parameters, arguments);
                else if (auto system = std::get_if<SystemFunction>(&function->ptr))
                    set_references(context, (*system).parameters, arguments);

                M<Reference> result;
                call_reference(result, potential, function, function_context, function_context.begin(), function_context.end());
                return result;
            } catch (FunctionArgumentsError & e) {}
        }

        if (position != nullptr)
            position->notify_error("The arguments given to the function don't match");
        return Reference(Data(context.new_object()));
    }

    M<Reference> call_function(Context & context, bool potential, std::shared_ptr<Parser::Position> position, std::list<std::shared_ptr<Function>> const& functions, std::shared_ptr<Expression> arguments) {
        std::map<std::shared_ptr<Expression>, M<Reference>> computed;

        for (auto const& function : functions) {
            try {
                Context function_context(context);
                for (auto & symbol : function->extern_symbols)
                    function_context[symbol.first] = symbol.second;

                if (auto custom = std::get_if<std::shared_ptr<FunctionDefinition>>(&function->ptr))
                    set_references(context, potential, function_context, computed, (*custom)->parameters, arguments);
                else if (auto system = std::get_if<SystemFunction>(&function->ptr))
                    set_references(context, potential, function_context, computed, (*system).parameters, arguments);

                M<Reference> result;
                call_reference(result, potential, function, function_context, function_context.begin(), function_context.end());
                return result;
            } catch (FunctionArgumentsError & e) {}
        }

        if (position != nullptr)
            position->notify_error("The arguments given to the function don't match");
        return Reference(Data(context.new_object()));
    }

    M<Reference> execute(Context & context, bool potential, std::shared_ptr<Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<FunctionCall>(expression)) {
            M<Reference> m;

            auto f = execute(context, potential, function_call->function);

            for (auto reference : f) {
                auto data = reference.to_data(context);
                for (auto d : data) {
                    std::list<Function> functions;
                    if (auto object = std::get_if<Object*>(&d))
                        functions = (*object)->functions;
                    auto result = call_function(context, potential, function_call->position, functions, function_call->arguments);
                    m.insert(result.begin(), result.end());
                }
            }

            return m;
        } else if (auto function_definition = std::dynamic_pointer_cast<FunctionDefinition>(expression)) {
            auto object = context.new_object();
            Function f = {function_definition};
            for (auto symbol : function_definition->body->symbols)
                if (context.has_symbol(symbol))
                    f.extern_symbols[symbol] = context[symbol];
            if (function_definition->filter != nullptr)
                for (auto symbol : function_definition->filter->symbols)
                    if (context.has_symbol(symbol))
                        f.extern_symbols[symbol] = context[symbol];
            object->functions.push_front(f);

            return Reference(Data(object));
        } else if (auto property = std::dynamic_pointer_cast<Property>(expression)) {
            M<Reference> m;

            auto r = execute(context, potential, property->object);
            for (auto reference : r) {
                auto data = reference.to_data(context);
                for (auto d : data) {
                    if (auto object = std::get_if<Object*>(&d)) {
                        auto field = (*object)->get_property(context, property->name);
                        m.insert(field.get().begin(), field.get().end());
                    } else
                        m.insert(M<Data>(context.new_object()));
                }
            }

            return m;
        } else if (auto symbol = std::dynamic_pointer_cast<Symbol>(expression)) {
            auto data = get_symbol(symbol->name);
            if (auto b = std::get_if<bool>(&data)) {
                return M<Data>(*b);
            } else if (auto l = std::get_if<long>(&data)) {
                return M<Data>(*l);
            } else if (auto d = std::get_if<double>(&data)) {
                return M<Data>(*d);
            } else if (auto str = std::get_if<std::string>(&data)) {
                return M<Data>(context.new_object(*str));
            } else {
                return context[symbol->name];
            }
        } else if (auto tuple = std::dynamic_pointer_cast<Tuple>(expression)) {
            std::vector<M<Reference>> v;
            for (auto e : tuple->objects)
                v.push_back(execute(context, potential, e));
            return Reference(v);
        } else return M<Data>(context.new_object());
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
