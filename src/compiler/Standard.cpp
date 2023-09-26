#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>

#include "Standard.hpp"

#include "../Utils.hpp"


namespace Analyzer::Standard {

    M<Data> compute(Context & context, ObjectKey const& key, Reference const& reference, M<Data> const& datas) {
        M<Data> m;

        for (auto const& data : datas) {
            if (data == Data{})
                if (context.gettings.find(reference) == context.gettings.end()) {
                    context.gettings.insert(reference);
                    M<std::list<Function>> functions;
                    for (auto const& d : context.get_global()["getter"].to_data(context, key))
                        functions.add(d.get<Object*>()->functions);
                    auto r = call_function(context, functions, Arguments(key, reference)).to_data(context, key);
                    context.gettings.erase(reference);
                    m.add(r);
                } else {
                    Data d = context.new_object(key);

                    if (auto symbol_reference = std::get_if<SymbolReference>(&reference)) static_cast<M<Data> &>(*symbol_reference).add(d);
                    else if (auto property_reference = std::get_if<PropertyReference>(&reference)) static_cast<M<Data> &>(*property_reference).add(d);
                    else if (auto array_reference = std::get_if<ArrayReference>(&reference)) static_cast<M<Data> &>(*array_reference).add(d);

                    m.add(d);
                }
            else
                m.add(data);
        }

        return m;
    }

    M<Data> IndirectReference::to_data(Context & context, ObjectKey const& key) const {
        return compute(context, key, *this, std::visit([](auto const& arg) -> M<Data>& {
            return arg;
        }, *this));
    }

    M<Data> M<IndirectReference>::to_data(Context & context, ObjectKey const& key) const {
        M<Data> m;
        for (auto const& e : *this)
            m.add(e.to_data(context, key));
        return m;
    }

    M<Data> Reference::to_data(Context & context, ObjectKey const& key) const {
        auto get_data = [this, &context, &key]() -> M<Data> {
            if (auto data = std::get_if<M<Data>>(this))
                return *data;
            else if (auto symbol_reference = std::get_if<SymbolReference>(this))
                return *symbol_reference;
            else if (auto property_reference = std::get_if<PropertyReference>(this))
                return *property_reference;
            else if (auto array_reference = std::get_if<ArrayReference>(this))
                return *array_reference;
            else if (auto tuple_reference = std::get_if<TupleReference>(this)) {
                auto object = context.new_object(key);
                for (auto d : *tuple_reference)
                    object->array.add(d.to_data(context, key));
                return Data(object);
            } else return {};
        };

        return compute(context, key, *this, get_data());
    }

    M<Data> M<Reference>::to_data(Context & context, ObjectKey const& key) const {
        M<Data> m;
        for (auto const& e : *this)
            m.add(e.to_data(context, key));
        return m;
    }

    SymbolReference::operator M<Data> &() const {
        return context.symbol_references[symbol];
    }

    PropertyReference::operator M<Data> &() const {
        return parent->properties[name];
    }

    ArrayReference::operator M<Data> &() const {
        return array->array;
    }

    IndirectReference Object::operator[](std::string name) {
        return PropertyReference{this, name};
    }

    Object* Context::new_object(ObjectKey const& key) {
        auto & objects = get_global().objects;
        return &objects[key];
    }

    bool Context::has_symbol(std::string const& symbol) {
        return symbols.find(symbol) != symbols.end();
    }

    void Context::add_symbol(std::string const& symbol, M<Reference> const& reference) {
        auto create_symbol_reference = [this, &symbol](M<Data> const& data) -> SymbolReference {
            auto r = SymbolReference{ *this, symbol };
            static_cast<M<Data>&>(r).add(data);
            return r;
        };

        for (auto const& ref : reference) {
            if (auto data = std::get_if<M<Data>>(&ref))
                symbols[symbol].add(IndirectReference(create_symbol_reference(*data)));
            else if (auto symbol_reference = std::get_if<SymbolReference>(&ref))
                symbols[symbol].add(IndirectReference(*symbol_reference));
            else if (auto property_reference = std::get_if<PropertyReference>(&ref))
                symbols[symbol].add(IndirectReference(*property_reference));
            else if (auto array_reference = std::get_if<ArrayReference>(&ref))
                symbols[symbol].add(IndirectReference(*array_reference));
            else if (auto tuple_reference = std::get_if<TupleReference>(&ref)) {
                auto key = SymbolReference{ *this, symbol };
                auto object = get_object(key);
                for (auto d : *tuple_reference)
                    object->array.add(d.to_data(*this, key));
                return symbols[symbol].add(IndirectReference(create_symbol_reference(Data(object))));
            }
        }
    }

