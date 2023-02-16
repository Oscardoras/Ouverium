#ifndef __COMPILER_ANALYZER_HPP__
#define __COMPILER_ANALYZER_HPP__

#include <list>
#include <map>
#include <set>
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
    class M: public std::set<T> {
    public:
        M() = default;
        M(T const& t) {
            this->push_back(t);
        }
        template<typename U>
        M(M<U> const& m) {
            for (U const& e : m)
                this->push_back(T(e));
        }
    };

    struct Data: public std::variant<Object*, bool, char, long, double> {
        class BadAccess: public std::exception {};

        bool defined = true;

        using std::variant<Object*, bool, char, long, double>::variant;

        template<typename T>
        T & const get() const {
            try {
                return std::get<T>(*this);
            } catch (std::bad_variant_access & e) {
                throw BadAccess();
            }
        }

        template<typename T>
        T & get() {
            return const_cast<T &>(const_cast<Data const&>(*this).get<T>());
        }
    };

    using SymbolReference = std::reference_wrapper<M<Data>>;
    template<>
    class M<SymbolReference> : public M<SymbolReference, true> {
        public:
        using M<SymbolReference, true>::M;

        M<Data> to_data() const;
    };

    using TupleReference = std::vector<M<Reference>>;

    class Reference: public std::variant<M<Data>, SymbolReference, std::vector<M<Reference>>> {
        public:
        using std::variant<M<Data>, SymbolReference, TupleReference>::variant;

        M<Data> to_data(Context & context) const;
        SymbolReference to_symbol_reference(Context & context) const;
    };
    template<>
    class M<Reference> : public M<Reference, true> {
        public:
        using M<Reference, true>::M;

        M<Data> to_data(Context & context) const;
        M<SymbolReference> to_symbol_reference(Context & context) const;
    };

    struct Object {
        std::shared_ptr<Expression> creation;

        std::map<std::string, M<Data>> properties;
        std::list<Function> functions;
        std::vector<M<Data>> array;

        SymbolReference get_property(Context & context, std::string name);
    };

    class Context: public Parser::Context {

    protected:

        std::reference_wrapper<Context> parent;
        std::map<std::string, M<SymbolReference>> symbols;

    public:

        Context(Context & parent, std::shared_ptr<Parser::Position> position):
            Parser::Context(position), parent(parent) {}

        virtual Context& get_parent() override;
        virtual GlobalContext& get_global();

        Object* new_object();
        Object* new_object(std::vector<M<Data>> const& array);
        Object* new_object(std::string const& data);
        SymbolReference new_reference(M<Data> data);

        bool has_symbol(std::string const& symbol);
        M<SymbolReference> & operator[](std::string const& symbol);
        auto begin() {
            return symbols.begin();
        }
        auto end() {
            return symbols.end();
        }
    };
    class GlobalContext: public Context {

    protected:

        std::list<Object> objects;
        std::list<M<Data>> references;

    public:

        std::map<std::string, std::shared_ptr<Expression>> files;

        GlobalContext();

        virtual GlobalContext& get_global() override;

        friend Object* Context::new_object();
        friend Object* Context::new_object(std::vector<M<Data>> const& array);
        friend Object* Context::new_object(std::string const& str);
        friend SymbolReference Context::new_reference(M<Data> data);

    };

    struct SystemFunction {
        std::shared_ptr<Expression> parameters;
        M<Reference> (*pointer)(Context&, bool);
    };
    using FunctionPointer = std::variant<std::shared_ptr<FunctionDefinition>, SystemFunction>;
    struct Function {
        std::map<std::string, M<SymbolReference>> extern_symbols;
        FunctionPointer ptr;
        Function(FunctionPointer const& ptr): ptr(ptr) {}
    };


    class Error: public std::exception {};
    class FunctionArgumentsError: public Error {};

    M<Reference> call_function(Context & context, bool potential, std::shared_ptr<Parser::Position> position, std::list<Function> const& functions, M<Reference> const& arguments);
    M<Reference> call_function(Context & context, bool potential, std::shared_ptr<Parser::Position> position, std::list<Function> const& functions, std::shared_ptr<Expression> arguments);
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
