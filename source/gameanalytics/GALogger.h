//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <Foundation/GASingleton.h>
#include <string>
#include <ppltasks.h>

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

        // notification id-string for communicating
        //extern NSString *k_GA_LoggerMessageNotification;

#if USE_UWP
        ref class GALogger sealed
#else
        class GALogger : public GASingleton<GALogger>
#endif
        {
         public:
            GALogger();

            // set debug enabled (client)
            static void setInfoLog(bool enabled);
            static void setVerboseInfoLog(bool enabled);
            static void addFileLog(const std::string& path);

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
            void OnChannelLoggingEnabled(Windows::Foundation::Diagnostics::ILoggingChannel ^sender, Platform::Object ^args);
            void OnAppSuspending(Platform::Object ^sender, Windows::ApplicationModel::SuspendingEventArgs ^e);
            void OnAppResuming(Platform::Object ^sender, Platform::Object ^args);
            concurrency::task<Windows::Storage::StorageFile^> CloseSessionSaveFinalLogFile();
            void OnLogFileGenerated(Windows::Foundation::Diagnostics::IFileLoggingSession^ sender, Windows::Foundation::Diagnostics::LogFileGeneratedEventArgs^ args);

            Windows::Foundation::Diagnostics::LoggingChannel^ channel;
            bool isPreparingForSuspend;
            Windows::Foundation::Diagnostics::FileLoggingSession^ session;
            bool isChannelEnabled;
            Windows::Foundation::Diagnostics::LoggingLevel channelLoggingLevel;

            static GALogger^ instance;

            static GALogger^ sharedInstance()
            {
                if (instance == nullptr)
                {
                    instance = ref new GALogger();
                }
                return instance;
            }

            static void SetAppLocalSettingsValue(Platform::String^ key, Platform::Object^ value)
            {
                if (Windows::Storage::ApplicationData::Current->LocalSettings->Values->HasKey(key))
                {
                    Windows::Storage::ApplicationData::Current->LocalSettings->Values->Remove(key);
                }
                Windows::Storage::ApplicationData::Current->LocalSettings->Values->Insert(key, value);
            }

            static bool IsAppLocalSettingsValue(Platform::String^ key)
            {
                return Windows::Storage::ApplicationData::Current->LocalSettings->Values->HasKey(key);
            }

            static Platform::Object^ GetAppLocalSettingsValueAsObject(Platform::String^ key)
            {
                return Windows::Storage::ApplicationData::Current->LocalSettings->Values->Lookup(key);
            }

            static bool GetAppLocalSettingsValueAsBool(Platform::String^ key)
            {
                Platform::Object^ obj = GetAppLocalSettingsValueAsObject(key);
                return safe_cast<bool>(obj);
            }

        internal:
            concurrency::task<void> PrepareToSuspendAsync();
            void ResumeLoggingIfApplicable();
            void StartLogging();
#endif
        };
    }
}
