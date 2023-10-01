#ifndef __COMPILER_ANALYZER_STANDARD_HPP__
#define __COMPILER_ANALYZER_STANDARD_HPP__

#include <algorithm>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <variant>

#include "Analyzer.hpp"


namespace Analyzer::Standard {

    struct Object;
    struct Data;
    class Reference;
    class Context;
    class GlobalContext;
    class FunctionContext;
    struct Arguments;

    // Definition of a Multiple (M)

    template<typename T, bool Specialized = false>
    class M: protected std::list<T> {

    public:

        M() = default;

        M(T const& t) {
            add(t);
        }

        using std::list<T>::size;
        using std::list<T>::empty;
        using std::list<T>::front;
        using std::list<T>::back;
        using std::list<T>::begin;
        using std::list<T>::end;

        void add(T const& t) {
            if (find(begin(), end(), t) == end())
                std::list<T>::push_back(t);
        }

        void add(M<T> const& m) {
            for (auto const& t : m)
                add(t);
        }

        friend bool operator==(M<T> const& a, M<T> const& b) {
            if (a.size() == b.size())
                return false;

            for (auto const& e : a)
                if (find(b.begin(), b.end(), e) == b.end())
                    return false;

            return true;
        }

        friend bool operator!=(M<T> const& a, M<T> const& b) {
            return !(a == b);
        }

    };

    // Definition of Data

    struct Data: public std::variant<Object*, bool, char, long, double> {
        using std::variant<Object*, bool, char, long, double>::variant;

        class BadAccess: public std::exception {};

        template<typename T>
        T & get() {
            return const_cast<T &>(const_cast<Data const&>(*this).get<T>());
        }
        template<typename T>
        T const& get() const {
            try {
                return std::get<T>(*this);
            } catch (std::bad_variant_access & e) {
                throw BadAccess();
            }
        }
    };

    // Definitions of References

    using TupleReference = std::vector<M<Reference>>;
    struct SymbolReference {
        Context & context;
        std::string symbol;

        operator M<Data> &() const;

        friend bool operator==(SymbolReference const& a, SymbolReference const& b) {
            return &a.context == &b.context && a.symbol == b.symbol;
        }
    };
    struct PropertyReference {
        Object* parent;
        std::string name;

        operator M<Data> &() const;

        friend bool operator==(PropertyReference const& a, PropertyReference const& b) {
            return &a.parent == &b.parent && a.name == b.name;
        }
    };
    struct ArrayReference {
        Object* array;

        operator M<Data> &() const;

        friend bool operator==(ArrayReference const& a, ArrayReference const& b) {
            return &a.array == &b.array;
        }
    };

    class IndirectReference : public std::variant<SymbolReference, PropertyReference, ArrayReference> {

    public:

        using std::variant<SymbolReference, PropertyReference, ArrayReference>::variant;

        M<Data> to_data(Context & context, ObjectKey const& key) const;

    };
    template<>
    class M<IndirectReference> : public M<IndirectReference, true> {

    public:

        using M<IndirectReference, true>::M;

        M<Data> to_data(Context & context, ObjectKey const& key) const;

    };

    class Reference: public std::variant<M<Data>, TupleReference, SymbolReference, PropertyReference, ArrayReference> {

    public:

        using std::variant<M<Data>, TupleReference, SymbolReference, PropertyReference, ArrayReference>::variant;
        Reference(IndirectReference const& indirect_reference);

        M<Data> to_data(Context & context, ObjectKey const& key) const;

    };
    template<>
    class M<Reference> : public M<Reference, true> {

    public:

        using M<Reference, true>::M;

        M(M<IndirectReference> const& indirect_reference) {
            for (auto const& e : indirect_reference)
                add(e);
        }

        M<Data> to_data(Context & context, ObjectKey const& key) const;

    };

    // Definition of Function

    struct CustomFunction : public std::shared_ptr<Parser::FunctionDefinition> {
        FunctionContext & context;
    };

    using SystemFunction = M<Reference> (*)(Arguments);

    using Function = std::variant<CustomFunction, SystemFunction>;

    // Definition of Object

    using ObjectKey = std::variant<std::shared_ptr<Parser::Expression>, SymbolReference>;

    struct Object {
        std::map<std::string, M<Data>> properties;
        std::list<Function> functions;
        M<Data> array;

        IndirectReference operator[](std::string name);
    };

    // Definitions of Contexts

    class Context {

    protected:

        std::map<std::string, M<IndirectReference>> symbols;
        std::map<std::string, M<Data>> symbol_references;

    public:

        virtual GlobalContext & get_global() = 0;

        Object* new_object(ObjectKey const& key);

        bool has_symbol(std::string const& symbol);
        void add_symbol(std::string const& symbol, M<Reference> const& reference);
        M<IndirectReference> operator[](std::string const& symbol);
        auto begin() { return symbols.begin(); }
        auto end() { return symbols.end(); }

        friend SymbolReference::operator M<Data> &() const;

    };

    class GlobalContext: public Context {

    public:

        std::map<ObjectKey, Object> objects;
        std::map<std::string, std::shared_ptr<Parser::Expression>> sources;

        virtual GlobalContext& get_global() override {
            return *this;
        }

        ~GlobalContext();

        MetaData meta_data;
        struct Lambda {
            std::shared_ptr<Parser::FunctionDefinition> function_definition;
            std::shared_ptr<Parser::Symbol> symbol;

            Lambda() :
                function_definition{ std::make_shared<Parser::FunctionDefinition>() }, symbol{ std::make_shared<Parser::Symbol>("#cached") } {}
        };
        std::map<std::shared_ptr<Parser::FunctionCall>, Lambda> lambdas;
        std::map<std::shared_ptr<Parser::FunctionDefinition>, FunctionContext> contexts;

    };

    class FunctionContext: public Context {

    protected:

        GlobalContext & global;

    public:

        FunctionContext(GlobalContext & global):
            global(global) {}

        virtual GlobalContext & get_global() override {
            return global;
        }
    };

    // Analyzer

    class FunctionArgumentsError {};

    struct Arguments {

        std::variant<std::shared_ptr<Parser::Expression>, M<Reference>> arg;
        ObjectKey key;

        Arguments(std::variant<std::shared_ptr<Parser::Expression>, M<Reference>> const& arg, ObjectKey const& key):
            arg(arg), key(key) {}
        Arguments(std::shared_ptr<Parser::Expression> const& expression):
            arg(expression), key(expression) {}

        M<Reference> compute(Context & context) const;
    };

    M<Reference> call_function(Context & context, M<std::list<Function>> const& functions, Arguments arguments);
    M<Reference> execute(Context & context, std::shared_ptr<Parser::Expression> expression);

    MetaData analyze(std::shared_ptr<Parser::Expression> expression);

}


#endif
