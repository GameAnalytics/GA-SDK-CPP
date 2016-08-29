#include "GALogger.h"
#include <iostream>
#include "GADevice.h"
#include <boost/filesystem.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/core.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/trivial.hpp>

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

			boost::log::core::get()->set_filter
		    (
		        boost::log::trivial::severity >= boost::log::trivial::debug
		    );
		#else
			debugEnabled = false;

			boost::log::core::get()->set_filter
		    (
		        boost::log::trivial::severity >= boost::log::trivial::info
		    );
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
            p /= "ga_log_%3N.txt";

			boost::log::add_file_log(
				boost::log::keywords::file_name = p.string(),
				boost::log::keywords::rotation_size = 1 * 1024 * 1024,
				boost::log::keywords::max_size = 10 * 1024 * 1024,
				boost::log::keywords::format = "[%TimeStamp%]: %Message%",
				boost::log::keywords::auto_flush = true);

			boost::log::add_common_attributes(); 

			GALogger::w("Log file added under: " + path);
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
					std::clog << message << std::endl;
					BOOST_LOG_TRIVIAL(error) << message;
					break;
				case Warning:
					std::clog << message << std::endl;
					BOOST_LOG_TRIVIAL(warning) << message;
					break;
				case Debug:
					std::clog << message << std::endl;
					BOOST_LOG_TRIVIAL(debug) << message;
					break;
				case Info:
					std::clog << message << std::endl;
					BOOST_LOG_TRIVIAL(info) << message;
					break;
			}
		}
	}
}
