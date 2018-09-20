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
        std::atomic<bool> GAThreading::_endThread(false);
        std::unique_ptr<GAThreading::State> GAThreading::state(new GAThreading::State(GAThreading::thread_routine, GAThreading::_endThread));

        void GAThreading::scheduleTimer(double interval, const Block& callback)
        {
            if(_endThread)
            {
                return;
            }
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
            std::lock_guard<std::mutex> lock(state->mutex);
            state->blocks.push_back({ taskBlock, std::chrono::steady_clock::now()} );
            std::push_heap(state->blocks.begin(), state->blocks.end());
        }

        void GAThreading::endThread()
        {
            logging::GALogger::d("endThread now");
            _endThread = true;
        }

        bool GAThreading::getNextBlock(TimedBlock& timedBlock)
        {
            std::lock_guard<std::mutex> lock(state->mutex);

            if((!state->blocks.empty() && state->blocks.front().deadline <= std::chrono::steady_clock::now()))
            {
                timedBlock = state->blocks.front();
                std::pop_heap(state->blocks.begin(), state->blocks.end());
                state->blocks.pop_back();
                return true;
            }

            return false;
        }

        void GAThreading::thread_routine(std::atomic<bool>& endThread)
        {
            logging::GALogger::d("thread_routine start");

            try
            {
                while (!endThread && state)
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

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                if(!endThread)
                {
                    logging::GALogger::d("thread_routine stopped");
                }
            }
            catch(const std::exception& e)
            {
                if(!endThread)
                {
                    logging::GALogger::e("Error on GA thread");
                    logging::GALogger::e(e.what());
                }
            }
        }
    }
}
