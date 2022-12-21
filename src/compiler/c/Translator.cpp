#include <algorithm>
#include <map>
#include <stdexcept>

#include "Translator.hpp"

#include "../../interpreter/system_functions/Array.hpp"
#include "../../interpreter/system_functions/ArrayList.hpp"
#include "../../interpreter/system_functions/Base.hpp"
#include "../../interpreter/system_functions/Math.hpp"
#include "../../interpreter/system_functions/Streams.hpp"
#include "../../interpreter/system_functions/String.hpp"
#include "../../interpreter/system_functions/Types.hpp"


namespace CTranslator {

    std::shared_ptr<Structures::Expression> eval_system_function(Interpreter::Reference (*function)(Interpreter::FunctionContext&), std::shared_ptr<Expression> arguments, Analyzer::MetaData & meta, Instructions & instructions) {
        switch ((unsigned long) function) {
        case (unsigned long) Interpreter::Base::separator:
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(arguments)) {
                for (auto const& o : tuple->objects) {
                    get_instructions(o, meta, instructions);
                }
            } else {
                get_instructions(arguments, meta, instructions);
            }
            break;

        case (unsigned long) Interpreter::Base::if_statement:
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(arguments)) {
                if (tuple->objects.size() >= 2) {
                    int i = 0;
                    auto structure = std::make_shared<Structures::If>();
                    structure->condition = get_expression(tuple->objects[i++], meta, instructions);
                    get_instructions(tuple->objects[i++], meta, structure->body);

                    while (i+2 < tuple->objects.size()) {
                        i++;
                        auto tmp = std::make_shared<Structures::If>();
                        tmp->condition = get_expression(tuple->objects[i++], meta, structure->alternative);
                        get_instructions(tuple->objects[i++], meta, tmp->body);
                        structure->alternative.push_back(tmp);
                        structure = tmp;
                    };

                    if (i+1 < tuple->objects.size()) {
                        i++;
                        get_instructions(tuple->objects[i++], meta, structure->alternative);
                    }

                    instructions.push_back(structure);
                }
            }
            break;

        case (unsigned long) Interpreter::Base::while_statement:
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(arguments)) {
                if (tuple->objects.size() == 2) {
                    auto condition = tuple->objects[0];
                    auto body = tuple->objects[1];

                    auto structure = std::make_shared<Structures::While>(Structures::While {
                        .condition = get_expression(condition, meta, instructions),
                    });
                    get_instructions(body, meta, structure->body);
                    instructions.push_back(structure);
                }
            }
            break;

        case (unsigned long) Interpreter::Base::copy_pointer:

            break;

        case (unsigned long) Interpreter::Base::assign:
            if (auto tuple = std::dynamic_pointer_cast<Tuple>(arguments)) {
                if (tuple->objects.size() == 2) {
                    auto var = get_expression(tuple->objects[0], meta, instructions);
                    auto object = get_expression(tuple->objects[1], meta, instructions);

                    if (auto r_value = std::dynamic_pointer_cast<Structures::RValue>(var)) {
                        return std::make_shared<Structures::Affectation>(Structures::Affectation {
                            .r_value = r_value,
                            .value = object
                        });
                    } else if (auto var_tuple = std::dynamic_pointer_cast<Tuple>(tuple->objects[0])) {
                        if (auto object_tuple = std::dynamic_pointer_cast<Tuple>(tuple->objects[1])) {
                            if (var_tuple->objects.size() == object_tuple->objects.size()) {
                                for (unsigned long i = 0; i < var_tuple->objects.size(); i++) {
                                    instructions.push_back(std::make_shared<Structures::Affectation>(Structures::Affectation {
                                        .r_value = std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                                            .name = "tmp_" +
                                        }),
                                        .value = get_expression(object_tuple->objects[i], meta, instructions)
                                    }));
                                }
                                for (unsigned long i = 0; i < var_tuple->objects.size(); i++) {
                                    instructions.push_back(std::make_shared<Structures::Affectation>(Structures::Affectation {
                                        .r_value = get_expression(var_tuple->objects[i], meta, instructions),
                                        .value = std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                                            .name = "tmp_" +
                                        })
                                    }));
                                }
                            }
                        }
                    }

                    return var;
                }
            }
            break;

        default:
            break;
        }
    }

    void get_instructions(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta, Instructions & instructions) {
        if (auto exp = std::dynamic_pointer_cast<Structures::Instruction>(get_expression(expression, meta, instructions)))
            instructions.push_back(exp);
    }

    std::shared_ptr<Structures::Expression> get_expression(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta, Instructions & instructions) {
        if (auto function_call = std::dynamic_pointer_cast<FunctionCall>(expression)) {
            auto & link = meta.links[function_call];
            if (link.size() == 1) {
                try {
                    auto f = std::get<std::shared_ptr<FunctionDefinition>>(link[0]);

                    auto r = std::make_shared<Structures::FunctionCall>(Structures::FunctionCall {
                        .function = get_expression(function_call->function, meta, instructions)
                    });

                    if (std::dynamic_pointer_cast<Tuple>(f->parameters)) {
                        if (auto args = std::dynamic_pointer_cast<Tuple>(function_call->arguments)) {
                            for (auto const& o : args->objects)
                                r->parameters.push_back(get_expression(o, meta, instructions));
                        }
                    } else {
                        r->parameters.push_back(get_expression(function_call->arguments, meta, instructions));
                    }

                    return r;
                } catch (std::bad_variant_access const& e) {
                    return eval_system_function(std::get<Interpreter::Reference (*)(Interpreter::FunctionContext&)>(link[0]), function_call->arguments, meta, instructions);
                }
            } else {
                auto r = std::make_shared<Structures::FunctionCall>(Structures::FunctionCall {
                    .function = std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                        .name = "GC_eval_function"
                    })
                });

                r->parameters.push_back(get_expression(function_call->function, meta, instructions));
                r->parameters.push_back(get_expression(function_call->arguments, meta, instructions));

                return r;
            }
        } else if (auto property = std::dynamic_pointer_cast<Property>(expression)) {
            auto o = get_expression(property->object, meta, instructions);
            return std::make_shared<Structures::Property>(Structures::Property {
                .object = o,
                .name = property->name,
                .pointer = meta.types[property]->pointer
            });
        } else if (auto symbol = std::dynamic_pointer_cast<Symbol>(expression)) {
            return std::make_shared<Structures::VariableCall>(Structures::VariableCall {
                .name = symbol->name
            });
        } else if (auto tuple = std::dynamic_pointer_cast<Tuple>(expression)) {
            auto list = std::make_shared<Structures::List>(Structures::List {});
            for (auto const& o : tuple->objects)
                list->objects.push_back(get_expression(o, meta, instructions));
        }
    }

}
