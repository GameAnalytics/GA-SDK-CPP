//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <Foundation/GASingleton.h>
#include <string>

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
#if !USE_UWP
            static void addFileLog(const std::string& path);
#endif

            // Debug (w/e always shows, d only shows during SDK development, i shows when client has set debugEnabled to YES)
            static void  w(const std::string& format);//const char* format, ...);
            static void  e(const std::string& format);//const char* format, ...);
            static void  d(const std::string& format);//const char* format, ...);
            static void  i(const std::string& format);//const char* format, ...);
            static void ii(const std::string& format);//const char* format, ...);

            void sendNotificationMessage(const std::string& message, EGALoggerMessageType type);

         private:
            // Settings
            bool infoLogEnabled;
            bool infoLogVerboseEnabled;
            bool debugEnabled;
            static const std::string tag;
#if USE_UWP
            static void LogMessageToConsole(Platform::Object^ parameter);
            Windows::Storage::StorageFile^ file;
#endif
        };
    }
}
