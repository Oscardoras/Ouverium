#pragma once

#include "Analyzer.hpp"

#include "../parser/Standard.hpp"


namespace Analyzer {

    struct SimpleAnalyzer {
        std::map<std::shared_ptr<Parser::FunctionCall>, std::set<Function>> calls;
        std::set<std::string> properties;

        std::map<std::filesystem::path, std::shared_ptr<Parser::Expression>> sources;
        std::map<std::shared_ptr<Parser::FunctionCall>, std::string> imports;

        std::filesystem::path import_file(std::shared_ptr<Parser::Expression> expression, std::string const& p);
        void iterate(std::shared_ptr<Parser::Expression> expression);

        MetaData analize(std::shared_ptr<Parser::Expression> expression);
    };

}
