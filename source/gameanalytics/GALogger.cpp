#include "GALogger.h"
#include <iostream>
#include "GADevice.h"
#if USE_UWP
#include "GAUtilities.h"
#include <collection.h>
#else
#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <boost/filesystem.hpp>
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
            file = concurrency::create_task(Windows::Storage::ApplicationData::Current->LocalFolder->CreateFileAsync("ga_log.txt", Windows::Storage::CreationCollisionOption::ReplaceExisting)).get();
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

#if !USE_UWP
		void GALogger::addFileLog(const std::string& path)
		{
            GALogger *ga = GALogger::sharedInstance();
			boost::filesystem::path p(path);
            p /= "ga_log.txt";
			
			static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(p.string().c_str(), 1 * 1024 * 1024, 10);
			static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
			
			if(ga->debugEnabled)
			{
				plog::init(plog::debug, &fileAppender).addAppender(&consoleAppender);
			}
			else
			{
				plog::init(plog::info, &fileAppender).addAppender(&consoleAppender);
			}

			GALogger::i("Log file added under: " + path);
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
#if USE_UWP
            auto m = ref new Platform::String(utilities::GAUtilities::s2ws(message).c_str());
            Platform::Collections::Vector<Platform::String^>^ lines = ref new Platform::Collections::Vector<Platform::String^>();
            lines->Append(m);
#endif
			switch(type)
			{
				case Error:
#if USE_UWP
                    concurrency::create_task(Windows::Storage::FileIO::AppendLinesAsync(file, lines)).wait();
                    LogMessageToConsole(m);
#else
					LOG_ERROR << message;
#endif
					break;

				case Warning:
#if USE_UWP
                    concurrency::create_task(Windows::Storage::FileIO::AppendLinesAsync(file, lines)).wait();
                    LogMessageToConsole(m);
#else
					LOG_WARNING << message;
#endif
					break;

				case Debug:
#if USE_UWP
                    concurrency::create_task(Windows::Storage::FileIO::AppendLinesAsync(file, lines)).wait();
                    LogMessageToConsole(m);
#else
					LOG_DEBUG << message;
#endif
					break;

				case Info:
#if USE_UWP
                    concurrency::create_task(Windows::Storage::FileIO::AppendLinesAsync(file, lines)).wait();
                    LogMessageToConsole(m);
#else
					LOG_INFO << message;
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
