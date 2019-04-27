//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//
#if !USE_UWP && !USE_TIZEN
#include "GAUncaughtExceptionHandler.h"
#include "GAState.h"
#include "GAEvents.h"
#include <iostream>
#include <sstream>
#include <string>
#include <stacktrace/call_stack.hpp>
#if defined(_WIN32)
#include <stdlib.h>
#include <tchar.h>
#else
#include <execinfo.h>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#endif

namespace gameanalytics
{
    namespace errorreporter
    {
        std::terminate_handler GAUncaughtExceptionHandler::previousTerminateHandler = NULL;
        int GAUncaughtExceptionHandler::errorCount = 0;
        int GAUncaughtExceptionHandler::MAX_ERROR_TYPE_COUNT = 5;

#if defined(_WIN32)
        void GAUncaughtExceptionHandler::signalHandler(int sig)
        {
            stacktrace::call_stack st;
            std::stringstream ss;

            ss << "Uncaught Signal (" << sig << ")" << std::endl;
            ss << "Stack trace:" << std::endl;
            ss << st.to_string() << std::endl;

            if(errorCount <= MAX_ERROR_TYPE_COUNT)
            {
                errorCount = errorCount + 1;
                events::GAEvents::addErrorEvent(EGAErrorSeverity::Critical, ss.str(), {});
                events::GAEvents::processEvents("error", false);
            }

            std::_Exit( EXIT_FAILURE );
        }

        void GAUncaughtExceptionHandler::setupUncaughtSignals()
        {
            signal(SIGILL, signalHandler);
            signal(SIGABRT, signalHandler);
            signal(SIGFPE, signalHandler);
            signal(SIGSEGV, signalHandler);
        }
#else
        void GAUncaughtExceptionHandler::setupUncaughtSignals()
        {
            struct sigaction mySigAction;
            mySigAction.sa_sigaction = signalHandler;
            mySigAction.sa_flags = SA_SIGINFO;

            sigemptyset(&mySigAction.sa_mask);
            sigaction(SIGQUIT, &mySigAction, NULL);
            sigaction(SIGILL, &mySigAction, NULL);
            sigaction(SIGTRAP, &mySigAction, NULL);
            sigaction(SIGABRT, &mySigAction, NULL);
#if !USE_LINUX
            sigaction(SIGEMT, &mySigAction, NULL);
#endif
            sigaction(SIGFPE, &mySigAction, NULL);
            sigaction(SIGBUS, &mySigAction, NULL);
            sigaction(SIGSEGV, &mySigAction, NULL);
            sigaction(SIGSYS, &mySigAction, NULL);
            sigaction(SIGPIPE, &mySigAction, NULL);
            sigaction(SIGALRM, &mySigAction, NULL);
            sigaction(SIGXCPU, &mySigAction, NULL);
            sigaction(SIGXFSZ, &mySigAction, NULL);
        }

        /*    signalHandler
         *
         *        Set up the uncaught signals
         */
        void GAUncaughtExceptionHandler::signalHandler(int sig, siginfo_t *info, void *context)
        {
            if(!state::GAState::useErrorReporting())
            {
                return;
            }

            void *frames[128];
            int i,len = backtrace(frames, 128);
            char **symbols = backtrace_symbols(frames,len);

            /*
             *    Now format into a message for sending to the user
             */

            std::stringstream ss;
            char buffer[8200] = "";
            strcat(buffer, "Uncaught Signal\n");
            strcat(buffer, "Uncaught Signal\n");

            ss << format("si_signo    %d", info->si_signo) << std::endl;
            ss << format("si_code     %d", info->si_code) << std::endl;
            ss << format("si_value    %d", info->si_value) << std::endl;
            ss << format("si_errno    %d", info->si_errno) << std::endl;
            ss << format("si_addr     0x%08lX", info->si_addr) << std::endl;
            ss << format("si_status   %d", info->si_status) << std::endl;
            ss << "Stack trace:" << std::endl;
            for (i = 0; i < len; ++i)
            {
                ss << format("%4d - %s", i, symbols[i]) << std::endl;
            }

            if(errorCount <= MAX_ERROR_TYPE_COUNT)
            {
                errorCount = errorCount + 1;
                events::GAEvents::addErrorEvent(EGAErrorSeverity::Critical, ss.str().c_str(), {});
                events::GAEvents::processEvents("error", false);
            }

            struct sigaction newact;
            newact.sa_flags = 0;
            sigemptyset(&newact.sa_mask);
            newact.sa_handler= SIG_DFL;

            std::_Exit( EXIT_FAILURE );
        }

        const std::string GAUncaughtExceptionHandler::format(const char* format, ...)
        {
            va_list args;
            va_start (args, format);
            size_t len = std::vsnprintf(NULL, 0, format, args);
            va_end (args);
            std::vector<char> vec(len + 1);
            va_start (args, format);
            std::vsnprintf(&vec[0], len + 1, format, args);
            va_end (args);
            return &vec[0];
        }
#endif

        /*    terminateHandler
         *
         *        C++ exception terminate handler
         */
        void GAUncaughtExceptionHandler::terminateHandler()
        {
            if(!state::GAState::useErrorReporting())
            {
                return;
            }

            stacktrace::call_stack st;

            /*
             *    Now format into a message for sending to the user
             */

            std::stringstream ss;
            ss << "Uncaught C++ Exception" << std::endl;
            ss << "Stack trace:" << std::endl;
            ss << st.to_string() << std::endl;

            if(errorCount <= MAX_ERROR_TYPE_COUNT)
            {
                errorCount = errorCount + 1;
                events::GAEvents::addErrorEvent(EGAErrorSeverity::Critical, ss.str().c_str(), {});
                events::GAEvents::processEvents("error", false);
            }

            if(previousTerminateHandler != NULL)
            {
                previousTerminateHandler();
            }
            else
            {
                std::_Exit( EXIT_FAILURE );
            }
        }

        void GAUncaughtExceptionHandler::setUncaughtExceptionHandlers()
        {
            if(state::GAState::useErrorReporting())
            {
                setupUncaughtSignals();
                previousTerminateHandler = std::set_terminate(terminateHandler);
            }
        }
    }
}
#endif
