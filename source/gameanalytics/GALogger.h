//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <memory>
#include <cstdio>
#if !USE_UWP && !USE_TIZEN
#define ZF_LOG_SRCLOC ZF_LOG_SRCLOC_NONE
#include "zf_log.h"
#endif

namespace gameanalytics
{
    namespace logging
    {
        enum EGALoggerMessageType
        {
            Error = 0,
            Warning = 1,
            Info = 2,
            Debug = 3
        };

        class GALogger
        {
         public:
            // set debug enabled (client)
            static void setInfoLog(bool enabled);
            static void setVerboseInfoLog(bool enabled);

            // Debug (w/e always shows, d only shows during SDK development, i shows when client has set debugEnabled to YES)
            static void  w(const char* format, ...);
            static void  e(const char* format, ...);
            static void  d(const char* format, ...);
            static void  i(const char* format, ...);
            static void ii(const char* format, ...);

#if !USE_UWP && !USE_TIZEN
            static void customInitializeLog();
#endif
        private:
            GALogger();
            ~GALogger();

            void sendNotificationMessage(const char* message, EGALoggerMessageType type);

            static bool _destroyed;
            static GALogger* _instance;
            static void cleanUp();
            static GALogger* getInstance();

#if !USE_UWP && !USE_TIZEN
            static void initializeLog();
#endif
            // Settings
            bool infoLogEnabled;
            bool infoLogVerboseEnabled;
            bool debugEnabled;
            static const char* tag;
#if USE_UWP
            static void LogMessageToConsole(Platform::Object^ parameter);
            Windows::Storage::StorageFile^ file;
#endif
#if !USE_UWP && !USE_TIZEN
            static void file_output_callback(const zf_log_message *msg, void *arg);
            bool logInitialized;
            FILE *log_file;
#endif
        };
    }
}
