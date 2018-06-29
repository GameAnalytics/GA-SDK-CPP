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
    } // namespace GAThreadHelpers
} // namespace
