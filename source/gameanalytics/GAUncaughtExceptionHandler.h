//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <exception>
#include <string>

namespace gameanalytics
{
    namespace errorreporter
    {
        class GAUncaughtExceptionHandler
        {
        public:
            static void setUncaughtExceptionHandlers();
        private:
            static void setupUncaughtSignals();
            static void signalHandler(int sig, siginfo_t *info, void *context);
            static void terminateHandler();
            static const std::string format(const std::string& format, ...);

            static std::terminate_handler previousTerminateHandler;
            static int errorCount;
            static int MAX_ERROR_TYPE_COUNT;
        };
    }
}
