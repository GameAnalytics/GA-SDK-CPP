//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#if !USE_UWP && !USE_TIZEN
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
#if defined(_WIN32)
            static void signalHandler(int sig);
#else
            static void signalHandler(int sig, siginfo_t *info, void *context);
            static const std::string format(const std::string& format, ...);
#endif
            static void setupUncaughtSignals();
            static void terminateHandler();

            static std::terminate_handler previousTerminateHandler;
            static int errorCount;
            static int MAX_ERROR_TYPE_COUNT;
        };
    }
}
#endif
