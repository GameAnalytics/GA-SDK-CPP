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
        enum GALoggerEventType
        {
            BusyStatusChanged,
            LogFileGenerated,
            LogFileGeneratedAtDisable,
            LogFileGeneratedAtSuspend,
            LoggingEnabledDisabled
        };

        ref class GALoggerEventArgs sealed
        {
        private:

            GALoggerEventType _type;
            std::wstring _logFileFullPath;
            bool _enabled;

        internal:

            GALoggerEventArgs(GALoggerEventType type)
            {
                _type = type;
            }

            GALoggerEventArgs(GALoggerEventType type, const std::wstring& logFileFullPath)
            {
                _type = type;
                _logFileFullPath = logFileFullPath;
            }

            GALoggerEventArgs(bool enabled)
            {
                _type = LoggingEnabledDisabled;
                _enabled = enabled;
            }

            property GALoggerEventType Type
            {
                GALoggerEventType get()
                {
                    return _type;
                }
            }

            property std::wstring LogFileFullPath
            {
                std::wstring get()
                {
                    return _logFileFullPath;
                }
            }

            property bool Enabled
            {
                bool get()
                {
                    return _enabled;
                }
            }
        };

        delegate void StatusChangedHandler(Platform::Object^ sender, GALoggerEventArgs^ args);

        ref class GALoggerUWP sealed
        {
        internal:

            concurrency::task<void> PrepareToSuspendAsync();
            void ResumeLoggingIfApplicable();
            void StartLogging();
            event StatusChangedHandler^ StatusChanged;

            static property GALoggerUWP^ Instance
            {
                GALoggerUWP^ get()
                {
                    if (_instance == nullptr)
                    {
                        _instance = ref new GALoggerUWP();
                    }
                    return _instance;
                }
            }

            property Windows::Foundation::Diagnostics::LoggingChannel^ Channel
            {
                Windows::Foundation::Diagnostics::LoggingChannel^ get()
                {
                    return channel;
                }
            }

            void Touch();

        private:
            GALoggerUWP();
            ~GALoggerUWP();

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
            long logFileGeneratedCount;
            static GALoggerUWP^ _instance;

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

            static Platform::String^ GetAppLocalSettingsValueAsString(Platform::String^ key)
            {
                Platform::Object^ obj = GetAppLocalSettingsValueAsObject(key);
                return safe_cast<Platform::String^>(obj);
            }

            inline std::wstring GetTimeStamp()
            {
                std::wstring result;
                SYSTEMTIME timeNow;
                GetLocalTime(&timeNow);
                result.resize(16);
                swprintf_s(
                    &result[0],
                    result.size() + 1,
                    L"%02u%02u%02u-%02u%02u%02u%03u",
                    timeNow.wYear - 2000,
                    timeNow.wMonth,
                    timeNow.wDay,
                    timeNow.wHour,
                    timeNow.wMinute,
                    timeNow.wSecond,
                    timeNow.wMilliseconds);
                return result;
            }
        };

#endif
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
#endif
        };
    }
}
