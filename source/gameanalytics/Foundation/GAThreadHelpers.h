//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <boost/thread.hpp>


namespace
{
    namespace GAThreadHelpers
    {
        typedef void *(*start_routine) (void *);

        typedef boost::mutex mutex;

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

        typedef boost::thread thread;

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
