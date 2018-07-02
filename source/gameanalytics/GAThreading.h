//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once


#include <functional>
#if USE_TIZEN
#include <Ecore.h>
#else
#include <Foundation/GAThreadHelpers.h>
#include <vector>
#include <chrono>
#include <memory>
#include <future>
#include "GALogger.h"
#endif

namespace gameanalytics
{
    namespace threading
    {
        class GAThreading
        {
         public:

            typedef std::function<void()> Block;

            static void performTaskOnGAThread(const Block& taskBlock);

            // timers
            static void scheduleTimer(double interval, const Block& callback);

            static void endThread();

         private:

#if USE_TIZEN
            static bool initialized;

            struct BlockHolder
            {
                BlockHolder() {}

                BlockHolder(const Block& block) :block(block) {}

                Block block;
            };

            static Eina_Bool _scheduled_function(void* data);
            static void _perform_task_function(void* data, Ecore_Thread* thread);
            static void _end_function(void* data, Ecore_Thread* thread);
#else
            //timers
            struct TimedBlock
            {
                typedef std::chrono::steady_clock::time_point time_point;

                TimedBlock() {}

                TimedBlock(const Block& block, const time_point& deadline) :block(block), deadline(deadline) {}

                Block block;
                time_point deadline;

                bool operator < (const TimedBlock& o) const
                {
                    // the priority_queue orders the items descending. but we want to retrieve the items in ascending order.
                    return deadline > o.deadline;
                }
            };

            typedef std::vector<TimedBlock> TimedBlocks;

            struct State
            {
                State(GAThreadHelpers::start_routine routine)
                {
                    GAThreadHelpers::mutex_init(mutex);
                    handle = std::async(std::launch::async, routine, nullptr);
                }

                ~State()
                {
                    endThread = true;
                }

                TimedBlocks blocks;
                GAThreadHelpers::mutex mutex;

                std::future<void*> handle;
                bool endThread = false;
            };

            static std::shared_ptr<State> state;

            //< The function that's running in the gaThread
            static void* thread_routine(void*);
            /*!
            retrieves the next block to execute.
            This will either be a regular Block or a Timed Block.
            return true, if a Block is retrieved, false if a TimedBlock is retrieved.
            */
            static bool getNextBlock(TimedBlock& timedBlock);
#endif

            static void initIfNeeded();
        };
    }
}
