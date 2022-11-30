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

    std::vector<std::shared_ptr<CStructures::Instruction>> eval_system_function(Reference (*function)(FunctionContext&), std::shared_ptr<Expression> arguments, Analyzer::MetaData & meta) {
        std::vector<std::shared_ptr<CStructures::Instruction>> r;

        switch ((long) function) {
        case (long) (Reference (*)(FunctionContext & context)) Base::separator:
            if (arguments->type == Expression::Tuple) {
                auto tuple = std::static_pointer_cast<Tuple>(arguments);

                for (auto const& o : tuple->objects) {
                    auto v = get_instructions(o, meta);
                    r.insert(r.end(), v.begin(), v.end());
                }
            } else {
                auto v = get_instructions(arguments, meta);
                r.insert(r.end(), v.begin(), v.end());
            }
            break;

        case (long) (Reference (*)(FunctionContext & context)) Base::if_statement:
            if (arguments->type == Expression::Tuple) {
                auto tuple = std::static_pointer_cast<Tuple>(arguments);

                if (tuple->objects.size() >= 2) {
                    int i = 0;
                    auto structure = std::make_shared<CStructures::If>();
                    r.push_back(structure);
                    structure->condition = get_expression(tuple->objects[i++], meta);
                    structure->body = get_instructions(tuple->objects[i++], meta);

                    while (i+2 < tuple->objects.size()) {
                        i++;
                        auto tmp = std::make_shared<CStructures::If>();
                        tmp->condition = get_expression(tuple->objects[i++], meta);
                        tmp->body = get_instructions(tuple->objects[i++], meta);
                        structure->alternative.push_back(tmp);
                        structure = tmp;
                    };

                    if (i+1 < tuple->objects.size()) {
                        i++;
                        structure->alternative = get_instructions(tuple->objects[i++], meta);
                    }
                }

            }
            break;

        case (long) (Reference (*)(FunctionContext & context)) Base::while_statement:
            if (arguments->type == Expression::Tuple) {
                auto tuple = std::static_pointer_cast<Tuple>(arguments);

                if (tuple->objects.size() == 2) {
                    auto condition = tuple->objects[0];
                    auto body = tuple->objects[1];

                    r.push_back(std::make_shared<CStructures::While>(CStructures::While {
                        .condition = get_expression(condition, meta),
                        .body = get_instructions(body, meta)
                    }));
                }

            }
            break;

        case (long) (Reference (*)(FunctionContext & context)) Base::while_statement:
            if (arguments->type == Expression::Tuple) {
                auto tuple = std::static_pointer_cast<Tuple>(arguments);

                if (tuple->objects.size() == 2) {
                    auto condition = tuple->objects[0];
                    auto body = tuple->objects[1];

                    r.push_back(std::make_shared<CStructures::While>(CStructures::While {
                        .condition = get_expression(condition, meta),
                        .body = get_instructions(body, meta)
                    }));
                }

            }
            break;

        case (long) (Reference (*)(FunctionContext & context)) Base::while_statement:
            if (arguments->type == Expression::Tuple) {
                auto tuple = std::static_pointer_cast<Tuple>(arguments);

                if (tuple->objects.size() == 2) {
                    auto condition = tuple->objects[0];
                    auto body = tuple->objects[1];

                    r.push_back(std::make_shared<CStructures::While>(CStructures::While {
                        .condition = get_expression(condition, meta),
                        .body = get_instructions(body, meta)
                    }));
                }

            }
            break;

        default:
            break;
        }

        return r;
    }

    std::vector<std::shared_ptr<CStructures::Instruction>> get_instructions(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta) {
        if (expression->type == Expression::FunctionCall) {
            auto function_call = std::static_pointer_cast<FunctionCall>(expression);

            auto & functions = meta.links[function_call];
            if (functions.size() == 1) {

            }
        }
    }

    std::shared_ptr<CStructures::Expression> get_expression(std::shared_ptr<Expression> expression, Analyzer::MetaData & meta) {
        if (expression->type == Expression::FunctionCall) {
            auto function_call = std::static_pointer_cast<FunctionCall>(expression);

            auto & link = meta.links[function_call];
            if (link.size() == 1) {
                try {
                    auto f = std::get<std::shared_ptr<FunctionDefinition>>(link[0]);

                    auto r = std::make_shared<CStructures::FunctionCall>(CStructures::FunctionCall {
                        .function = get_expression(function_call->function, meta)
                    });

                    if (f->parameters->type == Expression::Tuple) {
                        if (function_call->object->type == Expression::Tuple) {
                            for (auto const& o : std::static_pointer_cast<Tuple>(function_call->object)->objects)
                                r->parameters.push_back(get_expression(o, meta));
                        }
                    } else {
                        r->parameters.push_back(get_expression(function_call->object, meta));
                    }

                    return r;
                } catch (std::bad_variant_access const& e) {

                }
            } else {
                auto r = std::make_shared<CStructures::FunctionCall>(CStructures::FunctionCall {
                    .function = std::make_shared<CStructures::VariableCall>(CStructures::VariableCall { .name = "GC_eval_function" })
                });

                r->parameters.push_back(get_expression(function_call->function, meta));
                r->parameters.push_back(get_expression(function_call->object, meta));

                return r;
            }
        } else if (expression->type == Expression::Property) {
            auto property = std::static_pointer_cast<Property>(expression);

            auto o = get_expression(property->object, meta);
            return std::make_shared<CStructures::Property>(CStructures::Property {
                .object = o,
                .name = property->name,
                .pointer = meta.types[property]->pointer
            });
        } else if (expression->type == Expression::Symbol) {
            auto symbol = std::static_pointer_cast<Symbol>(expression);

            return std::make_shared<CStructures::VariableCall>(CStructures::VariableCall {.name = symbol->name});
        } else if (expression->type == Expression::Tuple) {
            auto tuple = std::static_pointer_cast<Tuple>(expression);

            auto list = std::make_shared<CStructures::List>(CStructures::List {});
            for (auto const& o : tuple->objects)
                list->objects.push_back(get_expression(o, meta));
        }
    }

}
