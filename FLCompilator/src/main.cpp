#include <iostream>
#include <fstream>

#include "parser/Standard.hpp"
#include "compiler/javascript/Translator.hpp"
#include "interpreter/Interpreter.hpp"


int main(int argc, char ** argv) {
    std::vector<std::string> symbols = {";", "$", ":=", ":", "=", "!=", "===", "!", "&", "|", "+", "-", "*", "/", "%", "print"};

    //std::cout << sizeof(int) << " " << sizeof(void*) << std::endl;

    if (argc == 1) {
        std::string line = "";
        while (std::getline(std::cin, line)) {
            std::shared_ptr<Expression> tree = StandardParser::getTree(line, symbols);
            Interpreter::run(tree);
        }
    } else if (argc == 2) {
        std::ifstream src(argv[1]);
        if (src) {
            std::string code = "";
            std::string line = "";
            while (std::getline(src, line)) code += line + " ";
            std::shared_ptr<Expression> tree = StandardParser::getTree(code, symbols);
            //std::cout << tree->toString() << std::endl;
            //std::string js = JavascriptTranslator::getJavaScript(tree);
            Interpreter::run(tree);

            //std::cout << js << std::endl;
        } else std::cerr << "Error: unable to load the source file " << argv[1] << "." << std::endl;
    } else std::cerr << "Error: please give the name of the source and the name of the destination." << std::endl;
	
	return 0;
}