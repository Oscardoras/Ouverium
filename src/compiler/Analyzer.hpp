#ifndef __COMPILER_ANALYZER_HPP__
#define __COMPILER_ANALYZER_HPP__

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <variant>

#include "../Expressions.hpp"


namespace Analyzer {

    struct Function;
    struct Object;
    struct Data;
    class Reference;
    class Context;
    class GlobalContext;
    struct Type;
    struct MetaData;

    // Definition of a Multiple (M)

    template<typename T, bool Specialized = false>
    class M: protected std::list<T> {

    public:

        void add(T const& t) {
            if (find(std::list<T>::begin(), std::list<T>::end(), t) == std::list<T>::end())
                std::list<T>::push_back(t);
        }

        void add(M<T> const& m) {
            for (auto const& t : m)
                add(t);
        }

        using std::list<T>::size;
        using std::list<T>::empty;
        using std::list<T>::front;
        using std::list<T>::back;
        using std::list<T>::begin;
        using std::list<T>::end;

        bool operator==(M<T> const& m) const {
            if (std::list<T>::size() == m.size()) {
                for (auto const& t : m)
                    if (find(std::list<T>::begin(), std::list<T>::end(), t) == std::list<T>::end())
                        return false;
                return true;
            } else return false;
        }

        M() = default;

        M(T const& t) {
            add(t);
        }

        template<typename U>
        M(M<U> const& m) {
            for (U const& e : m)
                add(T(e));
        }
    };

    // Definition of Data

    struct Data: public std::variant<Object*, bool, char, long, double> {
        class BadAccess: public std::exception {};

        bool defined = true;

        using std::variant<Object*, bool, char, long, double>::variant;

        template<typename T>
        T const& get() const {
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

    // Definitions of References

    using SymbolReference = std::reference_wrapper<M<Data>>;
    inline bool operator==(SymbolReference const& a, SymbolReference const& b) {
        return &a.get() == &b.get();
    }
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

    // Definition of Object

    struct Object {
        std::shared_ptr<Expression> creation;

        std::map<std::string, M<Data>> properties;
        std::list<Function> functions;
        std::vector<M<Data>> array;

        SymbolReference get_property(Context & context, std::string name);
    };

    struct SystemFunction {
        std::shared_ptr<Expression> parameters;
        M<Reference> (*pointer)(Context&, bool);
    };
    using FunctionPointer = std::variant<std::shared_ptr<FunctionDefinition>, SystemFunction>;
    inline bool operator<(FunctionPointer const& a, FunctionPointer const& b) {
        if (a.index() == b.index()) {
            if (auto p = std::get_if<std::shared_ptr<FunctionDefinition>>(&a))
                return p->get() < std::get_if<std::shared_ptr<FunctionDefinition>>(&b)->get();
            if (auto p = std::get_if<SystemFunction>(&a))
                return p->pointer < std::get_if<SystemFunction>(&b)->pointer;
            return false;
        } else
            return a.index() < b.index();
    }
    struct Function {
        std::map<std::string, M<SymbolReference>> extern_symbols;
        FunctionPointer ptr;
        Function(FunctionPointer const& ptr): ptr(ptr) {}
    };

    // Definitions of Contexts

    class Context: public Parser::Context {

    protected:

        std::reference_wrapper<Context> parent;
        std::map<std::string, M<SymbolReference>> symbols;

    public:

        Context(Context & parent, std::shared_ptr<Parser::Position> position):
            Parser::Context(position), parent(parent) {}

        virtual Context& get_parent() override {
            return this->parent.get();
        }
        virtual GlobalContext& get_global() {
            return this->parent.get().get_global();
        }

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

        std::reference_wrapper<MetaData> meta_data;
        std::map<std::string, std::shared_ptr<Expression>> files;

        GlobalContext(MetaData & meta_data):
            Context(*this), meta_data(meta_data) {}

        virtual GlobalContext& get_global() override {
            return *this;
        }

        friend Object* Context::new_object();
        friend Object* Context::new_object(std::vector<M<Data>> const& array);
        friend Object* Context::new_object(std::string const& str);
        friend SymbolReference Context::new_reference(M<Data> data);
        friend MetaData analyze(std::shared_ptr<Expression> expression);

    };

    // Excution functions

    class Error: public std::exception {};
    class FunctionArgumentsError: public Error {};

    M<Reference> call_function(Context & context, bool potential, std::shared_ptr<Parser::Position> position, std::list<Function> const& functions, M<Reference> const& arguments);
    M<Reference> call_function(Context & context, bool potential, std::shared_ptr<Parser::Position> position, std::list<Function> const& functions, std::shared_ptr<Expression> arguments, std::shared_ptr<FunctionCall> function_call);
    M<Reference> execute(Context & context, bool potential, std::shared_ptr<Expression> expression);

    // Analyzis data

    struct FunctionEnvironment {
        std::shared_ptr<FunctionDefinition> expression;
        std::vector<Symbol> parameters;
        std::vector<Symbol> local_variables;
    };

    struct GlobalEnvironment {
        std::vector<Symbol> global_variables;
        std::vector<FunctionEnvironment> functions;
    };

    struct MetaData {
        struct Type {
            virtual ~Type() {}
        };
        struct Structure: public Type, public std::map<std::string, std::set<std::reference_wrapper<Type>>> {};
        struct: public Type {} Pointer;
        struct: public Type {} Bool;
        struct: public Type {} Char;
        struct: public Type {} Int;
        struct: public Type {} Float;

        std::set<Structure> structures;
        std::map<std::shared_ptr<Expression>, std::set<std::reference_wrapper<Type>>> types;
        std::map<std::shared_ptr<FunctionCall>, std::set<FunctionPointer>> links;
    };

    MetaData analyze(std::shared_ptr<Expression> expression);

}


#endif
