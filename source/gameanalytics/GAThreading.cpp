//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#include "GAThreading.h"
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include "GALogger.h"
#include <thread>
#include <exception>

namespace gameanalytics
{
    namespace threading
    {
        // static members
        std::shared_ptr<GAThreading::State> GAThreading::state;

        void GAThreading::initIfNeeded()
        {
            if (!state)
            {
                state = std::make_shared<State>(GAThreading::thread_routine);
            }
        }

        void GAThreading::scheduleTimer(double interval, const Block& callback)
        {
            initIfNeeded();
            GAThreadHelpers::scoped_lock lock(state->mutex);

            GAThreading::BlockIdentifier blockIdentifier = GAThreading::BlockIdentifier::make();
            state->blocks.push_back({ callback, blockIdentifier, std::chrono::steady_clock::now() + std::chrono::milliseconds(static_cast<int>(1000 * interval)) } );
            std::push_heap(state->blocks.begin(), state->blocks.end());
        }

        void GAThreading::performTaskOnGAThread(const Block& taskBlock)
        {
            initIfNeeded();
            GAThreadHelpers::scoped_lock lock(state->mutex);
            state->blocks.push_back({ taskBlock, GAThreading::BlockIdentifier::make(), std::chrono::steady_clock::now()} );
            std::push_heap(state->blocks.begin(), state->blocks.end());
        }

        void GAThreading::endThread()
        {
            logging::GALogger::d("ending thread");
            GAThreadHelpers::scoped_lock lock(state->mutex);
            state->endThread = true;
        }

        bool GAThreading::getNextBlock(TimedBlock& timedBlock)
        {
            GAThreadHelpers::scoped_lock lock(state->mutex);

            if((!state->blocks.empty() && state->blocks.front().deadline <= std::chrono::steady_clock::now()))
            {
                timedBlock = state->blocks.front();
                std::pop_heap(state->blocks.begin(), state->blocks.end());
                state->blocks.pop_back();
                return true;
            }

            return false;
        }

        void* GAThreading::thread_routine(void*)
        {
            logging::GALogger::d("thread_routine start");
            while(!state)
            {
                // wait for the assignment to be complete
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            try
            {
                while (std::shared_ptr<State> state = GAThreading::state)
                {
                    TimedBlock timedBlock;

                    while (getNextBlock(timedBlock))
                    {
                        assert(timedBlock.block);
                        assert(timedBlock.deadline <= std::chrono::steady_clock::now());
                        if (!timedBlock.ignore)
                        {
                            timedBlock.block();
                        }
                        // clear the block, so that the assert works
                        timedBlock.block = {};
                    }

                    if(state->endThread)
                    {
                        break;
                    }

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
            catch(const std::exception& e)
            {
                logging::GALogger::e("Error on GA thread");
                logging::GALogger::e(e.what());
            }

            return nullptr;
        }
    }
}
