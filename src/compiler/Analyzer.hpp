#ifndef __COMPILER_ANALYZER_HPP__
#define __COMPILER_ANALYZER_HPP__

#include <list>
#include <map>
#include <variant>

#include "../Expressions.hpp"


namespace Analyzer {

    template<typename T>
    class M: public std::list<T> {
        public:
        inline M() = default;
        inline M(std::list<T> const& l): std::list<T>(l);
        inline M(T const& t) {
            push_back(t);
        }
    };

    class Object;
    using Variant = std::variant<Object*, bool, char, long, double>;
    struct Data: public Variant {
        using Variant::Variant;
        bool defined = true;
    };
    
    struct Function;
    struct Object {
        std::shared_ptr<Expression> creation;

        std::map<std::string, M<Data>> properties;
        std::list<std::shared_ptr<Function>> functions;
        std::vector<M<Data>> array;

        M<std::reference_wrapper<Data>> get_property(Context & context, std::string name);
    };

    class Reference: public std::variant<Data, std::reference_wrapper<Data>, std::vector<M<Reference>>> {
        public:
        inline Reference(Data data): std::variant<Data, std::reference_wrapper<Data>, std::vector<M<Reference>>>(data) {}
        inline Reference(std::reference_wrapper<Data> reference) : std::variant<Data, std::reference_wrapper<Data>, std::vector<M<Reference>>>(reference) {}
        inline Reference(std::vector<M<Reference>> const& tuple) : std::variant<Data, std::reference_wrapper<Data>, std::vector<M<Reference>>>(tuple) {}

        Data to_data(Context & context) const;
        Data& to_reference(Context & context) const;
    };

    class GlobalContext;
    class Context {
        protected:
        std::reference_wrapper<Context> parent;
        std::map<std::string, M<std::reference_wrapper<Data>>> symbols;

        public:
        Context(Context& parent);

        Context& get_parent();
        virtual GlobalContext& get_global();

        Object* new_object();
        Object* new_object(std::vector<M<Data>> const& array);
        Object* new_object(std::string const& data);
        Data& new_reference(Data data);

        M<std::reference_wrapper<Data>> operator[](std::string const& symbol);
        bool has_symbol(std::string const& symbol);
        inline auto begin() {
            return symbols.begin();
        }
        inline auto end() {
            return symbols.end();
        }
    };
    class GlobalContext: public Context {
        public:
        std::list<Object> objects;
        std::list<Data> references;

        GlobalContext();
        virtual GlobalContext& get_global();
    };

    struct SystemFunction {
        std::shared_ptr<Expression> parameters;
        M<Reference> (*pointer)(Context&);
    };
    using FunctionPointer = std::variant<std::shared_ptr<FunctionDefinition>, SystemFunction>;
    struct Function {
        std::map<std::string, M<std::reference_wrapper<Data>>> extern_symbols;
        FunctionPointer ptr;
    };


    struct FunctionArgumentsError {};

    M<Reference> execute(Context & context, std::shared_ptr<Expression> expression);


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
