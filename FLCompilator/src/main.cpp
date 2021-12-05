#include <iostream>
#include <fstream>

#include "compilator/javascript/Translator.cpp"


int main(int argc, char ** argv) {
    if (true || (argc == 2)) {
        std::ifstream src(argv[1]);
        std::string code = "";
        std::string line = "";
        while (std::getline(src, line)) code += line + " ";
        std::shared_ptr<Expression> tree = getTree(code);
        std::cout << printTree(tree) << std::endl;
        std::string js = getJavaScript(tree);

        std::cout << js << std::endl;
    } else {
        std::cerr << "Error: please give the name of the source and the name of the destination." << std::endl;
    }
	
	return 0;
}