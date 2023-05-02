#include <stdlib.h>

#include "gc.h"


int main(int argc, char* *argv) {
    __GC_init(NULL);


    __GC_end();
}
