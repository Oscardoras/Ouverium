#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>

#include "Standard.hpp"

#include "../Utils.hpp"


namespace Analyzer::Standard {

    M<Data> compute(Context & context, Reference const& reference, M<Data> const& datas) {
        M<Data> m;

        for (auto const& data : datas) {
            if (data == Data{})
                if (context.gettings.find(reference) == context.gettings.end()) {
                    context.gettings.insert(reference);
                    M<std::list<Function>> functions;
                    for (auto const& d : context.get_global()["getter"].to_data(context))
                        functions.add(d.get<Object*>()->functions);
                    auto r = call_function(context, context.expression, functions, reference).to_data(context);
                    context.gettings.erase(reference);
                    m.add(r);
                } else {
                    Data d = context.new_object();

                    if (auto symbol_reference = std::get_if<SymbolReference>(&reference)) symbol_reference->get().add(d);
                    else if (auto property_reference = std::get_if<PropertyReference>(&reference)) static_cast<M<Data> &>(*property_reference).add(d);
                    else if (auto array_reference = std::get_if<ArrayReference>(&reference)) static_cast<M<Data> &>(*array_reference).add(d);

                    m.add(d);
                }
            else
                m.add(data);
        }

        return m;
    }

    M<Data> IndirectReference::to_data(Context & context) const {
        return compute(context, *this, std::visit([](auto const& arg) -> M<Data>& {
            return arg;
        }, *this));
    }

    M<Data> M<IndirectReference>::to_data(Context & context) const {
        M<Data> m;
        for (auto const& e : *this)
            m.add(e.to_data(context));
        return m;
    }

    M<Data> Reference::to_data(Context & context) const {
        auto get_data = [this, &context]() -> M<Data> {
            if (auto data = std::get_if<M<Data>>(this))
                return *data;
            else if (auto symbol_reference = std::get_if<SymbolReference>(this))
                return *symbol_reference;
            else if (auto property_reference = std::get_if<PropertyReference>(this))
                return *property_reference;
            else if (auto array_reference = std::get_if<ArrayReference>(this))
                return *array_reference;
            else if (auto tuple_reference = std::get_if<TupleReference>(this)) {
                auto object = context.new_object();
                object->array.reserve(tuple_reference->size());
                for (auto d : *tuple_reference)
                    object->array.push_back(d.to_data(context));
                return Data(object);
            } else return {};
        };

        return compute(context, *this, get_data());
    }

    IndirectReference Reference::to_indirect_reference(Context & context) const {
        if (auto data = std::get_if<M<Data>>(this))
            return context.new_reference(*data);
        else if (auto symbol_reference = std::get_if<SymbolReference>(this))
            return *symbol_reference;
        else if (auto property_reference = std::get_if<PropertyReference>(this))
            return *property_reference;
        else if (auto array_reference = std::get_if<ArrayReference>(this))
            return *array_reference;
        else if (auto tuple_reference = std::get_if<TupleReference>(this)) {
            auto object = context.new_object();
            object->array.reserve(tuple_reference->size());
            for (auto d : *tuple_reference)
                object->array.push_back(d.to_data(context));
            return context.new_reference(Data(object));
        } else return context.new_reference();
    }

    M<Data> M<Reference>::to_data(Context & context) const {
        M<Data> m;
        for (auto const& e : *this)
            m.add(e.to_data(context));
        return m;
    }

    M<IndirectReference> M<Reference>::to_indirect_reference(Context & context) const {
        M<IndirectReference> m;
        for (auto const& e : *this)
            m.add(e.to_indirect_reference(context));
        return m;
    }

    IndirectReference Object::operator[](std::string name) {
        return PropertyReference{*this, name};
    }

    Object* Context::new_object() {
        auto & objects = get_global().objects;
        objects.push_back(Object());
        return &objects.back();
    }

    Object* Context::new_object(Object && object) {
        auto & objects = get_global().objects;
        objects.push_back(std::move(object));
        return &objects.back();
    }

    M<Data> & Context::new_reference(M<Data> const& data) {
        auto & references = get_global().references;
        references.push_back(data);
        return references.back();
    }

    bool Context::has_symbol(std::string const& symbol) {
        return symbols.find(symbol) != symbols.end();
    }

    M<IndirectReference> Context::operator[](std::string const& symbol) {
        auto it = symbols.find(symbol);
        if (it == symbols.end())
            return symbols.emplace(symbol, new_reference()).first->second;
        else {
            return it->second;
        }
    }

    GlobalContext::~GlobalContext() {
        for (auto const& object : objects) {
            auto it = object.properties.find("destructor");
            if (it != object.properties.end()) {
                M<std::list<Function>> functions;
                for (auto const& d : it->second)
                    functions.add(d.get<Object*>()->functions);
                call_function(get_global(), get_global().expression, functions, std::make_shared<Parser::Tuple>());
            }
        }
    }


