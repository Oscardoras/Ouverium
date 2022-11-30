#ifndef __INTERPRETER_REFERENCE_HPP__
#define __INTERPRETER_REFERENCE_HPP__

#include "Object.hpp"


namespace Interpreter {

    class Context;

    struct Reference {

        enum ReferenceType {
            SymbolReference = -3,
            PropertyReference = -2,
            ArrayReference = -1,
            Pointer = 0
            //Tuple > 0
        };
        long type;

        union {
            Object** symbol_reference;
            struct {
                Object* parent;
                Object** reference;
            } property_reference;
            struct {
                Object* array;
                unsigned long i;
            } array_reference;
            Object* pointer;
            Reference *tuple;
        };

        Reference();
        Reference(Reference const& reference);
        Reference(Object** const& symbol_reference);
        Reference(Object* const& parent, Object** const& reference);
        Reference(Object* const& array, unsigned long const& i);
        Reference(Object* const& pointer);
        Reference(size_t const& tuple_size);

        ~Reference();

        bool is_reference() const;
        Object*& get_reference() const;

        Object* to_object(Context & context) const;

        Reference& operator=(Reference const& reference);

    };

}


#endif
