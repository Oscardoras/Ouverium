#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include <stddef.h>

#include "reference.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*__FunctionFilter)(__Reference_Shared args);
typedef __Reference_Owned (*__FunctionBody)(__Reference_Shared args);

/**
 * Represents a function in a function stack.
*/
typedef struct __Function {
    struct __Function* next;
    __FunctionFilter filter;
    __FunctionBody body;
    struct {
        unsigned short size;
        __Reference_Owned* tab;
    } references;
} __Function;

/**
 * Represents a function stack.
*/
typedef __Function* __Function_Stack;


__Function_Stack __Function_new();

void __Function_push(__Function_Stack* function, __FunctionBody body, __FunctionFilter filter, __Reference_Owned references[], size_t references_size);
void __Function_pop(__Function_Stack* function);

__Reference_Owned __Function_eval(__Function_Stack function, __Reference_Shared args);

void __Function_free(__Function_Stack* function);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#include <cstdlib>
#include <vector>


class Function {

public:

    using Filter = bool (*)(Reference const& args);
    using Body = Reference (*)(Reference const& args);

    template<Filter f>
    static bool filter(__Reference_Shared args) {
        Reference r{(__Reference_Owned) args};
        bool b = f(r);
        r = (Reference) NULL;
        return b;
    }

    template<Body f>
    static __Reference_Owned body(__Reference_Shared args) {
        Reference r{(__Reference_Owned) args};
        __Reference_Owned o = f(r);
        r = (Reference) NULL;
        return o;
    }

protected:

    __Function_Stack & stack;

public:

    Function(__Function_Stack & function):
        stack{function} {}

    operator __Function_Stack() const {
        return stack;
    }

    void push(__FunctionBody body, __FunctionFilter filter = nullptr, std::vector<Reference> && references = {}) {
        __Function_push(&stack, body, filter, (__Reference_Owned *) references.data(), references.size());
    }

    template<Body body, Filter filter = nullptr>
    void push(std::vector<Reference> && references = {}) {
        push(Function::body<body>, Function::filter<filter>, references);
    }

    void pop() {
        __Function_pop(&stack);
    }

    Reference operator()(Reference const& args) const {
        return std::move(Reference(__Function_eval(stack, args)));
    }

    void clear() {
        __Function_free(&stack);
    }

};

#endif


#endif
