#ifndef __COMPILER_ANALYZER_HPP__
#define __COMPILER_ANALYZER_HPP__

#include <list>
#include <map>
#include <variant>

#include "../Expressions.hpp"


namespace Analyzer {

    class Function;
    class Object;
    class Data;
    class Reference;
    class Context;
    class GlobalContext;

    template<typename T, bool Specialized = false>
    class M: public std::list<T> {
    public:
        inline M() = default;
        inline M(T const& t) {
            this->push_back(t);
        }
        template<typename U>
        inline M(M<U> const& m) {
            for (U const& e : m)
                this->push_back(T(e));
        }
    };

    struct Data: public std::variant<Object*, bool, char, long, double> {
        class BadAccess {};

        using std::variant<Object*, bool, char, long, double>::variant;
        bool defined = true;

        template<typename T>
        inline T & get() {
            try {
                return std::get<T>(*this);
            } catch (std::bad_variant_access & e) {
                throw BadAccess();
            }
        }
    };

    using SymbolReference = std::reference_wrapper<M<Data>>;
    template<>
    class M<Reference> : public M<Reference, true> {
        public:
        using M<Reference, true>::M;

        M<Data> to_data(Context & context) const;
        M<SymbolReference> to_symbol_reference(Context & context) const;
    };
    using TupleReference = std::vector<M<Reference>>;
    class Reference: public std::variant<M<Data>, SymbolReference, std::vector<M<Reference>>> {
        public:
        using std::variant<M<Data>, SymbolReference, TupleReference>::variant;

        M<Data> to_data(Context & context) const;
        SymbolReference to_symbol_reference(Context & context) const;
    };

    struct Object {
        std::shared_ptr<Expression> creation;

        std::map<std::string, M<Data>> properties;
        std::list<std::shared_ptr<Function>> functions;
        std::vector<M<Data>> array;

        SymbolReference get_property(Context & context, std::string name);
    };

    class Context {
        protected:
        std::reference_wrapper<Context> parent;
        std::map<std::string, M<SymbolReference>> symbols;

        public:
        Context(Context & parent);

        Context& get_parent();
        virtual GlobalContext& get_global();

        Object* new_object();
        Object* new_object(std::vector<M<Data>> const& array);
        Object* new_object(std::string const& data);
        SymbolReference new_reference(M<Data> data);

        M<SymbolReference> & operator[](std::string const& symbol);
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
        std::list<M<Data>> references;

        GlobalContext();
        virtual GlobalContext& get_global();
    };

    struct SystemFunction {
        std::shared_ptr<Expression> parameters;
        M<Reference> (*pointer)(Context&, bool);
    };
    using FunctionPointer = std::variant<std::shared_ptr<FunctionDefinition>, SystemFunction>;
    struct Function {
        std::map<std::string, M<SymbolReference>> extern_symbols;
        FunctionPointer ptr;
        inline Function(FunctionPointer const& ptr): ptr(ptr) {}
    };


    struct FunctionArgumentsError {};

    M<Reference> call_function(Context & context, bool potential, std::shared_ptr<Parser::Position> position, std::list<std::shared_ptr<Function>> const& functions, std::shared_ptr<Expression> arguments);
    M<Reference> execute(Context & context, bool potential, std::shared_ptr<Expression> expression);


/*
    struct Symbol {
        std::string name;
    };
*/

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
