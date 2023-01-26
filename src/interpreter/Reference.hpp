#ifndef __INTERPRETER_REFERENCE_HPP__
#define __INTERPRETER_REFERENCE_HPP__

#include "Object.hpp"


namespace Interpreter {

    class Context;

    using SymbolReference = Data*;

    struct Reference {

        enum class Type {
            Symbol,
            Property,
            Array,
            Data,
            Tuple
        } type;

        union {
            SymbolReference symbol_reference;
            struct {
                Object* parent;
                Data* reference;
            } property_reference;
            struct {
                Object* array;
                size_t i;
            } array_reference;
            Data data;
            struct {
                Reference *tuple;
                size_t size;
            } tuple_reference;
        };

        private:
        Reference();

        public:
        Reference(Reference const& reference);
        Reference(SymbolReference symbol_reference);
        Reference(Object* parent, Data* reference);
        Reference(Object* array, size_t i);
        Reference(Data data);
        Reference(size_t tuple_size);

        ~Reference();

        bool is_reference() const;
        Data& get_reference() const;

        Data to_data(Context & context) const;

        Reference& operator=(Reference const& reference);

    };

}


#endif
