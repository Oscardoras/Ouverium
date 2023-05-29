#ifndef __COMPILER_ANALYZER_STANDARD_HPP__
#define __COMPILER_ANALYZER_STANDARD_HPP__

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <variant>

#include "Analyzer.hpp"


namespace Analyzer {

    namespace Standard {

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
            std::shared_ptr<Parser::Expression> creation;

            std::map<std::string, M<Data>> properties;
            std::list<Function> functions;
            std::vector<M<Data>> array;

            SymbolReference get_property(Context & context, std::string name);
        };

        struct SystemFunction {
            std::shared_ptr<Parser::Expression> parameters;
            M<Reference> (*pointer)(Context&, bool);
        };
        using FunctionPointer = std::variant<std::shared_ptr<Parser::FunctionDefinition>, SystemFunction>;
        inline bool operator<(FunctionPointer const& a, FunctionPointer const& b) {
            if (a.index() == b.index()) {
                if (auto p = std::get_if<std::shared_ptr<Parser::FunctionDefinition>>(&a))
                    return p->get() < std::get_if<std::shared_ptr<Parser::FunctionDefinition>>(&b)->get();
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

            Context& parent;
            std::map<std::string, M<SymbolReference>> symbols;

        public:

            Context(Context & parent, std::shared_ptr<Parser::Position> position):
                Parser::Context(position), parent(parent) {}

            virtual Context& get_parent() override {
                return this->parent;
            }
            virtual GlobalContext& get_global() {
                return this->parent.get_global();
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

            std::map<std::string, std::shared_ptr<Parser::Expression>> files;

            GlobalContext():
                Context(*this) {}

            virtual GlobalContext& get_global() override {
                return *this;
            }

            friend Object* Context::new_object();
            friend Object* Context::new_object(std::vector<M<Data>> const& array);
            friend Object* Context::new_object(std::string const& str);
            friend SymbolReference Context::new_reference(M<Data> data);
            friend std::shared_ptr<AnalyzedExpression> analyze(std::shared_ptr<Parser::Expression> expression);

        };

        class CacheContext {
            std::map<std::string, M<Reference>> symbols;
        };

        class Analyzer: public ::Analyzer::Analyzer {

        protected:

            MetaData meta_data;
            std::map<std::shared_ptr<Parser::FunctionDefinition>, CacheContext> cache_contexts;

            class Error: public std::exception {};
            class FunctionArgumentsError: public Error {};

            struct Analysis {
                M<Reference> references;
                std::shared_ptr<AnalyzedExpression> expression;
            };

            using It = std::map<std::string, M<SymbolReference>>::iterator;

            void set_references(Context & function_context, std::shared_ptr<Parser::Expression> parameters, M<Reference> const& reference);
            std::shared_ptr<AnalyzedExpression> set_references(Context & context, bool potential, Context & function_context, std::map<std::shared_ptr<Parser::Expression>, Analyzer::Analysis> & computed, std::shared_ptr<Parser::Expression> parameters, std::shared_ptr<Parser::Expression> arguments);
            void call_reference(M<Reference> & references, bool potential, Function const& function, Context function_context, It const it, It const end);

        public:

            Analysis call_function(Context & context, bool potential, std::shared_ptr<Parser::Position> position, M<std::list<Function>> const& all_functions, std::shared_ptr<Parser::Expression> arguments);
            Analysis execute(Context & context, bool potential, std::shared_ptr<Parser::Expression> expression);

            virtual std::shared_ptr<AnalyzedExpression> analyze(std::shared_ptr<Parser::Expression> expression) const override;

        };

    }

}


#endif
