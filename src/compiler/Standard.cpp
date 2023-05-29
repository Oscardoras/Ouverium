#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>

#include "Standard.hpp"

#include "../Utils.hpp"


namespace Analyzer {

    namespace Standard {

        M<Data> M<SymbolReference>::to_data() const {
            M<Data> m;
            for (auto & reference : *this)
                for (auto & data : reference.get())
                    m.add(data);
            return m;
        }

        M<Data> M<Reference>::to_data(Context & context) const {
            M<Data> m;
            for (auto const& e : *this) {
                auto data = e.to_data(context);
                m.add(data);
            }
            return m;
        }

        M<SymbolReference> M<Reference>::to_symbol_reference(Context & context) const {
            M<SymbolReference> m;
            for (auto const& e : *this)
                m.add(e.to_symbol_reference(context));
            return m;
        }

        std::reference_wrapper<M<Data>> Object::get_property(Context & context, std::string name) {
            auto & field = properties[name];

            if (field.empty())
                field.add(context.new_object());

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


        void Analyzer::set_references(Context & function_context, std::shared_ptr<Parser::Expression> parameters, M<Reference> const& reference) {
            if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
                auto & lvalue = function_context[symbol->name];
                auto rvalue = reference.to_symbol_reference(function_context);
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

        std::shared_ptr<AnalyzedExpression> Analyzer::set_references(Context & context, bool potential, Context & function_context, std::map<std::shared_ptr<Parser::Expression>, Analyzer::Analysis> & computed, std::shared_ptr<Parser::Expression> parameters, std::shared_ptr<Parser::Expression> arguments) {
            if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(parameters)) {
                auto it = computed.find(arguments);
                auto analyse = it != computed.end() ? it->second : (computed[arguments] = execute(context, potential, arguments));
                auto & lvalue = function_context[symbol->name];
                auto rvalue = analyse.references.to_symbol_reference(context);
                lvalue.add(rvalue);
                return analyse.expression;
            } else if (auto p_tuple = std::dynamic_pointer_cast<Parser::Tuple>(parameters)) {
                if (auto a_tuple = std::dynamic_pointer_cast<Parser::Tuple>(arguments)) {
                    if (p_tuple->objects.size() == a_tuple->objects.size()) {
                        std::vector<std::shared_ptr<AnalyzedExpression>> analyzed_expression;
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

                    return std::make_shared<FunctionRun>(meta_data.function_definitions[(std::stringstream() << p_function).str()], std::make_shared<Tuple>());
                } else throw FunctionArgumentsError();
            } else throw FunctionArgumentsError();
        }

        void Analyzer::call_reference(M<Reference> & references, bool potential, Function const& function, Context function_context, It const it, It const end) {
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

        Analyzer::Analysis Analyzer::call_function(Context & context, bool potential, std::shared_ptr<Parser::Position> position, M<std::list<Function>> const& all_functions, std::shared_ptr<Parser::Expression> arguments) {
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

                auto a = execute(context, potential, property->object);
                for (auto reference : a.references) {
                    auto data = reference.to_data(context);
                    for (auto d : data) {
                        if (auto object = std::get_if<Object*>(&d)) {
                            auto field = (*object)->get_property(context, property->name);
                            m.add(Reference(field));
                        } else
                            m.add(Reference(Data(context.new_object())));
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
                std::vector<std::shared_ptr<AnalyzedExpression>> analyzed_expression;
                for (auto e : tuple->objects) {
                    auto a = execute(context, potential, e);
                    reference.push_back(a.references);
                    analyzed_expression.push_back(a.expression);
                }
                return Analysis {
                    .references = Reference(reference),
                    .expression = std::make_shared<Tuple>(analyzed_expression)
                };
            } else return {};
        }

        std::shared_ptr<AnalyzedExpression> Analyzer::analyze(std::shared_ptr<Parser::Expression> expression) {
            MetaData meta_data;
            GlobalContext context(meta_data);

            execute(context, false, expression);

            meta_data.global_symbols = std::move(context.symbols);

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

}
