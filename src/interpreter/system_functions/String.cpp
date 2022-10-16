#include "String.hpp"
#include "Types.hpp"


namespace String {

    std::shared_ptr<Expression> string() {
        auto array = std::make_shared<Symbol>();
        array->name = "this";
        return array;
    }
    Reference char_constructor(FunctionContext & context) {
        auto array = context.get_symbol("this");
        auto array_object = array.to_object(context);

        if (array_object->type == Object::Char) {
            return array;
        } else if (array_object->type == 1 && array_object->data.a[1].o->type == Object::Char) {
            return Reference(array_object, (unsigned long) 0);
        } else throw Interpreter::FunctionArgumentsError();
    }

    std::shared_ptr<Expression> char_is() {
        auto c = std::make_shared<Symbol>();
        c->name = "c";
        return c;
    }

    Reference char_is_digit(FunctionContext & context) {
        auto c = context.get_symbol("c").to_object(context);
        return Reference(context.new_object('0' <= c->data.c && c->data.c <= '9'));
    }

    Reference char_is_alpha(FunctionContext & context) {
        auto c = context.get_symbol("c").to_object(context);
        return Reference(context.new_object(
            'A' <= c->data.c && c->data.c <= 'Z' && 'a' <= c->data.c && c->data.c <= 'z'
        ));
    }
    Reference char_is_alphanum(FunctionContext & context) {
        auto b1 = String::char_is_digit(context).to_object(context)->data.b;
        auto b2 = String::char_is_alpha(context).to_object(context)->data.b;
        return Reference(context.new_object(b1 || b2));
    }

    Reference string(FunctionContext & context) {
        auto array = context.get_symbol("this");
        auto array_object = array.to_object(context);

        FunctionContext function_context(context, context.position);
        function_context.add_symbol("this", array);
        ArrayList::array_list(function_context);
        function_context.add_symbol("object", array);
        function_context.add_symbol("type", context.get_global()->get_symbol("String"));
        Types::set_type(function_context);

        auto f = new SystemFunction(index_of(), index_of);
        f->extern_symbols["this"] = array;
        array_object->get_property("index_of", context)->functions.push_front(f);

        f = new SystemFunction(substring(), substring);
        f->extern_symbols["this"] = array;
        array_object->get_property("substring", context)->functions.push_front(f);

        return array;
    }

    std::shared_ptr<Expression> index_of() {
        auto substring = std::make_shared<Symbol>();
        substring->name = "substring";
        return substring;
    }
    Reference index_of(FunctionContext & context) {
        try {
            auto array = context.get_symbol("this").to_object(context)->to_string();
            auto substring = context.get_symbol("substring").to_object(context)->to_string();

            return Reference(context.new_object((long) array.find_first_of(substring)));
        } catch (std::exception & ex) {
            throw Interpreter::FunctionArgumentsError();
        }
    }

    std::shared_ptr<Expression> substring() {
        auto tuple = std::make_shared<Tuple>();

        auto begin = std::make_shared<Symbol>();
        begin->name = "begin";
        tuple->objects.push_back(begin);

        auto len = std::make_shared<Symbol>();
        len->name = "len";
        tuple->objects.push_back(len);

        return tuple;
    }
    Reference substring(FunctionContext & context) {
        try {
            auto array = context.get_symbol("this").to_object(context)->to_string();
            auto begin = context.get_symbol("begin").to_object(context)->data.i;
            auto len = context.get_symbol("len").to_object(context)->data.i;

            FunctionContext function_context(context, context.position);
            function_context.add_symbol("this", Reference(context.new_object(array.substr(begin, len))));
            return string(function_context);
        } catch (std::exception & ex) {
            throw Interpreter::FunctionArgumentsError();
        }
    }

    std::shared_ptr<Expression> includes() {
        auto tuple = std::make_shared<Tuple>();

        auto substring = std::make_shared<Symbol>();
        substring->name = "substring";
        tuple->objects.push_back(substring);

        auto string = std::make_shared<Symbol>();
        string->name = "string";
        tuple->objects.push_back(string);

        return tuple;
    }
    Reference includes(FunctionContext & context) {
        auto string = context.get_symbol("string");
        auto substring = context.get_symbol("substring");

        if (Types::check_type(context, substring.to_object(context), context.get_global()->get_symbol("String").to_object(context))) {
            FunctionContext function_context(context, context.position);
            function_context.add_symbol("this", string);
            function_context.add_symbol("substring", substring);

            return Reference(context.new_object(String::index_of(function_context).to_object(context)->data.i >= 0));
        } else throw Interpreter::FunctionArgumentsError();
    }

