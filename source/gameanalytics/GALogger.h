//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <Foundation/GASingleton.h>
#include <memory>
#if !USE_UWP && !USE_TIZEN
#include <spdlog/spdlog.h>
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

        class GALogger : public GASingleton<GALogger>
        {
         public:
            GALogger();

            // set debug enabled (client)
            static void setInfoLog(bool enabled);
            static void setVerboseInfoLog(bool enabled);

            // Debug (w/e always shows, d only shows during SDK development, i shows when client has set debugEnabled to YES)
            static void  w(const char* format);//const char* format, ...);
            static void  e(const char* format);//const char* format, ...);
            static void  d(const char* format);//const char* format, ...);
            static void  i(const char* format);//const char* format, ...);
            static void ii(const char* format);//const char* format, ...);

            void sendNotificationMessage(const char* message, EGALoggerMessageType type);

#if !USE_UWP && !USE_TIZEN
            static void customInitializeLog();
            static void addCustomLogStream(std::ostream& os);
#endif
         private:
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
            std::shared_ptr<spdlog::logger> logger;
            bool logInitialized;
            std::shared_ptr<spdlog::logger> custom_logger;
#endif
        };
    }
}
