#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stddef.h>


#define BYTE uint8_t

#if (SIZE_MAX == UINT64_MAX)
#define INT int64_t
#define FLOAT double
#elif (SIZE_MAX == UINT32_MAX)
#define INT int32_t
#define FLOAT float
#elif (SIZE_MAX == UINT16_MAX)
#define INT int16_t
#define FLOAT float
#else
#define INT long
#define FLOAT double
#endif


#endif
