#ifndef __HASH_STRING_H__
#define __HASH_STRING_H__

#include <stdint.h>


#ifdef __cplusplus
#include <string>

extern "C" {
constexpr
#endif
uint32_t hash(const char *string) {
    uint32_t hash = 0;
    uint32_t pow = 31;

    for (; *string != '\0'; ++string) {
        hash += *string * pow;
        pow *= 31;
    }

    return hash;
}

#ifdef __cplusplus
}
#endif


#endif
