#include <stdlib.h>

#include "gc.h"
#include "function.h"

__Reference && function(__Reference const& args) {}


int main(int argc, char* *argv) {
    __GC_init();

    __Function f;
    f.body = (__FunctionBody) function;

    __Function_eval(&f, __Reference(nullptr));

    __GC_end();
}
