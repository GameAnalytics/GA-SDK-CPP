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
#endif

namespace gameanalytics
{
    namespace threading
    {
        class GAThreading
        {
         public:

            typedef std::function<void()> Block;

#if !USE_TIZEN
            struct BlockIdentifier
            {
                static BlockIdentifier make()
                {
                    static unsigned long counter = 1;
                    BlockIdentifier result = none();
                    result.id = counter++;
                    return result;
                }

                static BlockIdentifier none()
                {
                    return BlockIdentifier();
                }

                bool isValid() const { return id != 0; }
                bool operator == (const BlockIdentifier& other)
                {
                    return id == other.id;
                }
             private:
                unsigned long id = 0;
            };
#endif

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

                TimedBlock(const Block& block, const BlockIdentifier& blockIdentifier, const time_point& deadline)
                    :block(block), blockIdentifier(blockIdentifier), deadline(deadline), ignore(false) {}

                Block block;
                BlockIdentifier blockIdentifier;
                time_point deadline;
                bool ignore;

                bool operator < (const TimedBlock& o) const
                {
                    // the priority_queue orders the items descending. but we want to retrieve the items in ascending order.
                    return deadline > o.deadline;
                }
            };

            typedef std::vector<TimedBlock> TimedBlocks;

            struct State
            {
                State(GAThreadHelpers::start_routine routine) : thread(routine)
                {
                    GAThreadHelpers::mutex_init(mutex);
                }

                TimedBlocks blocks;
                GAThreadHelpers::mutex mutex;

                GAThreadHelpers::scoped_thread thread;
                bool endThread;
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
