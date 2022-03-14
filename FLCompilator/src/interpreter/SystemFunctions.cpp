#include <algorithm>
#include <iostream>

#include "Context.hpp"
#include "InterpreterError.hpp"


namespace SystemFunctions {

    Reference separator(Reference reference, FunctionContext & context) {
        if (reference.isTuple()) return reference.tuple[reference.getTupleSize()-1];
        else {
            auto object = context.getSymbol("arguments").toObject();
            if (object->type > 0) return Reference(&object->data.a[object->type].o);
            else return reference;
        }
    }

    Reference copy(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto object = context.getSymbol("arguments").toObject();
        return Reference(new Object(*object));
    }

    void assign1(std::vector<Object*> & cache, Reference const& var, Object* const& object) {
        if (var.type >= 0) cache.push_back(object);
        else if (var.isTuple()) {
            auto n = var.getTupleSize();
            if ((long) n == object->type)
                for (long i = 0; i < n; i++) assign1(cache, var.tuple[i], object->data.a[i+1].o);
            else throw InterpreterError();
        }
    }

    void assign2(Context & context, std::vector<Object*>::iterator & it, Reference const& var) {
        if (var.isPointerReference()) {
            context.removeReference(*var.ptrRef);
            *var.ptrRef = *it++;
            context.addReference(*var.ptrRef);
        } else if (var.isArrayReference()) {
            auto ref = var.getArrayReference();
            context.removeReference(*ref);
            *ref = *it++;
            context.addReference(*ref);
        } else if (var.isTuple()) {
            auto n = var.getTupleSize();
            for (long i = 0; i < n; i++) assign2(context, it, var.tuple[i]);
        }
    }

    Reference assign(Reference reference, FunctionContext & context) {
        if (reference.getTupleSize() == 2) {
            auto var = reference.tuple[0];
            auto object = reference.tuple[1].toObject();
            context.addSymbol("object", Reference(object));

            std::vector<Object*> cache;
            assign1(cache, var, object);
            auto it = cache.begin();
            assign2(context, it, var);

            return var;
        } else {
            auto tuple = context.getSymbol("arguments").toObject();

            if (tuple->type == 2) return Reference(&tuple->data.a[2].o);
            else throw InterpreterError();
        }
    }

