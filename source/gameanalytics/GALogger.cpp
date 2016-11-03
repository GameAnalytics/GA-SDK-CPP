#include "GALogger.h"
#include <iostream>
#include "GADevice.h"
#if USE_UWP
#include "GAUtilities.h"
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
            GALoggerUWP::Instance->StartLogging();
            GALoggerUWP::Instance->SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, true);
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
#endif
			switch(type)
			{
				case Error:
#if USE_UWP
                    GALoggerUWP::Instance->Channel->LogMessage(m, Windows::Foundation::Diagnostics::LoggingLevel::Error);
                    LogMessageToConsole(m);
#else
					LOG_ERROR << message;
#endif
					break;

				case Warning:
#if USE_UWP
                    GALoggerUWP::Instance->Channel->LogMessage(m, Windows::Foundation::Diagnostics::LoggingLevel::Warning);
                    LogMessageToConsole(m);
#else
					LOG_WARNING << message;
#endif
					break;

				case Debug:
#if USE_UWP
                    GALoggerUWP::Instance->Channel->LogMessage(ref new Platform::String(utilities::GAUtilities::s2ws(message).c_str()), Windows::Foundation::Diagnostics::LoggingLevel::Information);
                    LogMessageToConsole(m);
#else
					LOG_DEBUG << message;
#endif
					break;

				case Info:
#if USE_UWP
                    GALoggerUWP::Instance->Channel->LogMessage(m, Windows::Foundation::Diagnostics::LoggingLevel::Information);
                    LogMessageToConsole(m);
#else
					LOG_INFO << message;
#endif
					break;
			}
		}

