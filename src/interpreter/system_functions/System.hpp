#ifndef __INTERPRETER_SYSTEMFUNCTIONS_SYSTEM_HPP__
#define __INTERPRETER_SYSTEMFUNCTIONS_SYSTEM_HPP__

#include "../Interpreter.hpp"


namespace Interpreter::SystemFunctions {

    namespace System {

        class Stream {
        public:

            virtual ~Stream() = default;

        };

        class InputStream : public virtual Stream {
        public:

            virtual int read() = 0;
            virtual std::string scan() = 0;
            virtual bool has() = 0;

        };

        class OutputStream : public virtual Stream {
        public:

            virtual void write(int) = 0;
            virtual void print(std::string const&) = 0;
            virtual void flush() = 0;

        };

        Reference stream_is(FunctionContext& context);
        Reference stream_read(FunctionContext& context);
        Reference stream_scan(FunctionContext& context);
        Reference stream_has(FunctionContext& context);
        Reference stream_write(FunctionContext& context);
        Reference stream_print(FunctionContext& context);
        Reference stream_flush(FunctionContext& context);

        Reference file_open(FunctionContext& context);
        Reference file_close(FunctionContext& context);
        Reference file_get_working_directory(FunctionContext& context);
        Reference file_set_working_directory(FunctionContext& context);
        Reference file_exists(FunctionContext& context);
        Reference file_is_directory(FunctionContext& context);
        Reference file_create_directories(FunctionContext& context);
        Reference file_copy(FunctionContext& context);
        Reference file_delete(FunctionContext& context);

        Reference socket_open(FunctionContext& context);
        Reference socket_close(FunctionContext& context);
        Reference acceptor_open(FunctionContext& context);
        Reference acceptor_accept(FunctionContext& context);
        Reference acceptor_close(FunctionContext& context);

        Reference time(FunctionContext& context);
        Reference clock_system(FunctionContext& context);
        Reference clock_steady(FunctionContext& context);

        class Thread : protected std::thread {
        protected:

            bool joined_detached = false;

        public:
            using std::thread::thread;

            void join() {
                if (!joined_detached) {
                    std::thread::join();
                    joined_detached = true;
                } else {
                    throw FunctionArgumentsError();
                }
            }

            void detach() {
                if (!joined_detached) {
                    std::thread::detach();
                    joined_detached = true;
                } else {
                    throw FunctionArgumentsError();
                }
            }

            OV_INT get_id() const {
                return static_cast<OV_INT>(std::hash<std::thread::id>{}(std::thread::get_id()));
            }

            ~Thread() {
                if (!joined_detached) {
                    std::thread::detach();
                }
            }
        };

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
