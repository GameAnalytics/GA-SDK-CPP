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
        std::atomic<bool> GAThreading::initialized(false);
        std::atomic<bool> GAThreading::_endThread(false);

        void GAThreading::initIfNeeded()
        {
            if (!initialized)
            {
                initialized = true;
                state = std::make_shared<State>(GAThreading::thread_routine);
            }
        }

        void GAThreading::scheduleTimer(double interval, const Block& callback)
        {
            if(_endThread)
            {
                return;
            }
            initIfNeeded();
            std::lock_guard<std::mutex> lock(state->mutex);

            state->blocks.push_back({ callback, std::chrono::steady_clock::now() + std::chrono::milliseconds(static_cast<int>(1000 * interval)) } );
            std::push_heap(state->blocks.begin(), state->blocks.end());
        }

        void GAThreading::performTaskOnGAThread(const Block& taskBlock)
        {
            if(_endThread)
            {
                return;
            }
            initIfNeeded();
            std::lock_guard<std::mutex> lock(state->mutex);
            state->blocks.push_back({ taskBlock, std::chrono::steady_clock::now()} );
            std::push_heap(state->blocks.begin(), state->blocks.end());
        }

        void GAThreading::endThread()
        {
            _endThread = true;
        }

        bool GAThreading::getNextBlock(TimedBlock& timedBlock)
        {
            std::lock_guard<std::mutex> lock(state->mutex);

            if((!state->blocks.empty() && state->blocks.front().deadline <= std::chrono::steady_clock::now()))
            {
                timedBlock = state->blocks.front();
                std::pop_heap(state->blocks.begin(), state->blocks.end());
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
                        timedBlock.block();
                        // clear the block, so that the assert works
                        timedBlock.block = {};
                    }

                    if(_endThread)
                    {
                        break;
                    }

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                logging::GALogger::d("thread_routine stopped");
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
