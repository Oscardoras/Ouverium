#include <iostream>

#include "Context.hpp"
#include "InterpreterError.hpp"


namespace SystemFunctions {

    Reference separator(Reference reference, FunctionContext & context) {
        if (reference.isTuple()) return reference.tuple[reference.getTupleSize()-1];
        else {
            auto object = reference.toObject();
            context.addSymbol("object", Reference(object));
            if (object->type > 0) return Reference(&object->data.a[object->type].o);
            else return reference;
        }
    }

    Reference copy(Reference reference, FunctionContext & context) {
        auto object = reference.toObject();
        context.addSymbol("object", Reference(object));
        return Reference(new Object(*object));
    }

    Reference assign(Reference reference, FunctionContext & context) {
        if (reference.getTupleSize() == 2) {
            auto var = reference.tuple[0];
            auto object = reference.tuple[1].toObject();
            context.addSymbol("object", Reference(object));

            if (var.isPointerReference()) {
                (*var.ptrRef)->removeReference();
                *var.ptrRef = object;
                (*var.ptrRef)->addReference();
                return var;
            } else if (var.isArrayReference()) {
                auto ref = var.getArrayReference();
                (*ref)->removeReference();
                *ref = object;
                (*ref)->addReference();
                return var;
            } else if (var.isTuple()) {
                auto n = var.getTupleSize();
                if ((long) n == object->type) {
                    for (unsigned long i = 0; i < n; i++) {
                        FunctionContext funcContext(context);
                        Reference arguments(2);
                        arguments.tuple[0] = var.tuple[i];
                        arguments.tuple[1] = object->data.a[i+1].o;
                        funcContext.addSymbol("arguments", arguments);

                        assign(arguments, context);

                        funcContext.free();
                    }

                    return var;
                } else throw InterpreterError();
            } else return Reference(object);
        } else {
            auto tuple = reference.toObject();
            context.addSymbol("tuple", Reference(tuple));

            if (tuple->type == 2) return Reference(&tuple->data.a[2].o);
            else throw InterpreterError();
        }
    }

    Reference function_definition(Reference reference, FunctionContext & context) {
        auto tuple = reference.toObject();
        context.addSymbol("tuple", tuple);

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
                            element.second->removeReference();
                    }
                }
                delete var->function;
            }
            if (object->function != nullptr) {
                if (object->function->type == Function::Custom) {
                    var->function = new CustomFunction(((CustomFunction*) object->function)->pointer);
                    for (auto & element : ((CustomFunction*) object->function)->objects) {
                        ((CustomFunction*) var->function)->objects[element.first] = element.second;
                        if (var->references > 0) element.second->addReference();
                    }
                } else var->function = new SystemFunction(((SystemFunction*) object->function)->pointer);
            } else var->function = nullptr;

            if (reference.getTupleSize() == 2) return reference.tuple[0];
            else return Reference(tuple->data.a[1].o);
        } else throw InterpreterError();
    }

    Reference equals(Reference reference, FunctionContext & context) {
        auto tuple = reference.toObject();
        context.addSymbol("tuple", tuple);

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
                            funcContext.free();
                            return Reference(new Object(false));
                        } else funcContext.free();
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
                            funcContext.free();
                            return Reference(new Object(false));
                        } else funcContext.free();
                    }
                    return Reference(new Object(true));
                } else if (object1->type == Object::Boolean)
                    return Reference(new Object(object1->data.b == object2->data.b));
                else if (object1->type == Object::Integer)
                    return Reference(new Object(object1->data.i == object2->data.i));
                else if (object1->type == Object::Float)
                    return Reference(new Object(object1->data.f == object2->data.f));
                else return Reference(new Object(true));
            } else return Reference(new Object(false));
        } else throw InterpreterError();
    }

    Reference not_equals(Reference reference, FunctionContext & context) {
        auto r = equals(reference, context);
        r.ptrCopy->data.b = !r.ptrCopy->data.b;
        return r;
    }

    Reference checkPointers(Reference reference, FunctionContext & context) {
        auto tuple = reference.toObject();
        context.addSymbol("tuple", tuple);

        if (tuple->type == 2) return Reference(new Object(tuple->data.a[1].o == tuple->data.a[2].o));
        else throw InterpreterError();
    }

    Reference logicalNot(Reference reference, FunctionContext & context) {
        auto object = reference.toObject();
        context.addSymbol("object", object);
        
        if (object->type == Object::Boolean) return Reference(new Object(!object->data.b));
        else throw InterpreterError();
    }

    Reference logicalAnd(Reference reference, FunctionContext & context) {
        auto tuple = reference.toObject();
        context.addSymbol("tuple", tuple);

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Boolean && object2->type == Object::Boolean)
                return Reference(new Object(object1->data.b && object2->data.b));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference logicalOr(Reference reference, FunctionContext & context) {
        auto tuple = reference.toObject();
        context.addSymbol("tuple", tuple);

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Boolean && object2->type == Object::Boolean)
                return Reference(new Object(object1->data.b || object2->data.b));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference print(Reference reference, FunctionContext & context) {
        auto object = reference.toObject();
        context.addSymbol("object", object);

        if (object->type == Object::Boolean) std::cout << object->data.b << std::endl;
        else if (object->type == Object::Integer) std::cout << object->data.i << std::endl;
        else if (object->type == Object::Float) std::cout << object->data.f << std::endl;
        else if (object->type >= 0)
            for (long i = 1; i <= object->type; i++) {
                FunctionContext funcContext(context);
                auto arguments = Reference(object->data.a[i].o);
                funcContext.addSymbol("arguments", arguments);

                print(arguments, funcContext);

                funcContext.free();
            }

        return Reference(new Object());
    }

    Reference addition(Reference reference, FunctionContext & context) {
        auto tuple = reference.toObject();
        context.addSymbol("tuple", tuple);

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

    Reference substraction(Reference reference, FunctionContext & context) {
        auto tuple = reference.toObject();
        context.addSymbol("tuple", tuple);

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

    Reference multiplication(Reference reference, FunctionContext & context) {
        auto tuple = reference.toObject();
        context.addSymbol("tuple", tuple);

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

    Reference division(Reference reference, FunctionContext & context) {
        auto tuple = reference.toObject();
        context.addSymbol("tuple", tuple);

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

    Reference modulo(Reference reference, FunctionContext & context) {
        auto tuple = reference.toObject();
        context.addSymbol("tuple", tuple);

        if (tuple->type == 2) {
            auto object1 = tuple->data.a[1].o;
            auto object2 = tuple->data.a[2].o;
            
            if (object1->type == Object::Integer && object2->type == Object::Integer)
                return Reference(new Object(object1->data.i % object2->data.i));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference getTupleSize(Reference reference, FunctionContext & context) {
        if (reference.isTuple())
            return Reference(new Object((long) reference.getTupleSize()));
        else {
            auto object = reference.toObject();
            context.addSymbol("object", object);
            
            if (object->type >= 0)
                return Reference(new Object((long) object->type));
            else throw InterpreterError();
        }
    }

    Reference getTupleElement(Reference reference, FunctionContext & context) {
        auto tuple = reference.toObject();
        context.addSymbol("tuple", tuple);

        if (tuple->type == 2) {
            auto array = tuple->data.a[1].o;
            auto index = tuple->data.a[2].o;
            
            if (array->type > 0 && index->type == Object::Integer)
                return Reference(&array->data.a[index->data.i].o);
            else throw InterpreterError();
        } else throw InterpreterError();
    }

}