#include <iostream>

#include "Context.hpp"
#include "InterpreterError.hpp"


int checkObject(Object* & object, Reference & result) {
    if (result.isReference()) {
        if (result.ptrRef == &object) {
            result.type = Reference::PointerCopy;
            result.ptrCopy = object;
            return 1;
        } else return 2;
    } else if (result.isArrayReference()) {
        if (result.getArrayReference() == &object) {
            result.type = Reference::PointerCopy;
            result.ptrCopy = object;
            return 1;
        } else return 2;
    } else if (result.isCopy()) {
        return result.ptrCopy == object && object->references == 1 ? 1 : 2;
    } else {
        int r = 2;
        for (auto i = 0; i < result.type; i++) {
            int t = checkObject(object, result.tuple[i]);
            if (t == 0 && r > 0) r = 0;
            else if (t == 1 && r > 1) r = 1;
        }
        return r;
    }
}

void freeContext(Object* object, Reference & result) {
    if (object->references <= 0) throw "Negative references";

    object->references--;
    if (object->references == 0) {
        for (auto & element : object->fields) {
            int r = checkObject(element.second, result);
            if (r == 2) freeContext(element.second, result);
            else if (r == 1) element.second->references--;
        }
        if (object->function != nullptr) {
            if (object->function->type == Function::Custom)
                for (auto & element : ((CustomFunction*) object->function)->objects) {
                    int r = checkObject(element.second, result);
                    if (r == 2) freeContext(element.second, result);
                    else if (r == 1) element.second->references--;
                }
            delete object->function;
        }

        if (object->type >= 0)
            for (auto i = 0; i < object->type; i++) {
                int r = checkObject(object->data.tuple[i], result);
                if (r == 2) freeContext(object->data.tuple[i], result);
                else if (r == 1) object->data.tuple[i]->references--;
            }

        object->type = Object::Deleting;
        delete object;
    }
}


namespace SystemFunctions {

    Reference separator(Reference reference) {
        if (reference.getTupleSize() == 2) {
            reference.tuple[0].unuse();
            return reference.tuple[1];
        } else {
            auto object = reference.toObject();
            if (object->type == 2) {
                auto last = object->data.tuple[1];
                if (object->references == 0) {
                    object->data.tuple[1] = nullptr;
                    delete object;
                }
                return Reference(last);
            } else {
                if (object->references == 0) delete object;
                throw InterpreterError();
            }
        }
    }

    Reference copy(Reference reference) {
        auto object = reference.toObject();
        auto obj = new Object(*object);
        if (object->references == 0) delete object;
        return Reference(obj);
    }

    Reference assign(Reference reference) {
        if (reference.getTupleSize() == 2) {
            auto var = reference.tuple[0];
            auto object = reference.tuple[1].toObject();

            if (var.isReference()) {
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
                if ((int) n == object->type) {
                    for (auto i = 0; i < (int) n; i++) {
                        Reference t(2);
                        t.tuple[0] = var.tuple[i];
                        t.tuple[1] = object->data.tuple[i];
                        assign(t);
                    }

                    if (object->references == 0) delete object;
                    return var;
                } else {
                    if (object->references == 0) delete object;
                    throw InterpreterError();
                }
            } else {
                var.unuse();
                return Reference(object);
            }
        } else {
            auto tuple = reference.toObject();
            if (tuple->type == 2) {
                auto r = tuple->data.tuple[1];
                if (tuple->references == 0) {
                    tuple->data.tuple[1] = nullptr;
                    delete tuple;
                } else tuple->data.tuple[0] = r;
                return Reference(r);
            } else {
                if (tuple->references == 0) delete tuple;
                throw InterpreterError();
            }
        }
    }

    Reference function_definition(Reference reference) {
        auto tuple = reference.toObject();
        if (tuple->type == 2) {
            auto var = tuple->data.tuple[0];
            auto object = tuple->data.tuple[1];

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

            if (reference.getTupleSize() == 2) {
                tuple->data.tuple[0] = nullptr;
                delete tuple;
                return reference.tuple[0];
            } else {
                auto r = tuple->data.tuple[0];
                if (tuple->references == 0) {
                    tuple->data.tuple[0] = nullptr;
                    delete tuple;
                }
                return Reference(r);
            }
        } else {
            if (tuple->references == 0) delete tuple;
            throw InterpreterError();
        }
    }

