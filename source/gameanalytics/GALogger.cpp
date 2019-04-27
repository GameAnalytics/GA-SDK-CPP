#include "GALogger.h"
#include "GameAnalytics.h"
#include <iostream>
#include "GADevice.h"
#include <cstdio>
#if USE_UWP
#include "GAUtilities.h"
#include <collection.h>
#include <ppltasks.h>
#elif USE_TIZEN
#include <dlog.h>
#else
#include "GAUtilities.h"
#include <vector>
#include <spdlog/sinks/ostream_sink.h>
#endif

#if USE_UWP
#define GALOGGER_PREFIX                     L"GALogger_"
#define DEFAULT_SESSION_NAME                GALOGGER_PREFIX L"Session"
#define DEFAULT_CHANNEL_NAME                GALOGGER_PREFIX L"Channel"
#define OUR_SAMPLE_APP_LOG_FILE_FOLDER_NAME GALOGGER_PREFIX L"LogFiles"
#define LOGGING_ENABLED_SETTING_KEY_NAME    GALOGGER_PREFIX L"LoggingEnabled"
#define LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME GALOGGER_PREFIX L"LogFileGeneratedBeforeSuspend"
#endif

namespace gameanalytics
{
    namespace logging
    {
        const char* GALogger::tag = "GameAnalytics";

        GALogger::GALogger()
        {
            infoLogEnabled = false;


#if defined(_DEBUG)
            // log debug is in dev mode
            debugEnabled = true;
#else
            debugEnabled = false;
#endif

#if USE_UWP
            Windows::Storage::StorageFolder^ gaFolder = concurrency::create_task(Windows::Storage::ApplicationData::Current->LocalFolder->CreateFolderAsync("GameAnalytics", Windows::Storage::CreationCollisionOption::OpenIfExists)).get();
            file = concurrency::create_task(gaFolder->CreateFileAsync("ga_log.txt", Windows::Storage::CreationCollisionOption::ReplaceExisting)).get();
#endif

#if !USE_UWP && !USE_TIZEN
            logInitialized = false;
#endif
        }

        void GALogger::setInfoLog(bool enabled)
        {
            GALogger::sharedInstance()->infoLogEnabled = enabled;
        }

        void GALogger::setVerboseInfoLog(bool enabled)
        {
            GALogger::sharedInstance()->infoLogVerboseEnabled = enabled;
        }

#if !USE_UWP && !USE_TIZEN
        void GALogger::initializeLog()
        {
            GALogger *ga = GALogger::sharedInstance();

            if(!ga->logInitialized)
            {
                std::string p(device::GADevice::getWritablePath() + std::string(utilities::GAUtilities::getPathSeparator()) + "ga_log.txt");
                std::vector<spdlog::sink_ptr> sinks;
                sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
                sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>(p, 1048576 * 5, 3));
                ga->logger = std::make_shared<spdlog::logger>("gameanalytics", begin(sinks), end(sinks));

                if(ga->debugEnabled)
                {
                    ga->logger->flush_on(spdlog::level::debug);
                    spdlog::set_level(spdlog::level::debug);
                }
                else
                {
                    ga->logger->flush_on(spdlog::level::info);
                    spdlog::set_level(spdlog::level::info);
                }

                spdlog::register_logger(ga->logger);

                ga->logInitialized = true;

                GALogger::i("Log file added under: %s", device::GADevice::getWritablePath());
            }
        }

        void GALogger::customInitializeLog()
        {
            GALogger *ga = GALogger::sharedInstance();

            spdlog::drop("gameanalytics");
            std::string p(device::GADevice::getWritablePath() + std::string(utilities::GAUtilities::getPathSeparator()) + "ga_log.txt");
            std::vector<spdlog::sink_ptr> sinks;
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
            sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_st>(p, 1048576 * 5, 3));
            ga->logger = std::make_shared<spdlog::logger>("gameanalytics", begin(sinks), end(sinks));