    M<IndirectReference> Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end()) {
            return symbols.emplace(symbol, SymbolReference{ *this, symbol }).first->second;
        } else {
            return it->second;
        }
    }

    GlobalContext::~GlobalContext() {
        for (auto & object : objects) {
            try {
                M<std::list<Function>> functions;
                for (auto const& d : object.second["destructor"].to_data(*this))
                    functions.add(d.get<Object*>()->functions);

                if (!functions.empty())
                    call_function(get_global(), get_global().expression, functions, std::make_shared<Parser::Tuple>());
            } catch (Data::BadAccess & e) {}
        }
    }


    M<Reference> Arguments::compute(Context & context) const {
        if (reference)
            return reference.value();
        else
            return execute(context, expression);
    }

    using ParserExpression = std::shared_ptr<Parser::Expression>;

    void set_arguments(Context & context, FunctionContext & function_context, std::shared_ptr<Parser::Expression> parameters, Arguments const& arguments) {
        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            auto reference = arguments.compute(context);

            if (function_context.has_symbol(symbol->name)) {
                if (reference.to_data(context, symbol) != function_context[symbol->name].to_data(context, arguments.expression))
                    throw FunctionArgumentsError();
            } else {
                function_context.add_symbol(symbol->name, reference);
            }
        } else if (auto p_tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                if (auto a_tuple = std::dynamic_pointer_cast<Parser::Tuple>(*expression)) {
                    if (p_tuple->objects.size() == a_tuple->objects.size()) {
                        for (size_t i = 0; i < p_tuple->objects.size(); i++)
                            set_arguments(context, function_context, p_tuple->objects[i], a_tuple->objects[i]);
                    } else throw FunctionArgumentsError();
                } else {
                    set_arguments(context, function_context, parameters, arguments.compute(context));
                }
            } else if (auto reference = std::get_if<M<Reference>>(&arguments)) {
                bool success = false;
                for (auto ref : *reference) {
                    try {
                        if (auto tuple_reference = std::get_if<TupleReference>(&ref)) {
                            if (tuple_reference->size() == p_tuple->objects.size()) {
                                for (size_t i = 0; i < p_tuple->objects.size(); i++)
                                    set_arguments(context, function_context, p_tuple->objects[i], (*tuple_reference)[i]);
                            } else throw FunctionArgumentsError();
                        } else {
                            try {
                                for (auto d : ref.to_data(function_context)) {
                                    auto object = d.get<Object*>();
                                    if (object->array.size() == p_tuple->objects.size()) {
                                        for (size_t i = 0; i < p_tuple->objects.size(); i++)
                                            set_arguments(context, function_context, p_tuple->objects[i], Reference(ArrayReference{*object, i}));
                                    } else throw FunctionArgumentsError();
                                }
                            } catch (Data::BadAccess const& e) {
                                throw FunctionArgumentsError();
                            }
                        }
                        success = true;
                    } catch (FunctionArgumentsError const& e) {}
                }
                if (!success)
                    throw FunctionArgumentsError();
            }
        } else if (auto p_function = std::dynamic_pointer_cast<Parser::FunctionCall>(parameters)) {
            if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(p_function->function)) {
                if (!function_context.has_symbol(symbol->name)) {
                    Object* object = context.new_object(symbol);
                    auto function_definition = context.get_global().lambdas[p_function].function_definition;
                    function_definition->parameters = p_function->arguments;

                    if (arguments.reference) {
                        function_definition->body = context.get_global().lambdas[p_function].symbol;

                        auto & function_context = context.get_global().contexts[function_definition];
                        function_context.add_symbol("#cached", arguments.reference.value());
                    } else {
                        function_definition->body = arguments.expression;

                        for (auto symbol : function_definition->body->symbols)
                           function_context.add_symbol(symbol, context[symbol]);
                    }
                    object->functions.push_front(CustomFunction{function_definition, function_context});

                    function_context.add_symbol(symbol->name, Reference(Data(object)));
                    return;
                }
            }

            M<std::list<Function>> all_functions;
            for (auto data : execute(context, p_function->function).to_data(context, p_function->function)) {
                try {
                    all_functions.add(data.get<Object*>()->functions);
                } catch (Data::BadAccess const& e) {}
            }

            M<Reference> reference = call_function(context, all_functions, arguments);
            // TODO : catch exception
            set_arguments(context, function_context, p_function->arguments, Arguments(arguments.expression, reference));
        } else if (auto p_property = std::dynamic_pointer_cast<Parser::Property>(parameters)) {
            bool success = false;
            for (auto reference : arguments.compute(context)) {
                try {
                    if (auto property_reference = std::get_if<PropertyReference>(&reference)) {
                        if (p_property->name == property_reference->name || p_property->name == ".") {
                            set_arguments(context, function_context, p_property->object, Arguments(arguments.expression, Reference(Data(property_reference->parent))));
                        } else throw FunctionArgumentsError();
                    } else throw FunctionArgumentsError();
                    success = true;
                } catch (FunctionArgumentsError const& e) {}
            }

            if (!success)
                throw FunctionArgumentsError();
        } else throw FunctionArgumentsError();
    }

    using It = std::map<std::string, M<IndirectReference>>::iterator;

    void call_argument(M<Reference> & result, CustomFunction const& function, FunctionContext function_context, It const it, It const end) {
        if (it != end) {
            bool success = false;
            for (auto const& m1 : it->second) {
                function_context.add_symbol(it->first, Reference(m1));
                It copy = it;
                try {
                    call_argument(result, function, function_context, copy++, end);
                    success = true;
                } catch (FunctionArgumentsError & e) {}
            }
            if (!success)
                throw FunctionArgumentsError();
        } else {
            if (function->filter != nullptr) {
                for (auto data : execute(function_context, function->filter).to_data(function_context, function_context.expression)) {
                    try {
                        data.get<bool>();
                    } catch (Data::BadAccess const& e) {
                        // TODO : exception
                    }
                }
            }

            result.add(execute(function_context, function->body));
        }
    }

    M<Reference> call_function(Context & context, M<std::list<Function>> const& all_functions, Arguments arguments) {
        M<Reference> m;

        for (auto const& functions : all_functions) {
            for (auto const& function : functions) {
                try {
                    if (auto custom = std::get_if<CustomFunction>(&function)) {
                        auto function_context = custom->context;

                        set_arguments(context, function_context, (*custom)->parameters, arguments);

                        M<Reference> result;
                        call_argument(result, *custom, function_context, function_context.begin(), function_context.end());
                        return result;
                    } else if (auto system = std::get_if<SystemFunction>(&function)) {
                        return (*system)(arguments);
                    }
                } catch (FunctionArgumentsError & e) {}
            }

            //TODO : exceptions
        }

        return m;
    }

    M<Reference> execute(Context & context, std::shared_ptr<Parser::Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(expression)) {
            auto function = execute(context, function_call->function);

            M<std::list<Function>> all_functions;
            for (auto data : function.to_data(context, expression)) {
                try {
                    all_functions.add(data.get<Object*>()->functions);
                } catch (Data::BadAccess const& e) {}
            }

            return call_function(context, all_functions, function_call->arguments);
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
            auto object = context.new_object(function_definition);

            context.get_global().contexts.emplace(function_definition, FunctionContext(context.get_global()));
            CustomFunction f{function_definition, context.get_global().contexts[function_definition]};
            for (auto symbol : function_definition->body->symbols)
                if (context.has_symbol(symbol))
                    f.context.add_symbol(symbol, context[symbol]);
            if (function_definition->filter != nullptr)
                for (auto symbol : function_definition->filter->symbols)
                    if (context.has_symbol(symbol))
                        f.context.add_symbol(symbol, context[symbol]);

            object->functions.push_front(Function(f));

            return Reference(Data(object));
        } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(expression)) {
            M<Reference> m;

            for (auto r : execute(context, property->object)) {
                auto data = r.to_data(context, expression);
                for (auto d : data) {
                    try {
                        auto object = d.get<Object*>();
                        m.add((*object)[property->name]);
                    } catch (Data::BadAccess const& e) {
                        m.add(Data{});
                    }
                }
            }

            return m;
        } else if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(expression)) {
            auto data = get_symbol(symbol->name);
            if (auto b = std::get_if<bool>(&data)) {
                return Reference(Data(*b));
            } else if (auto l = std::get_if<long>(&data)) {
                return Reference(Data(*l));
            } else if (auto d = std::get_if<double>(&data)) {
                return Reference(Data(*d));
            } else if (auto str = std::get_if<std::string>(&data)) {
                std::vector<M<Reference>> string;
                for (auto c : *str)
                    string.push_back(Reference(Data{c}));
                return Reference(TupleReference(string));
            } else {
                return context[symbol->name];
            }
        } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(expression)) {
            std::vector<M<Reference>> reference;

            for (auto e : tuple->objects)
                reference.push_back(execute(context, e));

            return Reference(reference);
        } else return {};
    }

    void create_structures(GlobalContext const& context) {

        for (auto const& o : context.objects) {
            o.
        }
    }

    MetaData analyze(std::shared_ptr<Parser::Expression> expression) {
        GlobalContext context(expression);

        auto analysis = execute(context, expression);

        for (auto const& symbol : context) {
            meta_data.global_variables[symbol.first] = symbol.second;
        }
        meta_data.global_variables = std::move(context.symbols);

        for (auto & object : context.objects) {
            MetaData::Structure structure;

            for (auto const& pair : object.properties) {
                auto & types = structure[pair.first];

                for (auto const& data : pair.second) {
                    if (std::get_if<Object*>(&data)) types.insert(MetaData::Pointer);
                    else if (std::get_if<bool>(&data)) types.insert(MetaData::Bool);
                    else if (std::get_if<char>(&data)) types.insert(MetaData::Char);
                    else if (std::get_if<long>(&data)) types.insert(MetaData::Int);
                    else if (std::get_if<double>(&data)) types.insert(MetaData::Float);
                }
            }

            meta_data.structures.insert(structure);
        }

        return meta_data;
    }

}
