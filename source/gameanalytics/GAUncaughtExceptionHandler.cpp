//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//
#if !USE_UWP && !USE_TIZEN
#include "GAUncaughtExceptionHandler.h"
#include "GAState.h"
#include "GAEvents.h"
#include <stacktrace/call_stack.hpp>
#include <string.h>
#include <stdio.h>
#include <cstdarg>
#if defined(_WIN32)
#include <stdlib.h>
#include <tchar.h>
#else
#include <execinfo.h>
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
            size_t totalSize = 0;
            totalSize += formatSize("Uncaught Signal (%d)\n", sig);
            totalSize += strlen("Stack trace:\n");

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
            size_t totalSize = 0;
            totalSize += strlen("Uncaught Signal\n");
            totalSize += formatSize("si_signo    %d\n", info->si_signo);
            totalSize += formatSize("si_code     %d\n", info->si_code);
            totalSize += formatSize("si_value    %d\n", info->si_value);
            totalSize += formatSize("si_errno    %d\n", info->si_errno);
            totalSize += formatSize("si_addr     0x%08lX\n", info->si_addr);
            totalSize += formatSize("si_status   %d\n", info->si_status);
            totalSize += strlen("Stack trace:\n");
            for (i = 0; i < len; ++i)
            {
                totalSize += formatSize("%4d - %s\n", i, symbols[i]);
            }

            char buffer[totalSize + 1];
            buffer[0] = 0;
            strcat(buffer, "Uncaught Signal\n");

            formatConcat(buffer, "si_signo    %d\n", info->si_signo);
            formatConcat(buffer, "si_code     %d\n", info->si_code);
            formatConcat(buffer, "si_value    %d\n", info->si_value);
            formatConcat(buffer, "si_errno    %d\n", info->si_errno);
            formatConcat(buffer, "si_addr     0x%08lX\n", info->si_addr);
            formatConcat(buffer, "si_status   %d\n", info->si_status);
            strcat(buffer, "Stack trace:\n");
            for (i = 0; i < len; ++i)
            {
                formatConcat(buffer, "%4d - %s\n", i, symbols[i]);
            }

            if(errorCount <= MAX_ERROR_TYPE_COUNT)
            {
                errorCount = errorCount + 1;
                events::GAEvents::addErrorEvent(EGAErrorSeverity::Critical, buffer, {});
                events::GAEvents::processEvents("error", false);
            }

            struct sigaction newact;
            newact.sa_flags = 0;
            sigemptyset(&newact.sa_mask);
            newact.sa_handler= SIG_DFL;

            std::_Exit( EXIT_FAILURE );
        }
#endif
        void GAUncaughtExceptionHandler::formatConcat(char* buffer, const char* format, ...)
        {
            va_list args;
            va_start (args, format);
            size_t len = std::vsnprintf(NULL, 0, format, args);
            va_end (args);
            char formatted[len + 1];
            va_start (args, format);
            std::vsnprintf(formatted, len + 1, format, args);
            va_end (args);
            strcat(buffer, formatted);
        }

        size_t GAUncaughtExceptionHandler::formatSize(const char* format, ...)
        {
            va_list args;
            va_start (args, format);
            size_t len = std::vsnprintf(NULL, 0, format, args);
            va_end (args);
            return len;
        }

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

            size_t totalSize = 0;
            totalSize += strlen("Uncaught C++ Exception\n");
            totalSize += strlen("Stack trace:\n");
            totalSize += st.to_string_size() + strlen("\n");
            char buffer[totalSize + 1];

            strcat(buffer, "Uncaught C++ Exception\n");
            strcat(buffer, "Stack trace:\n");
            strcat(buffer, "Stack trace:\n");
            st.to_string(buffer);
            strcat(buffer, "\n");

            if(errorCount <= MAX_ERROR_TYPE_COUNT)
            {
                errorCount = errorCount + 1;
                events::GAEvents::addErrorEvent(EGAErrorSeverity::Critical, buffer, {});
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