    Reference equals(Reference reference) {
        auto tuple = reference.toObject();
        if (tuple->type == 2) {
            auto object1 = tuple->data.tuple[0];
            auto object2 = tuple->data.tuple[1];
            
            if (object1->type == object2->type) {
                for (auto const& element : object1->fields) {
                    auto it = object2->fields.find(element.first);
                    if (it != object2->fields.end()) {
                        Reference t(2);
                        t.tuple[0] = Reference(element.second);
                        t.tuple[1] = Reference(it->second);
                        auto obj = equals(t).toObject();
                        if (!obj->data.b) {
                            if (obj->references == 0) delete obj;
                            if (tuple->references == 0) delete tuple;
                            return Reference(new Object(false));
                        }
                        if (obj->references == 0) delete obj;
                    } else {
                        if (tuple->references == 0) delete tuple;
                        return Reference(new Object(false));
                    }
                }

                if (object1->function != nullptr && object2->function != nullptr) {
                    if (object1->function->type == object2->function->type) {
                        if (object1->function->type == Function::Custom) {
                            if (((CustomFunction*) object1->function)->pointer != ((CustomFunction*) object2->function)->pointer) {
                                if (tuple->references == 0) delete tuple;
                                return Reference(new Object(false));
                            }
                            if (((CustomFunction*) object1->function)->objects != ((CustomFunction*) object2->function)->objects) {
                                if (tuple->references == 0) delete tuple;
                                return Reference(new Object(false));
                            }
                        } else if (((SystemFunction*) object1->function)->pointer != ((SystemFunction*) object2->function)->pointer) {
                            if (tuple->references == 0) delete tuple;
                            return Reference(new Object(false));
                        }
                    } else {
                        if (tuple->references == 0) delete tuple;
                        return Reference(new Object(false));
                    }
                } else if (object1->function != object2->function) {
                    if (tuple->references == 0) delete tuple;
                    return Reference(new Object(false));
                }

                if (object1->type >= 0) {
                    for (auto i = 0; i < object1->type; i++) {
                        Reference t(2);
                        t.tuple[0] = Reference(object1->data.tuple[i]);
                        t.tuple[1] = Reference(object2->data.tuple[i]);
                        auto obj = equals(t).toObject();
                        if (!obj->data.b) {
                            if (obj->references == 0) delete obj;
                            if (tuple->references == 0) delete tuple;
                            return Reference(new Object(false));
                        }
                        if (obj->references == 0) delete obj;
                    }
                } else if (object1->type == Object::Boolean) {
                    auto r = new Object(object1->data.b == object2->data.b);
                    if (tuple->references == 0) delete tuple;
                    return Reference(r);
                } else if (object1->type == Object::Integer) {
                    auto r = new Object(object1->data.i == object2->data.i);
                    if (tuple->references == 0) delete tuple;
                    return Reference(r);
                } else if (object1->type == Object::Float) {
                    auto r = new Object(object1->data.f == object2->data.f);
                    if (tuple->references == 0) delete tuple;
                    return Reference(r);
                }
                if (tuple->references == 0) delete tuple;
                return Reference(new Object(true));
            } else {
                if (tuple->references == 0) delete tuple;
                return Reference(new Object(false));
            }
        } else {
            if (tuple->references == 0) delete tuple;
            throw InterpreterError();
        }
    }

    Reference not_equals(Reference reference) {
        auto r = equals(reference);
        r.ptrCopy->data.b = !r.ptrCopy->data.b;
        return r;
    }

    Reference checkPointers(Reference reference) {
        auto tuple = reference.toObject();
        if (tuple->type == 2) {
            auto r = new Object(tuple->data.tuple[0] == tuple->data.tuple[1]);
            if (tuple->references == 0) delete tuple;
            return Reference(r);
        } else {
            if (tuple->references == 0) delete tuple;
            throw InterpreterError();
        }
    }

    Reference logicalNot(Reference reference) {
        auto object = reference.toObject();
        if (object->type == Object::Boolean) {
            auto r = new Object(!object->data.b);
            if (object->references == 0) delete object;
            return Reference(r);
        } else {
            if (object->references == 0) delete object;
            throw InterpreterError();
        }
    }

