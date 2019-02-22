#include "GALogger.h"
#include "GameAnalytics.h"
#include <iostream>
#include "GADevice.h"
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
        const std::string GALogger::tag = "GameAnalytics";

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
                std::string p(device::GADevice::getWritablePath() + utilities::GAUtilities::getPathSeparator() + "ga_log.txt");
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

                GALogger::i("Log file added under: " + device::GADevice::getWritablePath());
            }
        }

        void GALogger::customInitializeLog()
        {
            GALogger *ga = GALogger::sharedInstance();

            spdlog::drop("gameanalytics");
            std::string p(device::GADevice::getWritablePath() + utilities::GAUtilities::getPathSeparator() + "ga_log.txt");
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

            GALogger::i("Log file added under: " + device::GADevice::getWritablePath());
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
        void GALogger::i(const std::string& format)
        {
            GALogger *ga = GALogger::sharedInstance();

            if (!ga->infoLogEnabled) {
                // No logging of info unless in client debug mode
                return;
            }

            std::string message = "Info/" + ga->tag + ": " + format;
            ga->sendNotificationMessage(message, Info);
        }


        // w: warning logging (ALWAYS show)
        //
        // used for:
        // - validation errors
        // - trying to initialize with wrong keys
        // - other non-critical
        void GALogger::w(const std::string& format)
        {
            GALogger *ga = GALogger::sharedInstance();

            std::string message = "Warning/" + ga->tag + ": " + format;
            ga->sendNotificationMessage(message, Warning);
        }


        // e: error logging (ALWAYS show)
        //
        // used for:
        // - breaking app behaviour
        // - JSON decoding/encoding errors
        // - unexpected exceptions
        // - errors that never should happen
        void GALogger::e(const std::string& format)
        {
            GALogger *ga = GALogger::sharedInstance();

            std::string message = "Error/" + ga->tag + ": " + format;
            ga->sendNotificationMessage(message, Error);
        }


        // d: debug logging (show when developing)
        //
        // used for:
        // - development only
        // - use large debug text like HTTP payload etc.
        void GALogger::d(const std::string& format)
        {
            GALogger *ga = GALogger::sharedInstance();

            if (!ga->debugEnabled) {
                // No logging of debug unless in full debug logging mode
                return;
            }

            std::string message = "Debug/" + ga->tag + ": " + format;
            ga->sendNotificationMessage(message, Debug);
        }


        // ii: Advanced information logging
        //
        // used for:
        // - Large logs
        void GALogger::ii(const std::string& format)
        {
            GALogger *ga = GALogger::sharedInstance();

            if (!ga->infoLogVerboseEnabled) {
                // No logging of info unless in client debug mode
                return;
            }

            std::string message = "Verbose/" + ga->tag + ": " + format;
            ga->sendNotificationMessage(message, Info);
        }

        void GALogger::sendNotificationMessage(const std::string& message, EGALoggerMessageType type)
        {
            if(GameAnalytics::isThreadEnding())
            {
                return;
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
                    dlog_print(DLOG_ERROR, GALogger::tag.c_str(), message.c_str());
#else
                    try
                    {
                        logger->error(message.c_str());
                        if(custom_logger)
                        {
                            custom_logger->error(message.c_str());
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
                    dlog_print(DLOG_WARN, GALogger::tag.c_str(), message.c_str());
#else
                    try
                    {
                        logger->warn(message.c_str());
                        if(custom_logger)
                        {
                            custom_logger->warn(message.c_str());
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
                    dlog_print(DLOG_DEBUG, GALogger::tag.c_str(), message.c_str());
#else
                    try
                    {
                        logger->info(message.c_str());
                        if(custom_logger)
                        {
                            custom_logger->info(message.c_str());
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
                    dlog_print(DLOG_INFO, GALogger::tag.c_str(), message.c_str());
#else
                    try
                    {
                        logger->info(message.c_str());
                        if(custom_logger)
                        {
                            custom_logger->info(message.c_str());
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
