#include <cstddef>
#include <exception>
#include <map>
#include <memory>
#include <string>

#include <ouverium/compiler/c/Code.hpp>
#include <ouverium/compiler/c/Translator.hpp>
#include <ouverium/compiler/Analyzer.hpp>

#include <ouverium/parser/Expressions.hpp>
#include <ouverium/parser/Standard.hpp>
#include <ouverium/parser/Types.hpp>


namespace Translator::CStandard {

    std::shared_ptr<Reference> Translator::eval_system_function(Analyzer::SystemFunction const& function, std::shared_ptr<Parser::Expression> arguments, Instructions& instructions, Instructions::iterator it) {
        if (function == "import") {
            auto symbol = std::dynamic_pointer_cast<Parser::Symbol>(arguments);
            auto canonical_path = *Parser::Standard::get_canonical_path(symbol->position, std::get<std::string>(get_symbol(symbol->name)));
            std::string name = Name(canonical_path.string()).get();

            if (!code.imports.contains(name)) {
                auto expression = meta_data.sources[std::static_pointer_cast<Parser::FunctionCall>(arguments->parent.lock())];
                auto& main = code.imports[name];

                auto& import_global_variables = code.global_variables;
                for (auto const& symbol : expression->symbols)
                    import_global_variables[symbol] = Unknown;
                Instructions& import_instructions = main.body;
                auto r = get_expression(expression, import_instructions, import_instructions.end());
                if (r->owned) {
                    import_instructions.push_back(std::make_shared<FunctionCall>(FunctionCall(
                        std::make_shared<Symbol>("Ov_Reference_free"),
                        {
                            r
                        }
                    )));
                }

                instructions.push_back(std::make_shared<FunctionCall>(FunctionCall(
                    std::make_shared<Symbol>("import_" + name + "_body"),
                    {}
                )));
            }
        } else if (function == ";") {
            auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(arguments);

            get_expression(tuple->objects[0], instructions, it);
            std::shared_ptr<Reference> last = get_expression(tuple->objects[1], instructions, it);

            if (!last) {
                last = std::make_shared<Reference>(true);
                instructions.insert(it, std::make_shared<Affectation>(
                    last,
                    std::make_shared<FunctionCall>(FunctionCall(
                        std::make_shared<Symbol>("Ov_Reference_new_uninitialized"),
                        {}
                    ))
                ));
            }

            return last;
        } else if (function == "if") {
            auto r = std::make_shared<Reference>(true);

            instructions.push_back(std::make_shared<Affectation>(
                r,
                std::make_shared<Value>(nullptr)
            ));

            auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(arguments);
            if (tuple->objects.size() >= 2) {
                size_t i = 0;

                auto condition = get_expression(tuple->objects[i++], instructions, it);

                auto if_structure = std::make_shared<If>(
                    std::make_shared<Property>(
                        std::make_shared<Property>(
                            std::make_shared<FunctionCall>(FunctionCall(
                                std::make_shared<Symbol>("Ov_Reference_get"),
                                {
                                    std::make_shared<FunctionCall>(FunctionCall(
                                        std::make_shared<Symbol>("Ov_Reference_share"),
                                        {
                                            condition
                                        }
                                    ))
                                }
                            )),
                            "data",
                            false
                        ),
                        "b",
                        false
                    )
                );

                // TODO: copy reference
                if_structure->body.push_back(std::make_shared<Affectation>(
                    r,
                    get_expression(tuple->objects[i++], if_structure->body, if_structure->body.end()),
                    false
                ));

                auto old_if_structure = if_structure;
                while (i + 2 < tuple->objects.size()) {
                    i++;

                    auto condition = get_expression(tuple->objects[i++], old_if_structure->alternative, old_if_structure->alternative.end());

                    auto new_if_structure = std::make_shared<If>(
                        std::make_shared<Property>(
                            std::make_shared<Property>(
                                std::make_shared<FunctionCall>(FunctionCall(
                                    std::make_shared<Symbol>("Ov_Reference_get"),
                                    {
                                        std::make_shared<FunctionCall>(FunctionCall(
                                            std::make_shared<Symbol>("Ov_Reference_share"),
                                            {
                                                condition
                                            }
                                        ))
                                    }
                                )),
                                "data",
                                false
                            ),
                            "b",
                            false
                        )
                    );

                    new_if_structure->body.push_back(std::make_shared<Affectation>(
                        r,
                        get_expression(tuple->objects[i++], new_if_structure->body, new_if_structure->body.end()),
                        false
                    ));

                    old_if_structure->alternative.push_back(new_if_structure);

                    if (condition->owned) {
                        old_if_structure->alternative.push_back(std::make_shared<FunctionCall>(FunctionCall(
                            std::make_shared<Symbol>("Ov_Reference_free"),
                            {
                                condition
                            }
                        )));
                    }

                    old_if_structure = new_if_structure;
                };

                if (i + 1 < tuple->objects.size()) {
                    i++;
                    old_if_structure->alternative.push_back(std::make_shared<Affectation>(
                        r,
                        get_expression(tuple->objects[i++], old_if_structure->alternative, old_if_structure->alternative.end()),
                        false
                    ));
                } else {
                    old_if_structure->alternative.push_back(std::make_shared<Affectation>(
                        r,
                        std::make_shared<FunctionCall>(FunctionCall(
                            std::make_shared<Symbol>("Ov_Reference_new_uninitialized"),
                            {}
                        )),
                        false
                    ));
                }

                instructions.push_back(if_structure);

                if (condition->owned) {
                    instructions.insert(it, std::make_shared<FunctionCall>(FunctionCall(
                        std::make_shared<Symbol>("Ov_Reference_free"),
                        {
                            condition
                        }
                    )));
                }
            } else throw std::exception();

            return r;
        }
        //  else if (function.pointer == Analyzer::Functions::copy) {
        //     return std::make_shared<Structures::FunctionCall>(Structures::FunctionCall{
        //         .function = std::make_shared<Structures::VariableCall>(Structures::VariableCall {.name = ""}),
        //         .parameters = std::vector<std::shared_ptr<Structures::Expression>> { get_expression(arguments, meta, instructions, references) }
        //         });
        // } else if (function.pointer == Analyzer::Functions::copy_pointer) {
        //     return std::make_shared<Structures::FunctionCall>(Structures::FunctionCall{
        //         .function = std::make_shared<Structures::VariableCall>(Structures::VariableCall {.name = ""}),
        //         .parameters = std::vector<std::shared_ptr<Structures::Expression>> { get_expression(arguments, meta, instructions, references) }
        //         });
        // } else if (function.pointer == Analyzer::Functions::assign) {
        //     if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(arguments)) {
        //         if (tuple->objects.size() == 2) {
        //             auto var = get_expression(tuple->objects[0], meta, instructions, references);
        //             auto object = get_expression(tuple->objects[1], meta, instructions, references);

        //             if (auto lvalue = std::dynamic_pointer_cast<Structures::LValue>(var)) {
        //                 return std::make_shared<Structures::Affectation>(Structures::Affectation{
        //                     .lvalue = lvalue,
        //                     .value = object
        //                     });
        //             } else if (auto var_tuple = std::dynamic_pointer_cast<Parser::Tuple>(tuple->objects[0])) {
        //                 if (auto object_tuple = std::dynamic_pointer_cast<Parser::Tuple>(tuple->objects[1])) {
        //                     if (var_tuple->objects.size() == object_tuple->objects.size()) {
        //                         for (unsigned long i = 0; i < var_tuple->objects.size(); i++) {
        //                             instructions.push_back(std::make_shared<Structures::Affectation>(Structures::Affectation{
        //                                 .lvalue = std::make_shared<Structures::VariableCall>(Structures::VariableCall {
        //                                     .name = "tmp_" +
        //                                 }),
        //                                 .value = get_expression(object_tuple->objects[i], meta, instructions, references)
        //                                 }));
        //                         }
        //                         for (unsigned long i = 0; i < var_tuple->objects.size(); i++) {
        //                             instructions.push_back(std::make_shared<Structures::Affectation>(Structures::Affectation{
        //                                 .lvalue = get_expression(var_tuple->objects[i], meta, instructions, references),
        //                                 .value = std::make_shared<Structures::VariableCall>(Structures::VariableCall {
        //                                     .name = "tmp_" +
        //                                 })
        //                                 }));
        //                         }
        //                     }
        //                 }
        //             }

        //             return var;
        //         }
        //     }
        // }

        return nullptr;
    }

}