            if(ga->debugEnabled)
            {
                ga->logger->flush_on(spdlog::level::debug);
                spdlog::set_level(spdlog::level::debug);
            }
            else
            {
                ga->logger->flush_on(spdlog::level::info);
                spdlog::set_level(spdlog::level::info);
            }

            spdlog::register_logger(ga->logger);

            ga->logInitialized = true;

            GALogger::i("Log file added under: %s", device::GADevice::getWritablePath());
        }

        void GALogger::addCustomLogStream(std::ostream& os)
        {
            spdlog::drop("gameanalytics_stream");
            GALogger *ga = GALogger::sharedInstance();
            auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_st>(os);
            ga->custom_logger = std::make_shared<spdlog::logger>("gameanalytics_stream", ostream_sink);

            ga->custom_logger->flush_on(spdlog::level::info);
            spdlog::set_level(spdlog::level::info);

            spdlog::register_logger(ga->custom_logger);
        }
#endif

        // i: information logging
        //
        // used for:
        // - non-errors
        // - initializing, adding event, sending events etc.
        // - generally small text
        void GALogger::i(const char* format, ...)
        {
            GALogger *ga = GALogger::sharedInstance();

            if (!ga->infoLogEnabled) {
                // No logging of info unless in client debug mode
                return;
            }

            va_list args;
            va_start (args, format);
            size_t len = std::vsnprintf(NULL, 0, format, args);
            va_end (args);
            char formatted[len + 1];
            va_start (args, format);
            std::vsnprintf(formatted, len + 1, format, args);
            va_end (args);

            size_t s = len + 1 + 11 + strlen(ga->tag);
            char message[s];
            snprintf(message, s, "Info/%s: %s", ga->tag, formatted);
            ga->sendNotificationMessage(message, Info);
        }


        // w: warning logging (ALWAYS show)
        //
        // used for:
        // - validation errors
        // - trying to initialize with wrong keys
        // - other non-critical
        void GALogger::w(const char* format, ...)
        {
            GALogger *ga = GALogger::sharedInstance();

            va_list args;
            va_start (args, format);
            size_t len = std::vsnprintf(NULL, 0, format, args);
            va_end (args);
            char formatted[len + 1];
            va_start (args, format);
            std::vsnprintf(formatted, len + 1, format, args);
            va_end (args);

            size_t s = len + 1 + 14 + strlen(ga->tag);
            char message[s];
            snprintf(message, s, "Warning/%s: %s", ga->tag, formatted);
            ga->sendNotificationMessage(message, Warning);
        }


        // e: error logging (ALWAYS show)
        //
        // used for:
        // - breaking app behaviour
        // - JSON decoding/encoding errors
        // - unexpected exceptions
        // - errors that never should happen
        void GALogger::e(const char* format, ...)
        {
            GALogger *ga = GALogger::sharedInstance();

            va_list args;
            va_start (args, format);
            size_t len = std::vsnprintf(NULL, 0, format, args);
            va_end (args);
            char formatted[len + 1];
            va_start (args, format);
            std::vsnprintf(formatted, len + 1, format, args);
            va_end (args);

            size_t s = len + 1 + 12 + strlen(ga->tag);
            char message[s];
            snprintf(message, s, "Error/%s: %s", ga->tag, formatted);
            ga->sendNotificationMessage(message, Error);
        }


        // d: debug logging (show when developing)
        //
        // used for:
        // - development only
        // - use large debug text like HTTP payload etc.
        void GALogger::d(const char* format, ...)
        {
            GALogger *ga = GALogger::sharedInstance();

            if (!ga->debugEnabled) {
                // No logging of debug unless in full debug logging mode
                return;
            }

            va_list args;
            va_start (args, format);
            size_t len = std::vsnprintf(NULL, 0, format, args);
            va_end (args);
            char formatted[len + 1];
            va_start (args, format);
            std::vsnprintf(formatted, len + 1, format, args);
            va_end (args);

            int s = len + 1 + 12 + strlen(ga->tag);
            char message[s];
            snprintf(message, s, "Debug/%s: %s", ga->tag, formatted);
            ga->sendNotificationMessage(message, Debug);
        }


        // ii: Advanced information logging
        //
        // used for:
        // - Large logs
        void GALogger::ii(const char* format, ...)
        {
            GALogger *ga = GALogger::sharedInstance();

            if (!ga->infoLogVerboseEnabled) {
                // No logging of info unless in client debug mode
                return;
            }

            va_list args;
            va_start (args, format);
            size_t len = std::vsnprintf(NULL, 0, format, args);
            va_end (args);
            char formatted[len + 1];
            va_start (args, format);
            std::vsnprintf(formatted, len + 1, format, args);
            va_end (args);

            int s = len + 1 + 14 + strlen(ga->tag);
            char message[s];
            snprintf(message, s, "Verbose/%s: %s", ga->tag, formatted);
            ga->sendNotificationMessage(message, Info);
        }

        void GALogger::sendNotificationMessage(const char* message, EGALoggerMessageType type)
        {
            if(GameAnalytics::isThreadEnding())
            {
                //return;
            }
#if USE_UWP
            auto m = ref new Platform::String(utilities::GAUtilities::s2ws(message).c_str());
            Platform::Collections::Vector<Platform::String^>^ lines = ref new Platform::Collections::Vector<Platform::String^>();
            lines->Append(m);
#endif

#if !USE_UWP && !USE_TIZEN
            if(!logInitialized)
            {
                initializeLog();
            }
#endif
            switch(type)
            {
                case Error:
#if USE_UWP
                    try
                    {
                        concurrency::create_task(Windows::Storage::FileIO::AppendLinesAsync(file, lines)).wait();
                    }
                    catch (const std::exception&)
                    {
                    }
                    LogMessageToConsole(m);
#elif USE_TIZEN
                    dlog_print(DLOG_ERROR, GALogger::tag, message);
#else
                    try
                    {
                        logger->error(message);
                        if(custom_logger)
                        {
                            custom_logger->error(message);
                        }
                    }
                    catch (const std::exception&)
                    {
                    }
#endif
                    break;

                case Warning:
#if USE_UWP
                    try
                    {
                        concurrency::create_task(Windows::Storage::FileIO::AppendLinesAsync(file, lines)).wait();
                    }
                    catch (const std::exception&)
                    {
                    }
                    LogMessageToConsole(m);
#elif USE_TIZEN
                    dlog_print(DLOG_WARN, GALogger::tag, message);
#else
                    try
                    {
                        logger->warn(message);
                        if(custom_logger)
                        {
                            custom_logger->warn(message);
                        }
                    }
                    catch (const std::exception&)
                    {
                    }
#endif
                    break;

                case Debug:
#if USE_UWP
                    try
                    {
                        concurrency::create_task(Windows::Storage::FileIO::AppendLinesAsync(file, lines)).wait();
                    }
                    catch (const std::exception&)
                    {
                    }
                    LogMessageToConsole(m);
#elif USE_TIZEN
                    dlog_print(DLOG_DEBUG, GALogger::tag, message);
#else
                    try
                    {
                        logger->info(message);
                        if(custom_logger)
                        {
                            custom_logger->info(message);
                        }
                    }
                    catch (const std::exception&)
                    {
                    }
#endif
                    break;

                case Info:
#if USE_UWP
                    try
                    {
                        concurrency::create_task(Windows::Storage::FileIO::AppendLinesAsync(file, lines)).wait();
                    }
                    catch (const std::exception&)
                    {
                    }
                    LogMessageToConsole(m);
#elif USE_TIZEN
                    dlog_print(DLOG_INFO, GALogger::tag, message);
#else
                    try
                    {
                        logger->info(message);
                        if(custom_logger)
                        {
                            custom_logger->info(message);
                        }
                    }
                    catch (const std::exception&)
                    {
                    }
#endif
                    break;
            }
        }

#if USE_UWP
        void GALogger::LogMessageToConsole(Platform::Object^ parameter)
        {
            auto paraString = parameter->ToString();
            auto formattedText = std::wstring(paraString->Data()).append(L"\r\n");
            OutputDebugString(formattedText.c_str());
        }
#endif
    }
}
