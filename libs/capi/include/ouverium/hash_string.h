#ifndef __HASH_STRING_H__
#define __HASH_STRING_H__

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
    constexpr
    #endif
        static inline uint32_t hash_string(const char* string) {
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
