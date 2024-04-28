#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <ouverium/include.h>


void Ov_init_functions_base();
void Ov_init_functions_math();

void Ov_init_functions() {
    Ov_init_functions_base();
    Ov_init_functions_math();
}
