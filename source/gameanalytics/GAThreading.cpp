//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#include "GAThreading.h"
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include "GALogger.h"
#include <thread>

namespace gameanalytics
{
    namespace threading
    {
        // static members
        std::shared_ptr<GAThreading::State> GAThreading::state;

        bool GAThreading::isGAThread()
        {
            return true;
        }

        void GAThreading::createStateIfNeeded()
        {
            if (!state)
            {
                state = std::make_shared<State>(GAThreading::thread_routine);
            }
        }

        GAThreading::BlockIdentifier GAThreading::scheduleTimer(double interval, const Block& callback)
        {
            createStateIfNeeded();
            GAThreadHelpers::scoped_lock lock(state->mutex);

            GAThreading::BlockIdentifier blockIdentifier = GAThreading::BlockIdentifier::make();
            state->blocks.push_back({ callback, blockIdentifier, std::chrono::steady_clock::now() + std::chrono::milliseconds(static_cast<int>(1000 * interval)) } );
            std::push_heap(state->blocks.begin(), state->blocks.end());
            return blockIdentifier;
        }

        void GAThreading::ignoreTimer(const GAThreading::BlockIdentifier& blockIdentifier)
        {
            createStateIfNeeded();
            GAThreadHelpers::scoped_lock lock(state->mutex);
            for (auto& block : state->blocks)
            {
                if (block.blockIdentifier == blockIdentifier)
                {
                    block.ignore = true;
                }
            }
        }

        void GAThreading::performTaskOnGAThread(const Block& taskBlock)
        {
            performTaskWithDelayOnGAThread(taskBlock, 0);
        }

        void GAThreading::performTaskWithDelayOnGAThread(const Block& taskBlock, long delayInSeconds)
        {
            createStateIfNeeded();
            GAThreadHelpers::scoped_lock lock(state->mutex);
            state->blocks.push_back({ taskBlock, GAThreading::BlockIdentifier::make(), std::chrono::steady_clock::now() + std::chrono::seconds(delayInSeconds)} );
            std::push_heap(state->blocks.begin(), state->blocks.end());
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
            while(!state)
            {
                // wait for the assignment to be complete
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

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

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            return nullptr;
        }
    }
}