    std::shared_ptr<Expression> concat() {
        auto tuple = std::make_shared<Tuple>();

        auto str1 = std::make_shared<Symbol>();
        str1->name = "str1";
        tuple->objects.push_back(str1);

        auto str2 = std::make_shared<Symbol>();
        str2->name = "str2";
        tuple->objects.push_back(str2);

        return tuple;
    }
    Reference concat(FunctionContext & context) {
        auto str1 = context.get_symbol("str1").to_object(context);
        auto str2 = context.get_symbol("str2").to_object(context);

        Reference r;
        if (str1->type >= 0 && str2->type >= 0) {
            try {
                r = Reference(context.new_object(str1->to_string() + str2->to_string()));
            } catch (std::exception & ex) {
                throw Interpreter::FunctionArgumentsError();
            }
        } else if (str1->type == Object::Char && str2->type >= 0) {
            try {
                r = Reference(context.new_object(str1->data.c + str2->to_string()));
            } catch (std::exception & ex) {
                throw Interpreter::FunctionArgumentsError();
            }
        } else if (str1->type >= 0 && str2->type == Object::Char) {
            try {
                r = Reference(context.new_object(str1->data.c + str2->to_string()));
            } catch (std::exception & ex) {
                throw Interpreter::FunctionArgumentsError();
            }
        } else if (str1->type == Object::Char && str2->type == Object::Char)
            r = Reference(context.new_object("" + str1->data.c + str2->data.c));
        else throw Interpreter::FunctionArgumentsError();

        FunctionContext function_context(context, context.position);
        function_context.add_symbol("this", r);
        return string(function_context);
    }

    Reference assign_concat(FunctionContext & context) {
        auto str1 = context.get_symbol("str1").to_object(context);
        auto str2 = context.get_symbol("str2").to_object(context);

        if (str1->type >= 0) {
            if (str2->type >= 0) {
                FunctionContext function_context(context, context.position);
                function_context.add_symbol("array", str1);
                function_context.add_symbol("capacity", Reference(context.new_object(str1->type + str2->type)));
                Array::set_array_capacity(function_context);

                for (long i = 0; i < str2->type; i++)
                    str1->data.a[str1->type+i+1].o = str2->data.a[i+1].o;
                str1->type += str2->type;
            } else if (str1->type >= 0 && str2->type == Object::Char) {
                FunctionContext function_context(context, context.position);
                function_context.add_symbol("this", str1);
                function_context.add_symbol("capacity", Reference(context.new_object(str1->type + 1)));
                Array::set_array_capacity(function_context);

                str1->type++;
                str1->data.a[str1->type].o = str2;
            } else throw Interpreter::FunctionArgumentsError();
        } else throw Interpreter::FunctionArgumentsError();

        FunctionContext function_context(context, context.position);
        function_context.add_symbol("this", Reference(str1));
        return string(function_context);
    }

    void init(Context & context) {
        context.get_symbol("Char").to_object(context)->functions.push_front(new SystemFunction(string(), char_constructor));
        context.get_symbol("Char").to_object(context)->get_property("is_digit", context)->functions.push_front(new SystemFunction(char_is(), char_is_digit));
        context.get_symbol("Char").to_object(context)->get_property("is_alpha", context)->functions.push_front(new SystemFunction(char_is(), char_is_alpha));
        context.get_symbol("Char").to_object(context)->get_property("is_alphanum", context)->functions.push_front(new SystemFunction(char_is(), char_is_alphanum));

        context.get_symbol("String").to_object(context)->functions.push_front(new SystemFunction(string(), string));

        context.get_symbol("+").to_object(context)->functions.push_front(new SystemFunction(concat(), concat));
        context.get_symbol("~").to_object(context)->functions.push_front(new SystemFunction(includes(), includes));
        context.get_symbol(":+").to_object(context)->functions.push_front(new SystemFunction(concat(), assign_concat));
    }

}
