#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>

#include <ouverium/compiler/Analyzer.hpp>

#include <ouverium/parser/Expressions.hpp>


namespace Analyzer {

    struct SimpleAnalyzer {
        struct {
            std::map<std::string, std::set<Function>> functions;
            std::map<std::shared_ptr<Parser::FunctionCall>, std::string> calls;
        } trivial_calls;
        std::map<std::shared_ptr<Parser::FunctionCall>, std::set<Function>> calls;
        std::set<std::string> properties;

        std::map<std::filesystem::path, std::shared_ptr<Parser::Expression>> sources;
        std::map<std::shared_ptr<Parser::FunctionCall>, std::string> imports;

        std::filesystem::path import_file(std::shared_ptr<Parser::Expression> expression, std::string const& p);
        void iterate(std::shared_ptr<Parser::Expression> expression);

        MetaData analize(std::shared_ptr<Parser::Expression> expression);
    };

}
