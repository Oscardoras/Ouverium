#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stddef.h>


#define BYTE uint8_t

#if (SIZE_MAX == UINT64_MAX)
#define OV_INT int64_t
#define OV_FLOAT double
#elif (SIZE_MAX == UINT32_MAX)
#define OV_INT int32_t
#define OV_FLOAT float
#elif (SIZE_MAX == UINT16_MAX)
#define OV_INT int16_t
#define OV_FLOAT float
#else
#define OV_INT int
#define OV_FLOAT float
#endif


#endif
