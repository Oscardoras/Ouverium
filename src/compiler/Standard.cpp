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
                    auto r = call_function(context, context.expression, functions, reference).references.to_data(context);
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


    void Analyzer::set_references(Context & function_context, std::shared_ptr<Parser::Expression> parameters, M<Reference> const& reference) {
        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            auto & lvalue = function_context[symbol->name];
            auto rvalue = reference.to_indirect_reference(function_context);
            lvalue.add(rvalue);
        } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
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

    using ParserExpression = std::shared_ptr<Parser::Expression>;

    std::shared_ptr<Expression> set_arguments(Context & context, FunctionContext & function_context, std::shared_ptr<Parser::Expression> parameters, Arguments const& arguments) {
        if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
            if (auto expression = std::get_if<ParserExpression>(&arguments)) {
                auto a = execute(context, *expression);

                if (function_context.has_symbol(symbol->name)) {
                    if (reference.to_data(context) != function_context[symbol->name].to_data(context))
                        throw Interpreter::FunctionArgumentsError();
                } else {
                    function_context.add_symbol(symbol->name, reference);
                }

                function_context[symbol->name].add(a.references);
                return a.expression;
            } else if (auto reference = std::get_if<M<Reference>>(&arguments)) {
                return *reference;
            }
            auto it = computed.find(arguments);
            auto analyse = it != computed.end() ? it->second : (computed[arguments] = execute(context, potential, arguments));
            auto & lvalue = function_context[symbol->name];
            auto rvalue = analyse.references.to_symbol_reference(context);
            lvalue.add(rvalue);
            return analyse.expression;
        } else if (auto p_tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
            if (auto a_tuple = std::dynamic_pointer_cast<Parser::Tuple>(arguments)) {
                if (p_tuple->objects.size() == a_tuple->objects.size()) {
                    std::vector<std::shared_ptr<Expression>> analyzed_expression;
                    for (size_t i = 0; i < p_tuple->objects.size(); i++)
                        analyzed_expression.push_back(set_references(context, potential, function_context, computed, p_tuple->objects[i], a_tuple->objects[i]));
                    return std::make_shared<Tuple>(analyzed_expression);
                } else throw FunctionArgumentsError();
            } else {
                auto it = computed.find(arguments);
                auto a = it != computed.end() ? it->second : (computed[arguments] = execute(context, potential, arguments));
                set_references(function_context, parameters, a.references);
                return a.expression;
            }
        } else if (auto p_function = std::dynamic_pointer_cast<Parser::FunctionCall>(parameters)) {
            if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(p_function->function)) {
                auto function_definition = std::make_shared<Parser::FunctionDefinition>();
                function_definition->parameters = p_function->arguments;
                function_definition->body = arguments;

                auto object = context.new_object();
                Function f = {function_definition};
                for (auto symbol : function_definition->body->symbols)
                    f.extern_symbols[symbol] = context[symbol];
                object->functions.push_front(f);

                function_context[symbol->name] = SymbolReference(context.new_reference(Data(object)));

                return std::make_shared<FunctionRun>(meta_data.function_definitions[(std::ostringstream() << p_function).str()], std::make_shared<Tuple>());
            } else throw FunctionArgumentsError();
        } else throw FunctionArgumentsError();
    }

    using It = std::map<std::string, M<IndirectReference>>::iterator;

    void call_argument(M<Reference> & references, bool potential, Function const& function, FunctionContext function_context, It const it, It const end) {
        if (it != end) {
            bool success = false;
            for (auto const& m1 : it->second) {
                function_context[it->first] = SymbolReference(m1.get());
                It copy = it;
                try {
                    call_reference(references, potential, function, function_context, copy++, end);
                    success = true;
                } catch (FunctionArgumentsError & e) {}
            }
            if (!success)
                throw FunctionArgumentsError();
        } else {
            M<Reference> r;

            if (auto custom = std::get_if<std::shared_ptr<Parser::FunctionDefinition>>(&function.ptr)) {
                bool filter = false;
                if ((*custom)->filter != nullptr) {
                    auto a = execute(function_context, potential, (*custom)->filter);
                    for (auto reference : a.references)
                        for (auto data : reference.to_data(function_context)) {
                            if (auto c = std::get_if<bool>(&data)) {
                                if (*c) filter = true;
                            } else {
                                if ((*custom)->filter->position != nullptr)
                                    (*custom)->filter->position->notify_error("The expression must be a boolean");
                            }
                        }
                } else filter = true;

                if (filter) {
                    auto a = execute(function_context, potential, (*custom)->body);
                    r.add(a.references);
                } else throw FunctionArgumentsError();
            } else if (auto system = std::get_if<SystemFunction>(&function.ptr))
                r = (*system).pointer(function_context, potential);

            references.add(r);
        }
    }

    Analyzer::Analysis Analyzer::call_function(Context & context, bool potential, std::shared_ptr<Parser::Expression> expression, M<std::list<Function>> const& functions, Arguments arguments) {
        Analysis analysis;

        std::map<std::shared_ptr<Parser::Expression>, Analysis> computed;
        for (auto const& functions : all_functions) {
            for (auto const& function : functions) {
                try {
                    Context function_context(context);
                    for (auto & symbol : function.extern_symbols)
                        function_context[symbol.first] = symbol.second;

                    std::shared_ptr<Parser::Expression> parameters = nullptr;
                    if (auto custom = std::get_if<std::shared_ptr<Parser::FunctionDefinition>>(&function.ptr))
                        parameters = (*custom)->parameters;
                    else if (auto system = std::get_if<SystemFunction>(&function.ptr))
                        parameters = (*system).parameters;

                    set_references(context, potential, function_context, computed, parameters, arguments);

                    call_reference(analysis, potential, function, function_context, function_context.begin(), function_context.end());
                } catch (FunctionArgumentsError & e) {}
            }

            if (position != nullptr)
                position->notify_error("The arguments given to the function don't match");
        }

        return analysis;
    }

    Analyzer::Analysis Analyzer::execute(Context & context, bool potential, std::shared_ptr<Parser::Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(expression)) {
            M<Reference> m;
            std::vector<FunctionPointer> called_functions;

            auto f = execute(context, potential, function_call->function);

            for (auto reference : f.references) {
                for (auto d : reference.to_data(context)) {
                    std::list<Function> functions;
                    if (auto object = std::get_if<Object*>(&d))
                        functions = (*object)->functions;
                    auto result = call_function(context, potential, function_call->position, functions, function_call->arguments);
                    m.add(result.first);
                    called_functions.push_back(result.second);
                }
            }

            if (called_functions.size() == 1) {
                return Analysis {
                    .references = m,
                    .expression = std::make_shared<FunctionRun>(, )
                };
            }

            return Analysis {
                .references = m,
                .expression = std::make_shared<Property>(a.expression)
            };
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

            auto a = execute(context, property->object);
            for (auto r : a.references) {
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

            return Analysis {
                .references = m,
                .expression = std::make_shared<Property>(a.expression)
            };
        } else if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(expression)) {
            auto data = get_symbol(symbol->name);
            if (auto b = std::get_if<bool>(&data)) {
                return Analysis {
                    .references = Reference(Data(*b)),
                    .expression = std::make_shared<Value>(*b)
                };
            } else if (auto l = std::get_if<long>(&data)) {
                return Analysis {
                    .references = Reference(Data(*l)),
                    .expression = std::make_shared<Value>(*l)
                };
            } else if (auto d = std::get_if<double>(&data)) {
                return Analysis {
                    .references = Reference(Data(*d)),
                    .expression = std::make_shared<Value>(*d)
                };
            } else if (auto str = std::get_if<std::string>(&data)) {
                return Analysis {
                    .references = Reference(Data(context.new_object(*str))),
                    .expression = std::make_shared<Value>(*str)
                };
            } else {
                return Analysis {
                    .references = context[symbol->name],
                    .expression = std::make_shared<Symbol>(symbol->name)
                };
            }
        } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(expression)) {
            std::vector<M<Reference>> reference;
            std::vector<std::shared_ptr<Expression>> analyzed_expression;

            for (auto e : tuple->objects) {
                auto a = execute(context, e);
                reference.push_back(a.references);
                analyzed_expression.push_back(a.expression);
            }

            return Analysis {
                .references = Reference(reference),
                .expression = std::make_shared<Tuple>(analyzed_expression)
            };
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