#if USE_UWP
        GALoggerUWP^ GALoggerUWP::_instance;

        GALoggerUWP::GALoggerUWP()
        {
            isPreparingForSuspend = false;
            logFileGeneratedCount = 0;

            // Create the logging channel.
            // When an app logs messages to a channel, the messges will go 
            // to any sessions which are consuming messages from the channel.
            channel = ref new Windows::Foundation::Diagnostics::LoggingChannel(DEFAULT_CHANNEL_NAME, nullptr);

            channel->LoggingEnabled += ref new Windows::Foundation::TypedEventHandler<Windows::Foundation::Diagnostics::ILoggingChannel ^, Platform::Object ^>(this, &GALoggerUWP::OnChannelLoggingEnabled);

            Windows::UI::Xaml::Application::Current->Suspending += ref new Windows::UI::Xaml::SuspendingEventHandler(this, &GALoggerUWP::OnAppSuspending);
            Windows::UI::Xaml::Application::Current->Resuming += ref new Windows::Foundation::EventHandler<Platform::Object ^>(this, &GALoggerUWP::OnAppResuming);

            // If the app is being launched (not resumed), the 
            // following call will activate logging if it had been 
            // active at the time of the last suspend. 
            ResumeLoggingIfApplicable();
        }

        void GALogger::LogMessageToConsole(Platform::Object^ parameter)
        {
            auto paraString = parameter->ToString();
            auto formattedText = std::wstring(paraString->Data()).append(L"\r\n");
            OutputDebugString(formattedText.c_str());
        }

        GALoggerUWP::~GALoggerUWP()
        {
        }

        void GALoggerUWP::OnChannelLoggingEnabled(Windows::Foundation::Diagnostics::ILoggingChannel ^sender, Platform::Object ^args)
        {
            // This method is called when the channel is informing us of channel-related state changes.
            // Save new channel state. These values can be used for advanced logging scenarios where, 
            // for example, it's desired to skip blocks of logging code if the channel is not being
            // consumed by any sessions. 
            isChannelEnabled = sender->Enabled;
            channelLoggingLevel = sender->Level;
        }

        void GALoggerUWP::OnAppSuspending(Platform::Object ^sender, Windows::ApplicationModel::SuspendingEventArgs ^e)
        {
            (void)sender;    // Unused parameter

                             // Get a deferral before performing any async operations
                             // to avoid suspension prior to LoggingScenario completing 
                             // PrepareToSuspendAsync().
            auto deferral = e->SuspendingOperation->GetDeferral();
            PrepareToSuspendAsync().then([=](concurrency::task<void> previousTask)
            {
                // From LoggingScenario's perspective, it's now okay to 
                // suspend, so release the deferral. 
                deferral->Complete();
            });
        }

        void GALoggerUWP::OnAppResuming(Platform::Object ^sender, Platform::Object ^args)
        {
            // If logging was active at the last suspend,
            // ResumeLoggingIfApplicable will re-activate 
            // logging.
            ResumeLoggingIfApplicable();
        }

        concurrency::task<void> GALoggerUWP::PrepareToSuspendAsync()
        {
            if (session != nullptr)
            {
                isPreparingForSuspend = true;
                // Before suspend, save any final log file.
                return concurrency::create_task(CloseSessionSaveFinalLogFile()).then([this](Windows::Storage::StorageFile^ finalLogFile)
                {
                    // Cleanup the session.
                    delete session;
                    session = nullptr;
                    // Save values used when the app is resumed or started later.
                    // Logging is enabled.
                    SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, true);
                    // If there's a final log file...
                    if (finalLogFile != nullptr)
                    {
                        // Save the log file name saved at suspend so the sample UI can be 
                        // updated on resume with that information. 
                        SetAppLocalSettingsValue(LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME, finalLogFile->Path);
                    }
                    else
                    {
                        SetAppLocalSettingsValue(LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME, nullptr);
                    }
                    isPreparingForSuspend = false;
                    return concurrency::task<void>([]() -> void {});
                });
            }
            else
            {
                // Save values used when the app is resumed or started later.
                // Logging is not enabled and no log file was saved.
                SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, false);
                SetAppLocalSettingsValue(LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME, nullptr);
                return concurrency::task<void>([]() -> void {});
            }
        }

        void GALoggerUWP::OnLogFileGenerated(Windows::Foundation::Diagnostics::IFileLoggingSession^ sender, Windows::Foundation::Diagnostics::LogFileGeneratedEventArgs^ args)
        {
            InterlockedIncrement(&logFileGeneratedCount);

            // ContinuationData allows information to be
            // shared across the below async operations.
            struct ContinuationData
            {
                Windows::Storage::StorageFolder^ ourLogFolder;
                Windows::Storage::StorageFile^ generatedLogFile;
                Platform::String^ logFileName;
            };
            auto data = std::make_shared<ContinuationData>();
            data->generatedLogFile = args->File;
            // Create the app-defined folder under the app's local folder.
            // An app defines where/how it wants to store log files. 
            // This sample uses OUR_SAMPLE_APP_LOG_FILE_FOLDER_NAME under
            // the app's local folder. 
            concurrency::create_task(Windows::Storage::ApplicationData::Current->LocalFolder->CreateFolderAsync(OUR_SAMPLE_APP_LOG_FILE_FOLDER_NAME, Windows::Storage::CreationCollisionOption::OpenIfExists))
                .then([this, data](Windows::Storage::StorageFolder^ ourSampleAppLogFolder)
                {
                    // After creating/opening our sample app's log folder, 
                    // save it for use in a continuation further below. 
                    data->ourLogFolder = ourSampleAppLogFolder;
                    // Create a new log file name, and save it for use 
                    // in a continuation further below. 
                    data->logFileName = "Log-" + ref new Platform::String(GetTimeStamp().c_str()) + ".etl";
                    // Move the log file to our sample app's log folder. 
                    return data->generatedLogFile->MoveAsync(ourSampleAppLogFolder, data->logFileName, Windows::Storage::NameCollisionOption::FailIfExists);
                })
                    .then([this, data](concurrency::task<void> previousTask)
                {
                    // Update the UI to show the new log file.
                    if (isPreparingForSuspend == false)
                    {
                        StatusChanged(this, ref new GALoggerEventArgs(GALoggerEventType::LogFileGenerated, std::wstring((data->ourLogFolder->Path + "\\" + data->logFileName)->Data())));
                    }
                });
        }

        concurrency::task<Windows::Storage::StorageFile^> GALoggerUWP::CloseSessionSaveFinalLogFile()
        {
            // ContinuationData allows information to be
            // shared across the below async operations.
            struct ContinuationData
            {
                Windows::Storage::StorageFolder^ ourLogFolder;
                Platform::String^ logFileName;
            };
            auto data = std::make_shared<ContinuationData>();

            // Tell the session to save its final log file.
            return concurrency::create_task(session->CloseAndSaveToFileAsync())
                .then([this, data](Windows::Storage::StorageFile^ finalFileBeforeSuspend)
                {
                    if (finalFileBeforeSuspend != nullptr)
                    {
                        // A final log file was created. 
                        // Move it to the sample-defined log folder. 
                        // First, get create/open our sample-defined log folder.
                        return concurrency::create_task(Windows::Storage::ApplicationData::Current->LocalFolder->CreateFolderAsync(OUR_SAMPLE_APP_LOG_FILE_FOLDER_NAME, Windows::Storage::CreationCollisionOption::OpenIfExists))
                            .then([this, finalFileBeforeSuspend, data](Windows::Storage::StorageFolder^ ourSampleAppLogFolder)
                            {
                                // Next, move the log file to the sample-defined log folder. 
                                data->ourLogFolder = ourSampleAppLogFolder;
                                data->logFileName = "Log-" + ref new Platform::String(GetTimeStamp().c_str()) + ".etl";
                                return finalFileBeforeSuspend->MoveAsync(ourSampleAppLogFolder, data->logFileName, Windows::Storage::NameCollisionOption::FailIfExists);
                            })
                            .then([this, data](concurrency::task<void> previousTask)
                            {
                                // Return a StorageFile representing the log file. 
                                return Windows::Storage::StorageFile::GetFileFromPathAsync(data->ourLogFolder->Path + "\\" + data->logFileName);
                            });
                    }

                    // Async operation need to return consistent types.
                    // If the session did not produce any log file, 
                    // return a task which produces a null StorageFile.
                    return concurrency::task<Windows::Storage::StorageFile^>([]() -> Windows::Storage::StorageFile^
                    {
                        return nullptr;
                    });
                });
        }

        void GALoggerUWP::ResumeLoggingIfApplicable()
        {
            bool loggingEnabled;
            if (IsAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME))
            {
                loggingEnabled = GetAppLocalSettingsValueAsBool(LOGGING_ENABLED_SETTING_KEY_NAME);
            }
            else
            {
                SetAppLocalSettingsValue(LOGGING_ENABLED_SETTING_KEY_NAME, true);
                loggingEnabled = true;
            }

            if (loggingEnabled)
            {
                StartLogging();
            }

            // When the sample suspends, it retains state as to whether or not it had
            // generated a new log file at the last suspension. This allows any
            // UI to be updated on resume to reflect that fact. 
            if (IsAppLocalSettingsValue(LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME))
            {
                Platform::String^ pathOfLogFileGeneratedBeforeSuspend = GetAppLocalSettingsValueAsString(LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME);
                if (pathOfLogFileGeneratedBeforeSuspend != nullptr)
                {
                    StatusChanged(this, ref new GALoggerEventArgs(GALoggerEventType::LogFileGeneratedAtSuspend, pathOfLogFileGeneratedBeforeSuspend->Data()));
                    SetAppLocalSettingsValue(LOGFILEGEN_BEFORE_SUSPEND_SETTING_KEY_NAME, nullptr);
                }
            }
        }

        void GALoggerUWP::StartLogging()
        {
            // If no session exists, create one.
            // NOTE: There are use cases where an application
            // may want to create only a channel for sessions outside
            // of the application itself. See MSDN for details. This
            // sample is the common scenario of an app logging events
            // which it wants to place in its own log file, so it creates
            // a session and channel as a pair. The channel is created 
            // during construction of this GALogger class so 
            // it already exsits by the time this function is called. 
            if (session == nullptr)
            {
                session = ref new Windows::Foundation::Diagnostics::FileLoggingSession(DEFAULT_SESSION_NAME);
                session->LogFileGenerated += ref new Windows::Foundation::TypedEventHandler<Windows::Foundation::Diagnostics::IFileLoggingSession ^, Windows::Foundation::Diagnostics::LogFileGeneratedEventArgs ^>(this, &GALoggerUWP::OnLogFileGenerated);
            }

            session->AddLoggingChannel(channel, Windows::Foundation::Diagnostics::LoggingLevel::Verbose);
        }
#endif
	}
}
