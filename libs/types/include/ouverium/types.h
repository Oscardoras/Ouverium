#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stddef.h>


typedef unsigned char BYTE;

#if (SIZE_MAX == UINT64_MAX)
typedef int64_t OV_INT;
typedef double OV_FLOAT;
#elif (SIZE_MAX == UINT32_MAX)
typedef int32_t OV_INT;
typedef float OV_FLOAT;
#elif (SIZE_MAX == UINT16_MAX)
typedef int16_t OV_INT;
typedef float OV_FLOAT;
#else
typedef int OV_INT;
typedef float OV_FLOAT;
#endif


#endif
