#ifndef __COMPILER_ANALYZER_HPP__
#define __COMPILER_ANALYZER_HPP__

#include <list>
#include <variant>

#include "../Expressions.hpp"


namespace Analyzer {

    struct Function;

    struct Object;
    using ObjectPtr = Object*;
    using Data = std::variant<char, double, long, bool, std::vector<ObjectPtr>>;
    struct Object {
        std::shared_ptr<Expression> creation;
        bool accurate;

        std::map<std::string, ObjectPtr> properties;
        std::list<std::shared_ptr<Function>> functions;
        Data data;

        ObjectPtr& get_property(Context & context, std::string name);
    };

    class Reference: public std::variant<ObjectPtr, std::reference_wrapper<ObjectPtr>, std::vector<Reference>> {
        public:
        Reference(ObjectPtr ptr);
        Reference(std::reference_wrapper<ObjectPtr> reference);
        Reference(std::vector<Reference> const& tuple);
        ObjectPtr to_object(Context & context) const;
        ObjectPtr& to_reference(Context & context) const;
    };

    class Context {
        protected:
        std::reference_wrapper<GlobalContext> global;
        std::map<std::string, std::reference_wrapper<ObjectPtr>> symbols;

        public:
        Context(GlobalContext& global);

        GlobalContext& get_global() const;

        ObjectPtr new_object();
        ObjectPtr new_object(Data const& data);
        ObjectPtr new_object(std::string const& data);
        ObjectPtr& new_reference(ObjectPtr object);

        ObjectPtr& operator[](std::string const& symbol);
        bool has_symbol(std::string const& symbol);
    };
    class GlobalContext: public Context {
        public:
        std::list<Object> objects;
        std::list<ObjectPtr> references;
    };

    struct SystemFunction {
        std::shared_ptr<Expression> parameters;
        Reference (*pointer)(Context&);
    };
    using FunctionPointer = std::variant<std::shared_ptr<FunctionDefinition>, SystemFunction>;
    struct Function {
        std::map<std::string, ObjectPtr> extern_symbols;
        FunctionPointer ptr;
    };


    struct FunctionArgumentsError {};

    Reference execute(Context & context, std::shared_ptr<Expression> expression);


    struct Symbol {
        std::string name;
    };

    struct FunctionEnvironment {
        std::shared_ptr<FunctionDefinition> expression;
        std::vector<Symbol> parameters;
        std::vector<Symbol> local_variables;
    };

    struct GlobalEnvironment {
        std::vector<Symbol> global_variables;
        std::vector<FunctionEnvironment> functions;
    };

    struct Type {
        bool reference;
        bool pointer;
        std::map<std::string, std::shared_ptr<Type>> properties;
    };
    struct MetaData {
        std::map<std::shared_ptr<Expression>, std::shared_ptr<Type>> types;
        std::map<std::shared_ptr<FunctionCall>, std::vector<FunctionPointer>> links;
    };

}


#endif