    Reference logicalAnd(Reference reference) {
        auto tuple = reference.toObject();
        if (tuple->type == 2) {
            auto object1 = tuple->data.tuple[0];
            auto object2 = tuple->data.tuple[1];
            
            if (object1->type == Object::Boolean && object2->type == Object::Boolean)
                return Reference(new Object(object1->data.b && object2->data.b));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference logicalOr(Reference reference) {
        auto tuple = reference.toObject();
        if (tuple->type == 2) {
            auto object1 = tuple->data.tuple[0];
            auto object2 = tuple->data.tuple[1];
            
            if (object1->type == Object::Boolean && object2->type == Object::Boolean)
                return Reference(new Object(object1->data.b || object2->data.b));
            else throw InterpreterError();
        } else throw InterpreterError();
    }

    Reference print(Reference reference) {
        auto object = reference.toObject();

        if (object->type == Object::Boolean) std::cout << object->data.b << std::endl;
        else if (object->type == Object::Integer) std::cout << object->data.i << std::endl;
        else if (object->type == Object::Float) std::cout << object->data.f << std::endl;
        else if (object->type >= 0)
            for (auto i = 0; i < object->type; i++)
                print(Reference(object->data.tuple[i]));

        if (object->references == 0) delete object;
        return Reference(new Object());
    }

    Reference addition(Reference reference) {
        auto tuple = reference.toObject();
        if (tuple->type == 2) {
            auto object1 = tuple->data.tuple[0];
            auto object2 = tuple->data.tuple[1];
            
            if (object1->type == Object::Integer && object2->type == Object::Integer) {
                if (tuple->references == 0) delete tuple;
                return Reference(new Object(object1->data.i + object2->data.i));
            } else if (object1->type == Object::Float && object2->type == Object::Float) {
                if (tuple->references == 0) delete tuple;
                return Reference(new Object(object1->data.f + object2->data.f));
            } else {
                if (tuple->references == 0) delete tuple;
                throw InterpreterError();
            }
        } else {
            if (tuple->references == 0) delete tuple;
            throw InterpreterError();
        }
    }

    Reference substraction(Reference reference) {
        auto tuple = reference.toObject();
        if (tuple->type == 2) {
            auto object1 = tuple->data.tuple[0];
            auto object2 = tuple->data.tuple[1];
            
            if (object1->type == Object::Integer && object2->type == Object::Integer) {
                if (tuple->references == 0) delete tuple;
                return Reference(new Object(object1->data.i - object2->data.i));
            } else if (object1->type == Object::Float && object2->type == Object::Float) {
                if (tuple->references == 0) delete tuple;
                return Reference(new Object(object1->data.f - object2->data.f));
            } else {
                if (tuple->references == 0) delete tuple;
                throw InterpreterError();
            }
        } else {
            if (tuple->references == 0) delete tuple;
            throw InterpreterError();
        }
    }

    Reference multiplication(Reference reference) {
        auto tuple = reference.toObject();
        if (tuple->type == 2) {
            auto object1 = tuple->data.tuple[0];
            auto object2 = tuple->data.tuple[1];
            
            if (object1->type == Object::Integer && object2->type == Object::Integer) {
                if (tuple->references == 0) delete tuple;
                return Reference(new Object(object1->data.i * object2->data.i));
            } else if (object1->type == Object::Float && object2->type == Object::Float) {
                if (tuple->references == 0) delete tuple;
                return Reference(new Object(object1->data.f * object2->data.f));
            } else {
                if (tuple->references == 0) delete tuple;
                throw InterpreterError();
            }
        } else {
            if (tuple->references == 0) delete tuple;
            throw InterpreterError();
        }
    }

    Reference division(Reference reference) {
        auto tuple = reference.toObject();
        if (tuple->type == 2) {
            auto object1 = tuple->data.tuple[0];
            auto object2 = tuple->data.tuple[1];
            
            if (object1->type == Object::Integer && object2->type == Object::Integer) {
                if (tuple->references == 0) delete tuple;
                return Reference(new Object(object1->data.i / object2->data.i));
            } else if (object1->type == Object::Float && object2->type == Object::Float) {
                if (tuple->references == 0) delete tuple;
                return Reference(new Object(object1->data.f / object2->data.f));
            } else {
                if (tuple->references == 0) delete tuple;
                throw InterpreterError();
            }
        } else {
            if (tuple->references == 0) delete tuple;
            throw InterpreterError();
        }
    }

    Reference modulo(Reference reference) {
        auto tuple = reference.toObject();
        if (tuple->type == 2) {
            auto object1 = tuple->data.tuple[0];
            auto object2 = tuple->data.tuple[1];
            
            if (object1->type == Object::Integer && object2->type == Object::Integer) {
                auto r = new Object(object1->data.i % object2->data.i);
                if (tuple->references == 0) delete tuple;
                return Reference(r);
            } else {
                if (tuple->references == 0) delete tuple;
                throw InterpreterError();
            }
        } else {
            if (tuple->references == 0) delete tuple;
            throw InterpreterError();
        }
    }

    Reference getTupleSize(Reference reference) {
        if (reference.isTuple()) {
            auto r = new Object((long) reference.getTupleSize());
            reference.unuse();
            return Reference(r);
        } else {
            auto object = reference.toObject();
            
            if (object->type >= 0) {
                auto r = new Object((long) object->type);
                if (object->references == 0) delete object;
                return Reference(r);
            } else {
                if (object->references == 0) delete object;
                throw InterpreterError();
            }
        }
    }

    Reference getTupleElement(Reference reference) {
        auto tuple = reference.toObject();
        if (tuple->type == 2) {
            auto array = tuple->data.tuple[0];
            auto index = tuple->data.tuple[1];
            
            if (array->type >= 0 && index->type == Object::Integer) {
                Reference r = Reference(&array->data.tuple[index->data.i]);
                freeContext(tuple, r);
                return r;
            } else {
                if (tuple->references == 0) delete tuple;
                throw InterpreterError();
            }
        } else {
            if (tuple->references == 0) delete tuple;
            throw InterpreterError();
        }
    }

}