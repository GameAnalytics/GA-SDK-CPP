//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <mutex>
#include <thread>

namespace
{
    namespace GAThreadHelpers
    {
        typedef void *(*start_routine) (void *);

        typedef std::mutex mutex;

        void mutex_init(mutex& mtx)
        {
            // nothing to do here
        }

        void mutex_lock(mutex& mtx)
        {
            mtx.lock();
        }

        void mutex_unlock(mutex& mtx)
        {
            mtx.unlock();
        }

        typedef std::thread thread;

        void thread_create(thread& t, start_routine f, void* arg)
        {
            t = thread(f, arg);
        }

        void thread_join(thread& t)
        {
            t.join();
        }


        struct scoped_lock
        {
            // TODO(nikolaj): added explicit - remove if not working
            explicit scoped_lock(mutex& mtx) : mutex_(mtx)
            {
                mutex_lock(mutex_);
            }

            ~scoped_lock()
            {
                mutex_unlock(mutex_);
            }
         private:
            mutex& mutex_;
        };

        struct scoped_thread
        {
            // TODO(nikolaj): added explicit - remove if not working
            explicit scoped_thread(start_routine routine)
            {
                thread_create(thread_, routine, nullptr);
            }

            ~scoped_thread()
            {
                thread_join(thread_);
            }

         private:
            thread thread_;
        };
    } // namespace GAThreadHelpers
} // namespace
