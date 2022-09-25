#include "ArrayList.hpp"


namespace ArrayList {

    std::shared_ptr<Expression> array_list();
    Reference array_list(FunctionContext & context);

    Reference get_array_element(FunctionContext & context) {
        FunctionContext function_context(context, context.position);
        function_context.add_symbol("array", context.get_symbol("this"));
        function_context.add_symbol("i", context.get_symbol("i"));

        return Array::get_array_element(function_context);
    }

    std::shared_ptr<Expression> foreach() {
        auto function = std::make_shared<Symbol>();
        function->name = "function";
        return function;
    }
    Reference foreach(FunctionContext & context) {
        auto array = context.get_symbol("this").to_object(context);
        auto function = context.get_symbol("function").to_object(context);

        for (long i = 1; i < array->type; i++) {
            Interpreter::call_function(context, context.position, function->functions, std::make_shared<Tuple>());
        }

        return Reference(context.new_object());
    }

    Reference lenght(FunctionContext & context) {
        FunctionContext function_context(context, context.position);
        function_context.add_symbol("array", context.get_symbol("this"));

        return Array::get_array_size(function_context);
    }

    Reference is_empty(FunctionContext & context) {
        FunctionContext function_context(context, context.position);
        function_context.add_symbol("array", context.get_symbol("this"));

        return Reference(context.new_object(Array::get_array_size(function_context).to_object(context)->data.i == 0));
    }

    Reference get_capacity(FunctionContext & context) {
        FunctionContext function_context(context, context.position);
        function_context.add_symbol("array", context.get_symbol("this"));

        return Array::get_array_capacity(function_context);
    }

    std::shared_ptr<Expression> set_capacity() {
        auto capacity = std::make_shared<Symbol>();
        capacity->name = "capacity";
        return capacity;
    }
    Reference set_capacity(FunctionContext & context) {
        FunctionContext function_context(context, context.position);
        function_context.add_symbol("array", context.get_symbol("this"));
        function_context.add_symbol("capacity", context.get_symbol("capacity"));

        return Array::set_array_capacity(function_context);
    }

    Reference get_first(FunctionContext & context) {
        auto array = context.get_symbol("this").to_object(context);

        return Reference(array, (unsigned long) 0);
    }

    Reference get_last(FunctionContext & context) {
        auto array = context.get_symbol("this").to_object(context);

        return Reference(array, array->type-1);
    }

    std::shared_ptr<Expression> add_element() {
        auto element = std::make_shared<Symbol>();
        element->name = "element";
        return element;
    }

    Reference add_first(FunctionContext & context) {
        auto array = context.get_symbol("this");
        auto array_object = array.to_object(context);
        auto element = context.get_symbol("element");

        if (array_object->type > 0) {
            FunctionContext function_context(context, context.position);
            function_context.add_symbol("array", array);
            function_context.get_symbol("element").get_reference() = array_object->data.a[array_object->type].o;
            Array::add_array_element(function_context);

            for (long i = array_object->type-1; i > 1; i--)
                array_object->data.a[i+1].o = array_object->data.a[i-1+1].o;
            
            array_object->data.a[1].o = element.to_object(context);
            return Reference(array_object, (unsigned long) 0);
        } else {
            FunctionContext function_context(context, context.position);
            function_context.add_symbol("array", array);
            function_context.add_symbol("element", element);
            return Array::add_array_element(function_context);
        }
    }

    Reference add_last(FunctionContext & context) {
        auto array = context.get_symbol("this");
        auto element = context.get_symbol("element");

        FunctionContext function_context(context, context.position);
        function_context.add_symbol("array", array);
        function_context.add_symbol("element", element);
        return Array::add_array_element(function_context);
    }

    Reference remove_first(FunctionContext & context);

    Reference remove_last(FunctionContext & context);

    std::shared_ptr<Expression> insert();
    Reference insert(FunctionContext & context);

    std::shared_ptr<Expression> remove();
    Reference remove(FunctionContext & context);

    Reference iterator_first(FunctionContext & context);

    Reference iterator_last(FunctionContext & context);

    Reference iterator(FunctionContext & context);

    std::shared_ptr<Expression> iterator_index();
    Reference iterator_index(FunctionContext & context);

    void init(Context & context);

}