    M<Reference> Arguments::compute(Context & context) const {
        if (auto expression = std::get_if<const std::shared_ptr<Parser::Expression>>(this)) {
            return execute(context, *expression);
        } else if (auto reference = std::get_if<const M<Reference>>(this)) {
            return *reference;
        } else return {};
    }

    using ParserExpression = std::shared_ptr<Parser::Expression>;

    void set_arguments(Context & context, FunctionContext & function_context, std::shared_ptr<Parser::Expression> parameters, Arguments const& arguments) {
        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            auto reference = arguments.compute(context);

            if (function_context.has_symbol(symbol->name)) {
                if (reference.to_data(context) != function_context[symbol->name].to_data(context))
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
                    Object* object = context.new_object();
                    auto function_definition = std::make_shared<Parser::FunctionDefinition>();
                    function_definition->parameters = p_function->arguments;

                    if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                        function_definition->body = *expression;

                        object->functions.push_front(CustomFunction{function_definition});
                        auto & f = object->functions.back();
                        for (auto symbol : function_definition->body->symbols)
                            f.extern_symbols.emplace(symbol, context[symbol]);

                    } else if (auto reference = std::get_if<Reference>(&arguments)) {
                        function_definition->body = std::make_shared<Parser::Symbol>("#cached");

                        object->functions.push_front(CustomFunction{function_definition});
                        object->functions.back().extern_symbols.emplace("#cached", reference->to_indirect_reference(context));
                    }

                    function_context.add_symbol(symbol->name, Reference(Data(object)));
                    return;
                }
            }

            M<std::list<Function>> all_functions;
            for (auto data : execute(context, p_function->function).to_data(context)) {
                try {
                    all_functions.add(data.get<Object*>()->functions);
                } catch (Data::BadAccess const& e) {}
            }

            M<Reference> reference = call_function(context, p_function, all_functions, arguments);
            // TODO : catch exception
            set_arguments(context, function_context, p_function->arguments, reference);
        } else if (auto p_property = std::dynamic_pointer_cast<Parser::Property>(parameters)) {
            bool success = false;
            for (auto reference : arguments.compute(context)) {
                try {
                    if (auto property_reference = std::get_if<PropertyReference>(&reference)) {
                        if (p_property->name == property_reference->name || p_property->name == ".") {
                            set_arguments(context, function_context, p_property->object, Reference(Data(&property_reference->parent.get())));
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

    void call_argument(M<Reference> & result, Function const& function, FunctionContext function_context, It const it, It const end) {
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
            if (auto custom = std::get_if<CustomFunction>(&function)) {
                bool filter = false;
                if ((*custom)->filter != nullptr) {
                    for (auto data : execute(function_context, (*custom)->filter).to_data(function_context)) {
                        try {
                            if (data.get<bool>())
                                filter = true;
                        } catch (Data::BadAccess const& e) {
                            // TODO : exception
                        }
                    }
                } else filter = true;

                if (filter) {
                    result.add(execute(function_context, (*custom)->body));
                } else throw FunctionArgumentsError();
            } else if (auto system = std::get_if<SystemFunction>(&function))
                result.add((*system).pointer(function_context));
        }
    }

    M<Reference> call_function(Context & context, std::shared_ptr<Parser::Expression> expression, M<std::list<Function>> const& all_functions, Arguments arguments) {
        M<Reference> m;

        for (auto const& functions : all_functions) {
            for (auto const& function : functions) {
                try {
                    FunctionContext function_context(context, expression);
                    for (auto & symbol : function.extern_symbols)
                        function_context.add_symbol(symbol.first, symbol.second);

                    std::shared_ptr<Parser::Expression> parameters = nullptr;
                    if (auto custom = std::get_if<CustomFunction>(&function))
                        parameters = (*custom)->parameters;
                    else if (auto system = std::get_if<SystemFunction>(&function))
                        parameters = (*system).parameters;

                    set_arguments(context, function_context, parameters, arguments);

                    M<Reference> result;
                    call_argument(result, function, function_context, function_context.begin(), function_context.end());
                    return result;
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
            for (auto data : function.to_data(context)) {
                try {
                    all_functions.add(data.get<Object*>()->functions);
                } catch (Data::BadAccess const& e) {}
            }

            return call_function(context, function_call, all_functions, function_call->arguments);
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
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
        } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(expression)) {
            M<Reference> m;

            for (auto r : execute(context, property->object)) {
                auto data = r.to_data(context);
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
                return Reference(Data(context.new_object(*str)));
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

    std::pair<std::shared_ptr<Expression>, MetaData> Analyzer::analyze(std::shared_ptr<Parser::Expression> expression) {
        GlobalContext context;

        auto analysis = execute(context, false, expression);

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
