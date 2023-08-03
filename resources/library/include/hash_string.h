#ifndef __HASH_STRING_H__
#define __HASH_STRING_H__

#include <stdint.h>


uint32_t hash(const char* string) {
    uint32_t hash = 0;
    uint32_t pow = 31;

    for (; *string != '\0'; ++string) {
        hash += *string * pow;
        pow *= 31;
    }

    return hash;
}

#ifdef __cplusplus
constexpr uint32_t hash(std::string const& string) {
    return hash(string.c_str());
}
#endif


#endif
