#pragma once

#include "Analyzer.hpp"


namespace Analyzer {

    namespace {

        inline void iterate(std::shared_ptr<Parser::Expression> expression, std::set<std::string> & properties) {
            if (auto function_call = std::dynamic_pointer_cast<Parser::FunctionCall>(expression)) {
                iterate(function_call->function, properties);
                iterate(function_call->arguments, properties);
            } else if (auto function_definition = std::dynamic_pointer_cast<Parser::FunctionDefinition>(expression)) {
                iterate(function_definition->parameters, properties);
                iterate(function_definition->body, properties);
                if (function_definition->filter)
                    iterate(function_definition->filter, properties);
            } else if (auto property = std::dynamic_pointer_cast<Parser::Property>(expression)) {
                iterate(property->object, properties);
                properties.insert(property->name);
            } else if (auto tuple = std::dynamic_pointer_cast<Parser::Tuple>(expression)) {
                for (auto const& e : tuple->objects)
                    iterate(e, properties);
            }
        }

    }

    inline MetaData simple_analize(std::shared_ptr<Parser::Expression> expression) {
        auto structure = std::make_shared<Structure>();
        structure->name = "Object";
        structure->function = true;
        structure->array.insert(structure);
        structure->array.insert(Bool);
        structure->array.insert(Char);
        structure->array.insert(Int);
        structure->array.insert(Float);

        std::set<std::string> properties;
        iterate(expression, properties);
        for (auto const& property : properties) {
            auto & p = structure->properties[property];
            p.insert(structure);
            p.insert(Bool);
            p.insert(Char);
            p.insert(Int);
            p.insert(Float);
        }

        MetaData meta_data;
        meta_data.structures.insert(structure);
        return meta_data;
    }

}
