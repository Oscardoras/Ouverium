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
 * Represents a function in a function linked list.
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


__Reference_Owned __Function_eval(__Function* function, __Reference_Shared args);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#include <cstdlib>
#include <vector>


class Function {

public:

    using Filter = bool (*)(Reference const& args);
    using Body = Reference && (*)(Reference const& args);

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

    __Function* & begin;

public:

    Function(__Function* & function):
        begin{function} {}

    ~Function() {
        
    }

    operator __Function*() const {
        return begin;
    }

    void push(__FunctionBody body, __FunctionFilter filter = nullptr, std::vector<Reference> const& references = {}) {
        __Function* function = (__Function*) std::malloc(sizeof(__Function) + references.size() * sizeof(Reference));

        function->next = begin;
        function->filter = filter;
        function->body = body;
        function->references.size = references.size();
        function->references.tab = (__Reference_Owned*) function+1;

        for (size_t i = 0; i < references.size(); ++i)
            function->references.tab[i] = std::move(references[i]);

        begin = function;
    }

    template<Body body, Filter filter = nullptr>
    void push(std::vector<Reference> const& references = {}) {
        push(Function::body<body>, Function::filter<filter>, references);
    }

    void pop() {
        begin = begin->next;
    }

};

#endif


#endif
