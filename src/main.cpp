#include <iostream>
#include <fstream>

#include "interpreter/Interpreter.hpp"


int main(int argc, char ** argv) {
    if (argc == 1) {
        GlobalContext context;
        Interpreter::setStandardContext(context);

        std::string line;
        while (std::getline(std::cin, line))
            if (line.length() > 0) {
                auto r = Interpreter::run(context, ".", line);
                if (Interpreter::print(std::cout, r.toObject(context)))
                    std::cout << std::endl;
            }
    } else if (argc == 2) {
        std::ifstream src(argv[1]);
        if (src) {
            std::string code;
            std::string line;
            while (std::getline(src, line))
                code += line + '\n';
            
            GlobalContext context;
            Interpreter::setStandardContext(context);
            Interpreter::run(context, argv[1], code);
        } else std::cerr << "Error: unable to load the source file " << argv[1] << "." << std::endl;
    } else std::cerr << "Error: please give the name of the source and the name of the destination." << std::endl;
	
	return 0;
}