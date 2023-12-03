#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "../parser/Standard.hpp"
#include "SimpleAnalyzer.hpp"
#include "c/Translator.hpp"


int main(int argc, char ** argv) {
    auto path = argc > 1 ? argv[1] : "../examples/test3.fl";

    std::ifstream src(path);
    std::ostringstream oss;
    oss << src.rdbuf();
    std::string code = oss.str();

    auto expression = Parser::Standard(code, path).get_tree();
    std::set<std::string> symbols = {";", ":", "print"};
    expression->compute_symbols(symbols);
    auto meta_data = Analyzer::simple_analize(expression);
    auto translator = Translator::CStandard::Translator(expression, meta_data);

    translator.translate("build/out");
}
