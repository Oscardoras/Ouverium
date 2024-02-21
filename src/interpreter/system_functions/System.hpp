#ifndef __INTERPRETER_SYSTEMFUNCTIONS_SYSTEM_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_SYSTEM_HPP__

#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    namespace System {

        Reference stream_is(FunctionContext& context);
        Reference stream_read(FunctionContext& context);
        Reference stream_has(FunctionContext& context);
        Reference stream_write(FunctionContext& context);
        Reference stream_flush(FunctionContext& context);

        Reference file_open(FunctionContext& context);
        Reference file_get_working_directory(FunctionContext& context);
        Reference file_set_working_directory(FunctionContext& context);
        Reference file_exists(FunctionContext& context);
        Reference file_is_directory(FunctionContext& context);
        Reference file_create_directories(FunctionContext& context);
        Reference file_copy(FunctionContext& context);
        Reference file_delete(FunctionContext& context);

        Reference time(FunctionContext& context);
        Reference clock_system(FunctionContext& context);
        Reference clock_steady(FunctionContext& context);

        Reference thread_is(FunctionContext& context);
        Reference thread_create(FunctionContext& context);
        Reference thread_join(FunctionContext& context);
        Reference thread_detach(FunctionContext& context);
        Reference thread_get_id(FunctionContext& context);
        Reference thread_current_id(FunctionContext& context);
        Reference thread_sleep(FunctionContext& context);
        Reference thread_hardware_concurrency(FunctionContext& context);

        Reference mutex_is(FunctionContext& context);
        Reference mutex_create(FunctionContext& context);
        Reference mutex_lock(FunctionContext& context);
        Reference mutex_try_lock(FunctionContext& context);
        Reference mutex_unlock(FunctionContext& context);

        Reference weak_reference(FunctionContext& context);
        Reference GC_collect(FunctionContext& context);


        void init(GlobalContext& context);

    }

}


#endif
