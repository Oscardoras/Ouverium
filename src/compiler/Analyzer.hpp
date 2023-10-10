#ifndef __COMPILER_ANALYZER_HPP__
#define __COMPILER_ANALYZER_HPP__

#include <map>
#include <memory>
#include <set>
#include <string>

#include "../parser/Parser.hpp"


namespace Analyzer {

    struct Type {
        virtual ~Type() = default;
    };
    using Types = std::vector<std::weak_ptr<Type>>;

    inline std::shared_ptr<Type> Bool = std::make_shared<Type>();
    inline std::shared_ptr<Type> Char = std::make_shared<Type>();
    inline std::shared_ptr<Type> Int = std::make_shared<Type>();
    inline std::shared_ptr<Type> Float = std::make_shared<Type>();

    struct Structure: public Type {
        std::map<std::string, std::set<std::weak_ptr<Type>>> properties;
        std::set<std::weak_ptr<Type>> array;
        bool function = false;
    };


    struct MetaData {
        std::set<std::shared_ptr<Structure>> structures;
        //std::map<std::shared_ptr<Parser::Expression>, std::set<std::shared_ptr<Type>>> types;
        //std::set<std::shared_ptr<Parser::Expression>> lambdas;
    };

}


#endif
