#ifndef __FUNCTION_H__
#define __FUNCTION_H__


typedef struct Function_t {
    bool (*filter)();
    void* (*body)();
} Function;


#endif
