#ifndef __COMPILER_ANALYZER_STANDARD_HPP__
#define __COMPILER_ANALYZER_STANDARD_HPP__

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <variant>

#include "Analyzer.hpp"


namespace Analyzer::Standard {

    struct Function;
    struct Object;
    struct Data;
    class Reference;
    class Context;
    class GlobalContext;

    // Definition of a Multiple (M)

    template<typename T, bool Specialized = false>
    class M: protected std::list<T> {

    public:

        M() = default;

        M(T const& t) {
            add(t);
        }

        template<typename U>
        M(M<U> const& m) {
            for (U const& e : m)
                add(T(e));
        }

        using std::list<T>::size;
        using std::list<T>::empty;
        using std::list<T>::front;
        using std::list<T>::back;
        using std::list<T>::begin;
        using std::list<T>::end;
        using std::list<T>::operator==;

        void add(T const& t) {
            if (find(std::list<T>::begin(), std::list<T>::end(), t) == std::list<T>::end())
                std::list<T>::push_back(t);
        }

        void add(M<T> const& m) {
            for (auto const& t : m)
                add(t);
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

    using IndirectReference = std::reference_wrapper<M<Data>>;
    template<>
    class M<IndirectReference> : public M<IndirectReference, true> {
    public:
        using M<IndirectReference, true>::M;

        operator M<Data &>() const {
            M<Data> m;
            for (auto & reference : *this)
                for (auto & data : reference.get())
                    m.add(data);
            return m;
        }
    };

    using TupleReference = std::vector<M<Reference>>;

    class Reference: public std::variant<M<Data>, IndirectReference, std::vector<M<Reference>>> {
    public:
        using std::variant<M<Data>, IndirectReference, TupleReference>::variant;

        M<Data> to_data(Context & context) const;
        IndirectReference to_indirect_reference(Context & context) const;
    };
    template<>
    class M<Reference> : public M<Reference, true> {
    public:
        using M<Reference, true>::M;

        M<Data> to_data(Context & context) const;
        M<IndirectReference> to_indirect_reference(Context & context) const;
    };

    // Definition of Object

    struct Object {
        std::map<std::string, M<Data>> properties;
        std::list<Function> functions;
        std::vector<M<Data>> array;

        Object() = default;

        Object(std::string const& str) {
            array.reserve(str.size());
            for (auto c : str)
                array.push_back(Data(c));
        }

        IndirectReference get_property(Context & context, std::string name);
    };

    // Definition of Function

    using CustomFunction = std::shared_ptr<Parser::FunctionDefinition>;

    struct SystemFunction {
        std::shared_ptr<Parser::Expression> parameters;
        M<Reference> (*pointer)(Context&, bool);
    };

    struct Function : public std::variant<CustomFunction, SystemFunction> {
        std::map<std::string, M<IndirectReference>> extern_symbols;

        using std::variant<CustomFunction, SystemFunction>::variant;
    };

    // Definitions of Contexts

    class Context: public Parser::Context {

    protected:

        std::map<std::string, M<IndirectReference>> symbols;

    public:

        Context(std::shared_ptr<Parser::Expression> expression):
            Parser::Context(expression) {}

        Object* new_object();
        Object* new_object(Object && object);

        M<Data> & new_reference(M<Data> const& data = {});

        bool has_symbol(std::string const& symbol);
        M<IndirectReference> & operator[](std::string const& symbol);
        auto begin() { return symbols.begin(); }
        auto end() { return symbols.end(); }
    };

    class GlobalContext: public Context {

    protected:

        std::list<Object> objects;
        std::list<M<Data>> references;

    public:

        std::map<std::string, std::shared_ptr<Parser::Expression>> files;

        GlobalContext(std::shared_ptr<Parser::Expression> expression):
            Context(expression) {}

        virtual GlobalContext& get_global() override {
            return *this;
        }

        virtual Context& get_parent() override {
            return *this;
        }

        friend Object* Context::new_object();
        friend Object* Context::new_object(Object && object);
        friend M<Data> & Context::new_reference(M<Data> const& data);
        friend std::shared_ptr<Expression> analyze(std::shared_ptr<Parser::Expression> expression);

    };

    class Context: public Parser::Context {

    protected:

        std::map<std::string, M<IndirectReference>> symbols;

    public:

        Context(std::shared_ptr<Parser::Expression> expression):
            Parser::Context(expression) {}

        virtual Context& get_parent() override {
            return this->parent;
        }
        virtual GlobalContext& get_global() {
            return this->parent.get_global();
        }

        Object* new_object();
        Object* new_object(std::vector<M<Data>> const& array);
        Object* new_object(std::string const& data);
        IndirectReference new_reference(M<Data> data);

        bool has_symbol(std::string const& symbol);
        M<IndirectReference> & operator[](std::string const& symbol);
        auto begin() {
            return symbols.begin();
        }
        auto end() {
            return symbols.end();
        }
    };

    class CacheContext {
        std::map<std::string, M<Reference>> symbols;
    };

    // Analyzer

    class Analyzer: public ::Analyzer::Analyzer {

    protected:

        MetaData meta_data;
        std::map<std::shared_ptr<Parser::FunctionDefinition>, CacheContext> cache_contexts;

        class Error: public std::exception {};
        class FunctionArgumentsError: public Error {};

        struct Analysis {
            M<Reference> references;
            std::shared_ptr<Expression> expression;
        };

        using It = std::map<std::string, M<IndirectReference>>::iterator;

        void set_references(Context & function_context, std::shared_ptr<Parser::Expression> parameters, M<Reference> const& reference);
        std::shared_ptr<Expression> set_references(Context & context, bool potential, Context & function_context, std::map<std::shared_ptr<Parser::Expression>, Analyzer::Analysis> & computed, std::shared_ptr<Parser::Expression> parameters, std::shared_ptr<Parser::Expression> arguments);
        void call_reference(M<Reference> & references, bool potential, Function const& function, Context function_context, It const it, It const end);

    public:

        using Arguments = std::variant<std::shared_ptr<Parser::Expression>, Reference>;
        Analysis call_function(Context & context, bool potential, std::shared_ptr<Parser::Expression> expression, M<std::list<Function>> const& functions, Arguments arguments);
        Analysis execute(Context & context, bool potential, std::shared_ptr<Parser::Expression> expression);

        void create_structures(GlobalContext const& context);

        virtual std::pair<std::shared_ptr<Expression>, MetaData> analyze(std::shared_ptr<Parser::Expression> expression) override;

    };

}


#endif
