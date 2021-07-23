#include <iostream>
#include <fstream>

#include "Tree.cpp"


int main(int argc, char** argv) {
    if (true) {
        std::ifstream src("test.fl");
        std::string code = "";
        std::string line = "";
        while (std::getline(src, line)) code += line + " ";
        std::shared_ptr<Expression> tree = getTree(code);

    } else {
        std::cout << "Error: please give the name of the source and the name of the destination." << std::endl;
    }
	
	return 0;
}