    Reference function_definition(Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto var = tuple->data.a[1].o;
            auto object = tuple->data.a[2].o;

            if (var->function != nullptr) {
                if (var->function->type == Function::Custom) {
                    if (var->references == 0) {
                        for (auto & element : ((CustomFunction*) var->function)->objects)
                            if (element.second->references == 0) delete element.second;
                    } else {
                        for (auto & element : ((CustomFunction*) var->function)->objects)
                            context.removeReference(element.second);
                    }
                }
                delete var->function;
            }
            if (object->function != nullptr) {
                if (object->function->type == Function::Custom) {
                    var->function = new CustomFunction(((CustomFunction*) object->function)->pointer);
                    for (auto & element : ((CustomFunction*) object->function)->objects) {
                        ((CustomFunction*) var->function)->objects[element.first] = element.second;
                        if (var->references > 0) context.removeReference(element.second);
                    }
                } else var->function = new SystemFunction(((SystemFunction*) object->function)->pointer);
            } else var->function = nullptr;

            if (reference.getTupleSize() == 2) return reference.tuple[0];
            else return Reference(tuple->data.a[1].o);
        } else throw InterpreterError();
    }

    Reference equals(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == object2->type) {
                for (auto const& element : object1->fields) {
                    auto it = object2->fields.find(element.first);
                    if (it != object2->fields.end()) {
                        FunctionContext funcContext(context);
                        Reference arguments(2);
                        arguments.tuple[0] = Reference(element.second);
                        arguments.tuple[1] = Reference(it->second);
                        funcContext.addSymbol("arguments", arguments);

                        auto result = equals(arguments, context).toObject();

                        if (!result->data.b) {
                            funcContext.freeContext();
                            return Reference(new Object(false));
                        } else funcContext.freeContext();
                    } else  return Reference(new Object(false));
                }

                if (object1->function != nullptr && object2->function != nullptr) {
                    if (object1->function->type == object2->function->type) {
                        if (object1->function->type == Function::Custom) {
                            if (((CustomFunction*) object1->function)->pointer != ((CustomFunction*) object2->function)->pointer)
                                return Reference(new Object(false));
                            if (((CustomFunction*) object1->function)->objects != ((CustomFunction*) object2->function)->objects)
                                return Reference(new Object(false));
                        } else if (((SystemFunction*) object1->function)->pointer != ((SystemFunction*) object2->function)->pointer)
                            return Reference(new Object(false));
                    } else
                        return Reference(new Object(false));
                } else if (object1->function != object2->function)
                    return Reference(new Object(false));

                if (object1->type >= 0) {
                    for (long i = 1; i <= object1->type; i++) {
                        FunctionContext funcContext(context);
                        Reference arguments(2);
                        arguments.tuple[0] = Reference(object1->data.a[i].o);
                        arguments.tuple[1] = Reference(object2->data.a[i].o);
                        funcContext.addSymbol("arguments", arguments);

                        auto result = equals(arguments, context).toObject();

                        if (!result->data.b) {
                            funcContext.freeContext();
                            return Reference(new Object(false));
                        } else funcContext.freeContext();
                    }
                    return Reference(new Object(true));
                } else if (object1->type == Object::Boolean)
                    return Reference(new Object(object1->data.b == object2->data.b));
                else if (object1->type == Object::Integer)
                    return Reference(new Object(object1->data.i == object2->data.i));
                else if (object1->type == Object::Float)
                    return Reference(new Object(object1->data.f == object2->data.f));
                else if (object1->type == Object::Char)
                    return Reference(new Object(object1->data.c == object2->data.c));
                else return Reference(new Object(true));
            } else return Reference(new Object(false));
        } else throw InterpreterError();
    }

    Reference not_equals(Reference reference, FunctionContext & context) {
        auto r = equals(reference, context);
        r.ptrCopy->data.b = !r.ptrCopy->data.b;
        return r;
    }

    Reference checkPointers(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) return Reference(new Object(tuple->data.a[1].o == tuple->data.a[2].o));
        else throw InterpreterError();
    }

    Reference logicalNot(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto object = context.getSymbol("arguments").toObject();
        
        if (object->type == Object::Boolean) return Reference(new Object(!object->data.b));
        else throw InterpreterError();
    }

    Reference logicalAnd(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Boolean && object2->type == Object::Boolean)
                return Reference(new Object(object1->data.b && object2->data.b));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference logicalOr(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Boolean && object2->type == Object::Boolean)
                return Reference(new Object(object1->data.b || object2->data.b));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference print(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto object = context.getSymbol("arguments").toObject();

        if (object->type == Object::Boolean) std::cout << object->data.b;
        else if (object->type == Object::Integer) std::cout << object->data.i;
        else if (object->type == Object::Float) std::cout << object->data.f;
        else if (object->type == Object::Char) std::cout << object->data.c;
        else if (object->type > 0)
            for (long i = 1; i <= object->type; i++) {
                FunctionContext funcContext((Context&) context);
                auto arguments = Reference(&object->data.a[i].o);
                funcContext.addSymbol("arguments", arguments);

                auto ref = print(arguments, funcContext);
                context.unuse(ref);
            }

        return Reference(new Object());
    }

    Reference addition(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Integer && object2->type == Object::Integer)
                return Reference(new Object(object1->data.i + object2->data.i));
            else if (object1->type == Object::Float && object2->type == Object::Float)
                return Reference(new Object(object1->data.f + object2->data.f));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference substraction(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == Object::Integer) return Reference(new Object(-tuple->data.i));
        else if (tuple->type == Object::Float) return Reference(new Object(-tuple->data.f));
        else if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Integer && object2->type == Object::Integer)
                return Reference(new Object(object1->data.i - object2->data.i));
            else if (object1->type == Object::Float && object2->type == Object::Float)
                return Reference(new Object(object1->data.f - object2->data.f));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference multiplication(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Integer && object2->type == Object::Integer)
                return Reference(new Object(object1->data.i * object2->data.i));
            else if (object1->type == Object::Float && object2->type == Object::Float)
                return Reference(new Object(object1->data.f * object2->data.f));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference division(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Integer && object2->type == Object::Integer)
                return Reference(new Object(object1->data.i / object2->data.i));
            else if (object1->type == Object::Float && object2->type == Object::Float)
                return Reference(new Object(object1->data.f / object2->data.f));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference modulo(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Integer && object2->type == Object::Integer)
                return Reference(new Object(object1->data.i % object2->data.i));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference get_array_size(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto object = context.getSymbol("arguments").toObject();
        
        if (object->type >= 0)
            return Reference(new Object((long) object->type));
        else throw InterpreterError();
    }

    Reference get_array_element(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto array = tuple->data.a[1].o;
            auto index = tuple->data.a[2].o;
            
            if (array->type > 0 && index->type == Object::Integer)
                return Reference(array, index->data.i);
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference get_array_capacity(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto array = context.getSymbol("arguments").toObject();

        if (array->type == 0) return new Object((long) 0);
        else if (array->type > 0) return new Object((long) array->data.a[0].c);
        else throw InterpreterError();
    }

    Reference set_array_capacity(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 3) {
            auto array = tuple->data.a[1].o;
            auto capacity = tuple->data.a[2].o;
            
            if (capacity->type == Object::Integer && capacity->data.i >= 0) {
                if (array->type <= 0) {
                    if (capacity->data.i > 0) {
                        array->data.a = (Object::Data::ArrayElement *) malloc(sizeof(Object::Data::ArrayElement) * (capacity->data.i+1));
                        array->data.a[0].c = capacity->data.i;
                    }
                    array->type = 0;
                } else if (capacity->data.i != (long) array->data.a[0].c) {
                    if (capacity->data.i < array->type && array->references > 0)
                        for (int i = capacity->data.i; i <= array->type; i++)
                            context.addReference(capacity->data.a[i].o);
                    array->data.a[0].c = capacity->data.i;
                    array->data.a = (Object::Data::ArrayElement *) realloc(array->data.a, sizeof(Object::Data::ArrayElement) * (1 + array->data.a[0].c));
                }

                return new Object();
            } else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference add_array_element(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto tuple = context.getSymbol("arguments").toObject();

        if (tuple->type == 2) {
            auto array = tuple->data.a[1].o;
            auto element = tuple->data.a[2].o;
            
            if (array->type <= 0) {
                array->data.a = (Object::Data::ArrayElement *) malloc(sizeof(Object::Data::ArrayElement) * 2);
                array->data.a[0].c = 1;
            } else if ((long) array->data.a[0].c <= array->type) {
                array->data.a[0].c *= 2;
                array->data.a = (Object::Data::ArrayElement *) realloc(array->data.a, sizeof(Object::Data::ArrayElement) * (1 + array->data.a[0].c));
            }

            array->type++;
            array->data.a[array->type].o = element;
            if (array->references > 0) context.addReference(element);

            return Reference(array, array->type);
        } else throw InterpreterError();
    }

    Reference remove_array_element(__attribute__((unused)) Reference reference, FunctionContext & context) {
        auto array = context.getSymbol("arguments").toObject();

        if (array->type > 0) {
            if (array->references > 0) context.removeReference(array->data.a[array->type].o);
            array->type--;
        } else throw InterpreterError();
    }

}