#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <variant>

#include <ouverium/compiler/c/Translator.hpp>
#include <ouverium/compiler/Analyzer.hpp>
#include <ouverium/compiler/SimpleAnalyzer.hpp>

#include <ouverium/parser/Expressions.hpp>
#include <ouverium/parser/Standard.hpp>
#include <ouverium/parser/Types.hpp>


namespace Analyzer {

    std::filesystem::path SimpleAnalyzer::import_file(std::shared_ptr<Parser::Expression> caller, std::string const& p) {
        if (auto path = Parser::Standard::get_canonical_path(caller->position, p)) {
            auto root = caller->get_root();

            auto it = sources.find(path.value());
            if (it == sources.end()) {
                if (auto expression = Parser::Standard(path.value()).get_tree()) {
                    sources[path.value()] = expression;

                    {
                        auto symbols = Translator::CStandard::Translator::symbols;
                        expression->compute_symbols(symbols);
                    }

                    {
                        auto symbols = expression->get_root()->symbols;
                        symbols.insert(expression->symbols.begin(), expression->symbols.end());
                        root->compute_symbols(symbols);
                    }

                    iterate(expression);
                } else throw Exception({ caller->position });
            } else {
                auto expression = it->second;

                auto symbols = root->symbols;
                symbols.insert(expression->symbols.begin(), expression->symbols.end());
                root->compute_symbols(symbols);
            }

            return path.value();
        } else throw Exception({ caller->position });
    }

    void SimpleAnalyzer::iterate(std::shared_ptr<Parser::Expression> expression) {
        if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(expression)) {
            iterate(function_call->function);
            iterate(function_call->arguments);

            if (auto function_symbol = std::dynamic_pointer_cast<Parser::Symbol>(function_call->function)) {
                if (function_symbol->name == "import") {
                    calls[function_call].insert("import");

                    if (auto arg_symbol = std::dynamic_pointer_cast<Parser::Symbol>(function_call->arguments)) {
                        auto value = get_symbol(arg_symbol->name);
                        if (auto* str = std::get_if<std::string>(&value)) {
                            auto path = import_file(arg_symbol, *str);
                            imports[function_call] = path;
                        }
                    }
                } else if (function_symbol->name == ";" && std::dynamic_pointer_cast<Parser::Tuple>(function_call->arguments)) {
                    calls[function_call].insert(";");
                } else if (function_symbol->name == "if") {
                    calls[function_call].insert("if");
                } else {
                    trivial_calls.calls[function_call] = function_symbol->name;
                }
            }
        } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
            iterate(function_definition->parameters);
            iterate(function_definition->body);
            if (function_definition->filter)
                iterate(function_definition->filter);

            if (auto call = std::dynamic_pointer_cast<Parser::FunctionCall>(function_definition->parent.lock())) {
                if (auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(call->function)) {
                    trivial_calls.functions[symbol->name].insert(function_definition);
                }
            }
        } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(expression)) {
            iterate(*std::get_if<std::shared_ptr<Parser::Expression>>(&property->object));
            properties.insert(property->name);
        } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(expression)) {
            for (auto const& e : tuple->objects)
                iterate(e);
        }
    }

    MetaData SimpleAnalyzer::analize(std::shared_ptr<Parser::Expression> expression) {
        auto structure = std::make_shared<Structure>();
        structure->name = "Object";
        structure->function = true;
        structure->array.insert(structure);
        structure->array.insert(Bool);
        structure->array.insert(Char);
        structure->array.insert(Int);
        structure->array.insert(Float);

        std::set<std::string> properties;
        iterate(expression);

        for (auto const& property : properties) {
            auto& p = structure->properties[property];
            p.insert(structure);
            p.insert(Bool);
            p.insert(Char);
            p.insert(Int);
            p.insert(Float);
        }

        MetaData meta_data;

        // meta_data.structures.insert(structure);

        meta_data.calls = std::move(calls);
        for (auto const& [function_call, name] : trivial_calls.calls) {
            auto const& functions = trivial_calls.functions[name];
            meta_data.calls[function_call].insert(functions.begin(), functions.end());
        }

        for (auto const& [function_call, path] : imports)
            meta_data.sources[function_call] = sources[path];

        return meta_data;
    }

}
