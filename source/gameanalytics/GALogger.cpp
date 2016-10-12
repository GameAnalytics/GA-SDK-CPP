#include "GALogger.h"
#include <iostream>
#include "GADevice.h"
#include <plog/Log.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <boost/filesystem.hpp>

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
		}

		void GALogger::setInfoLog(bool enabled)
		{
			GALogger::sharedInstance()->infoLogEnabled = enabled;
		}

		void GALogger::setVerboseInfoLog(bool enabled)
		{
			GALogger::sharedInstance()->infoLogVerboseEnabled = enabled;
		}

		void GALogger::addFileLog(const std::string& path)
		{
			boost::filesystem::path p(path);
            p /= "ga_log.txt";
			
			GALogger *ga = GALogger::sharedInstance();
			
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
			switch(type)
			{
				case Error:
					LOG_ERROR << message;
					break;
				case Warning:
					LOG_WARNING << message;
					break;
				case Debug:
					LOG_DEBUG << message;
					break;
				case Info:
					LOG_INFO << message;
					break;
			}
		}
	}
}
