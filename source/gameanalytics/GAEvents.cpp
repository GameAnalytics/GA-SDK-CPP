//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#include <vector>
#include "GAEvents.h"
#include "GAState.h"
#include "GAUtilities.h"
#include "GALogger.h"
#include "GAStore.h"
#include "GAThreading.h"
#include "GAValidator.h"

namespace gameanalytics
{
    namespace events
    {
        const std::string GAEvents::CategorySessionStart = "user";
        const std::string GAEvents::CategorySessionEnd = "session_end";
        const std::string GAEvents::CategoryDesign = "design";
        const std::string GAEvents::CategoryBusiness = "business";
        const std::string GAEvents::CategoryProgression = "progression";
        const std::string GAEvents::CategoryResource = "resource";
        const std::string GAEvents::CategoryError = "error";
        const double GAEvents::ProcessEventsIntervalInSeconds = 8.0;
        const int GAEvents::MaxEventCount = 500;

        GAEvents::GAEvents()
        {
            isRunning = false;
            keepRunning = false;
        }

        void GAEvents::stopEventQueue()
        {
            GAEvents::sharedInstance()->keepRunning = false;
        }

        void GAEvents::ensureEventQueueIsRunning()
        {
            GAEvents::sharedInstance()->keepRunning = true;
            if (!GAEvents::sharedInstance()->isRunning)
            {
                GAEvents::sharedInstance()->isRunning = true;
                threading::GAThreading::scheduleTimer(GAEvents::ProcessEventsIntervalInSeconds, processEventQueue);
            }
        }

        // USER EVENTS
        void GAEvents::addSessionStartEvent()
        {
            std::string categorySessionStart = GAEvents::CategorySessionStart;

            // Event specific data
            Json::Value eventDict;
            eventDict["category"] = categorySessionStart;

            // Increment session number  and persist
            state::GAState::incrementSessionNum();
            std::vector<std::string> parameters;
            parameters.push_back("session_num");
            parameters.push_back(std::to_string(state::GAState::getSessionNum()));
            store::GAStore::executeQuerySync("INSERT OR REPLACE INTO ga_state (key, value) VALUES(?, ?);", parameters);

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventDict);

            // Add to store
            addEventToStore(eventDict);

            // Log
            logging::GALogger::i("Add SESSION START event");

            // Send event right away
            GAEvents::processEvents(categorySessionStart, false);
        }

        void GAEvents::addSessionEndEvent()
        {
            Json::Int64 session_start_ts = state::GAState::sharedInstance()->getSessionStart();
            Json::Int64 client_ts_adjusted = state::GAState::getClientTsAdjusted();
            Json::Int64 sessionLength = client_ts_adjusted - session_start_ts;

            if(sessionLength < 0)
            {
                // Should never happen.
                // Could be because of edge cases regarding time altering on device.
                logging::GALogger::w("Session length was calculated to be less then 0. Should not be possible. Resetting to 0.");
                sessionLength = 0;
            }

            // Event specific data
            Json::Value eventDict;

            eventDict["category"] = GAEvents::CategorySessionEnd;
            eventDict["length"] = sessionLength;

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventDict);

            // Add to store
            addEventToStore(eventDict);

            // Log
            logging::GALogger::i("Add SESSION END event.");

            // Send all event right away
            GAEvents::processEvents("", false);
        }

        // BUSINESS EVENT
        void GAEvents::addBusinessEvent(const std::string& currency, int amount, const std::string& itemType, const std::string& itemId, const std::string& cartType)
        {
            // Validate event params
            if (!validators::GAValidator::validateBusinessEvent(currency, amount, cartType, itemType, itemId))
            {
                http::GAHTTPApi::sharedInstance()->sendSdkErrorEvent(http::EGASdkErrorType::Rejected);
                return;
            }

            // Create empty eventData
            Json::Value eventDict;

            // Increment transaction number and persist
            state::GAState::incrementTransactionNum();
            store::GAStore::executeQuerySync("INSERT OR REPLACE INTO ga_state (key, value) VALUES(?, ?);", {"transaction_num", std::to_string(state::GAState::getTransactionNum())});

            // Required
            eventDict["event_id"] = itemType + ":" + itemId;
            eventDict["category"] = GAEvents::CategoryBusiness;
            eventDict["currency"] = currency;
            eventDict["amount"] = amount;
            eventDict["transaction_num"] = state::GAState::getTransactionNum();

            // Optional
            if (!cartType.empty())
            {
                eventDict["cart_type"] = cartType;
            }

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventDict);

            // Log
            logging::GALogger::i("Add BUSINESS event: {currency:" + currency + ", amount:" + std::to_string(amount) + ", itemType:" + itemType + ", itemId:" + itemId + ", cartType:" + cartType + "}");

            // Send to store
            addEventToStore(eventDict);
        }

        void GAEvents::addResourceEvent(EGAResourceFlowType flowType, const std::string& currency, double amount, const std::string& itemType, const std::string& itemId)
        {
            // Validate event params
            if (!validators::GAValidator::validateResourceEvent(flowType, currency, amount, itemType, itemId))
            {
                http::GAHTTPApi::sharedInstance()->sendSdkErrorEvent(http::EGASdkErrorType::Rejected);
                return;
            }

            // If flow type is sink reverse amount
            if (flowType == Sink)
            {
                amount *= -1;
            }

            // Create empty eventData
            Json::Value eventDict;

            // insert event specific values
            std::string flowTypeString = resourceFlowTypeString(flowType);
            eventDict["event_id"] = flowTypeString + ":" + currency + ":" + itemType + ":" + itemId;
            eventDict["category"] = GAEvents::CategoryResource;
            eventDict["amount"] = amount;

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventDict);

            // Log
            logging::GALogger::i("Add RESOURCE event: {currency:" + currency + ", amount:" + std::to_string(amount) + ", itemType:" + itemType + ", itemId:" + itemType + "}");

            // Send to store
            addEventToStore(eventDict);
        }

        void GAEvents::addProgressionEvent(EGAProgressionStatus progressionStatus, const std::string& progression01, const std::string& progression02, const std::string& progression03, double score, bool sendScore)
        {
            std::string progressionStatusString = GAEvents::progressionStatusString(progressionStatus);

            // Validate event params
            if (!validators::GAValidator::validateProgressionEvent(progressionStatus, progression01, progression02, progression03))
            {
                http::GAHTTPApi::sharedInstance()->sendSdkErrorEvent(http::EGASdkErrorType::Rejected);
                return;
            }

            // Create empty eventData
            Json::Value eventDict;

            // Progression identifier
            std::string progressionIdentifier;

            if (progression02.empty())
            {
                progressionIdentifier = progression01;
            }
            else if (progression03.empty())
            {
                progressionIdentifier = progression01 + ":" + progression02;
            }
            else
            {
                progressionIdentifier = progression01 + ":" + progression02 + ":" + progression03;
            }

            // Append event specifics
            eventDict["category"] = GAEvents::CategoryProgression;
            eventDict["event_id"] = progressionStatusString + ":" + progressionIdentifier;

            // Attempt
            double attempt_num = 0;

            // Add score if specified and status is not start
            if (sendScore && progressionStatus != EGAProgressionStatus::Start)
            {
                eventDict["score"] = score;
            }

            // Count attempts on each progression fail and persist
            if (progressionStatus == EGAProgressionStatus::Fail)
            {
                // Increment attempt number
                state::GAState::incrementProgressionTries(progressionIdentifier);
            }

            // increment and add attempt_num on complete and delete persisted
            if (progressionStatus == EGAProgressionStatus::Complete)
            {
                // Increment attempt number
                state::GAState::incrementProgressionTries(progressionIdentifier);

                // Add to event
                attempt_num = state::GAState::getProgressionTries(progressionIdentifier);
                eventDict["attempt_num"] = attempt_num;

                // Clear
                state::GAState::clearProgressionTries(progressionIdentifier);
            }

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventDict);

            // Log
            logging::GALogger::i("Add PROGRESSION event: {status:" + progressionStatusString + ", progression01:" + progression01 + ", progression02:" + progression02 + ", progression03:" + progression03 + ", score:" + std::to_string(score) + ", attempt:" + std::to_string(attempt_num) + "}");

            // Send to store
            addEventToStore(eventDict);
        }

        void GAEvents::addDesignEvent(const std::string& eventId, double value, bool sendValue)
        {
            // Validate
            if (!validators::GAValidator::validateDesignEvent(eventId, value))
            {
                http::GAHTTPApi::sharedInstance()->sendSdkErrorEvent(http::EGASdkErrorType::Rejected);
                return;
            }

            // Create empty eventData
            Json::Value eventData;

            // Append event specifics
            eventData["category"] = GAEvents::CategoryDesign;
            eventData["event_id"] = eventId;

            if (sendValue)
            {
                eventData["value"] = value;
            }

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventData);

            // Log
            logging::GALogger::i("Add DESIGN event: {eventId:" + eventId + ", value:" + std::to_string(value) + "}");

            // Send to store
            addEventToStore(eventData);
        }

        void GAEvents::addErrorEvent(EGAErrorSeverity severity, const std::string& message)
        {
            std::string severityString = errorSeverityString(severity);

            // Validate
            if (!validators::GAValidator::validateErrorEvent(severity, message))
            {
                http::GAHTTPApi::sharedInstance()->sendSdkErrorEvent(http::EGASdkErrorType::Rejected);
                return;
            }

            // Create empty eventData
            Json::Value eventData;

            // Append event specifics
            eventData["category"] = GAEvents::CategoryError;
            eventData["severity"] = severityString;
            eventData["message"] = message;

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventData);

            // Log
            logging::GALogger::i("Add ERROR event: {severity:" + severityString + ", message:" + message + "}");

            // Send to store
            addEventToStore(eventData);
        }

        void GAEvents::processEventQueue()
        {
            processEvents("", true);
            if (GAEvents::sharedInstance()->keepRunning)
            {
                threading::GAThreading::scheduleTimer(GAEvents::ProcessEventsIntervalInSeconds, processEventQueue);
            }
            else
            {
                GAEvents::sharedInstance()->isRunning = false;
            }
        }

        void GAEvents::processEvents(const std::string& category, bool performCleanup)
        {
            // Request identifier
            std::string requestIdentifier = utilities::GAUtilities::generateUUID();

            std::string selectSql;
            std::string updateSql;
            std::string deleteSql = "DELETE FROM ga_events WHERE status = '" + requestIdentifier + "'";
            std::string putbackSql = "UPDATE ga_events SET status = 'new' WHERE status = '" + requestIdentifier + "';";

            // Cleanup
            if (performCleanup)
            {
                cleanupEvents();
                fixMissingSessionEndEvents();
            }

            // Prepare SQL
            std::string andCategory = "";
            if (!category.empty())
            {
                andCategory = " AND category='" + category + "' ";
            }
            selectSql = "SELECT event FROM ga_events WHERE status = 'new' " + andCategory + ";";
            updateSql = "UPDATE ga_events SET status = '" + requestIdentifier + "' WHERE status = 'new' " + andCategory + ";";

            // Get events to process
            auto events = store::GAStore::executeQuerySync(selectSql);

            // Check for errors or empty
            if (events.empty())
            {
                logging::GALogger::i("Event queue: No events to send");
                GAEvents::updateSessionTime();
                return;
            }

            // Check number of events and take some action if there are too many?
            if (events.size() > GAEvents::MaxEventCount)
            {
                // Make a limit request
                selectSql = "SELECT client_ts FROM ga_events WHERE status = 'new' " + andCategory + " ORDER BY client_ts ASC LIMIT 0," + std::to_string(GAEvents::MaxEventCount) + ";";
                events = store::GAStore::executeQuerySync(selectSql);
                if (events.empty())
                {
                    return;
                }

                // Get last timestamp
                auto lastItem = events[events.size() - 1];
                auto lastTimestamp = lastItem["client_ts"].asString();

                // Select again
                selectSql = "SELECT event FROM ga_events WHERE status = 'new' " + andCategory + " AND client_ts<='" + lastTimestamp + "';";
                events = store::GAStore::executeQuerySync(selectSql);
                if (events.empty())
                {
                    return;
                }

                // Update sql
                updateSql = "UPDATE ga_events SET status='" + requestIdentifier + "' WHERE status='new' " + andCategory + " AND client_ts<='" + lastTimestamp + "';";
            }



            // Log
            logging::GALogger::i("Event queue: Sending " + std::to_string(events.size()) + " events.");

            // Set status of events to 'sending' (also check for error)
            if (store::GAStore::executeQuerySync(updateSql).isNull())
            {
                return;
            }

            // Create payload data from events
            std::vector<Json::Value> payloadArray;

            for (auto ev : events)
            {
                auto eventDict = utilities::GAUtilities::jsonFromString(ev["event"].asString());
                if (!eventDict.empty())
                {
                    payloadArray.push_back(eventDict);
                }
            }

            // send events
#if USE_UWP
            std::pair<http::EGAHTTPApiResponse, Json::Value> pair = http::GAHTTPApi::sharedInstance()->sendEventsInArray(payloadArray).get();
#else
            std::pair<http::EGAHTTPApiResponse, Json::Value> pair = http::GAHTTPApi::sharedInstance()->sendEventsInArray(payloadArray);
#endif
            Json::Value dataDict = pair.second;
            http::EGAHTTPApiResponse responseEnum = pair.first;

            if (responseEnum == http::Ok)
            {
                // Delete events
                store::GAStore::executeQuerySync(deleteSql);
                logging::GALogger::i("Event queue: " + std::to_string(events.size()) + " events sent.");
            }
            else
            {
                // Put events back (Only in case of no response)
                if (responseEnum == http::NoResponse)
                {
                    logging::GALogger::w("Event queue: Failed to send events to collector - Retrying next time");
                    store::GAStore::executeQuerySync(putbackSql);
                    // Delete events (When getting some anwser back always assume events are processed)
                }
                else
                {
                    if (responseEnum == http::BadRequest && dataDict.isArray())
                    {
                        logging::GALogger::w("Event queue: " + std::to_string(events.size()) + " events sent. " + std::to_string(dataDict.size()) + " events failed GA server validation.");
                    }
                    else
                    {
                        logging::GALogger::w("Event queue: Failed to send events.");
                    }

                    store::GAStore::executeQuerySync(deleteSql);
                }
            }
        }

        void GAEvents::updateSessionTime()
        {
            if(state::GAState::sessionIsStarted())
            {
                Json::Value ev = state::GAState::getEventAnnotations();
                auto jsonDefaults = utilities::GAUtilities::jsonToString(ev);
                std::string sql = "INSERT OR REPLACE INTO ga_session(session_id, timestamp, event) VALUES(?, ?, ?);";
                std::vector<std::string> parameters = { ev["session_id"].asString(), std::to_string(static_cast<int>(state::GAState::sharedInstance()->getSessionStart())), jsonDefaults};
                store::GAStore::executeQuerySync(sql, parameters);
            }
        }

        void GAEvents::cleanupEvents()
        {
            store::GAStore::executeQuerySync("UPDATE ga_events SET status = 'new';");
        }

        void GAEvents::fixMissingSessionEndEvents()
        {
            // Get all sessions that are not current
            std::vector<std::string> parameters = { state::GAState::getSessionId() };

            std::string sql = "SELECT timestamp, event FROM ga_session WHERE session_id != ?;";
            auto sessions = store::GAStore::executeQuerySync(sql, parameters);

            if (sessions.empty())
            {
                return;
            }

            logging::GALogger::i(std::to_string(sessions.size()) + " session(s) located with missing session_end event.");

            // Add missing session_end events
            for (auto session : sessions)
            {
                auto sessionEndEvent = utilities::GAUtilities::jsonFromString(session["event"].asString());
                auto event_ts = sessionEndEvent["client_ts"].asInt();
                auto start_ts = utilities::GAUtilities::parseString<int>(session.get("timestamp", "0").asString());

                auto length = event_ts - start_ts;

                logging::GALogger::d("fixMissingSessionEndEvents length calculated: " + std::to_string(static_cast<int>(length)));

                sessionEndEvent["category"] = GAEvents::CategorySessionEnd;
                sessionEndEvent["length"] = length;

                // Add to store
                addEventToStore(sessionEndEvent);
            }
        }

        // GENERAL
        void GAEvents::addEventToStore(const Json::Value& eventData)
        {
            // Check if datastore is available
            if (!store::GAStore::sharedInstance()->getTableReady())
            {
                logging::GALogger::w("Could not add event: SDK datastore error");
                return;
            }

            // Check if we are initialized
            if (!state::GAState::isInitialized())
            {
                logging::GALogger::w("Could not add event: SDK is not initialized");
                return;
            }

            // Check db size limits (10mb)
            // If database is too large block all except user, session and business
            if (store::GAStore::isDbTooLargeForEvents() && !utilities::GAUtilities::stringMatch(eventData["category"].asString(), "^(user|session_end|business)$"))
            {
                logging::GALogger::w("Database too large. Event has been blocked.");
                return;
            }

            // Get default annotations
            Json::Value ev = state::GAState::getEventAnnotations();

            // Create json with only default annotations
            auto jsonDefaults = utilities::GAUtilities::jsonToString(ev);

            // Merge with eventData
            for (auto e : eventData.getMemberNames())
            {
                ev[e] = eventData[e];
            }

            // Create json string representation
            std::string json = utilities::GAUtilities::jsonToString(ev);

            // output if VERBOSE LOG enabled

            logging::GALogger::ii("Event added to queue: " + json);

            // Add to store
            std::vector<std::string> parameters = { "new", ev["category"].asString(), ev["session_id"].asString(), ev["client_ts"].asString(), json };
            std::string sql = "INSERT INTO ga_events (status, category, session_id, client_ts, event) VALUES(?, ?, ?, ?, ?);";

            store::GAStore::executeQuerySync(sql, parameters);

            // Add to session store if not last
            if (eventData["category"].asString() == GAEvents::CategorySessionEnd)
            {
                parameters = { ev["session_id"].asString() };
                sql = "DELETE FROM ga_session WHERE session_id = ?;";
                store::GAStore::executeQuerySync(sql, parameters);
            }
            else
            {
                sql = "INSERT OR REPLACE INTO ga_session(session_id, timestamp, event) VALUES(?, ?, ?);";
                parameters = { ev["session_id"].asString(), std::to_string(static_cast<int>(state::GAState::sharedInstance()->getSessionStart())), jsonDefaults};
                store::GAStore::executeQuerySync(sql, parameters);
            }
        }

        void GAEvents::addDimensionsToEvent(Json::Value& eventData)
        {
            if (!eventData)
            {
                return;
            }
            // add to dict (if not nil)
            if (!state::GAState::getCurrentCustomDimension01().empty())
            {
                eventData["custom_01"] = state::GAState::getCurrentCustomDimension01();
            }
            if (!state::GAState::getCurrentCustomDimension02().empty())
            {
                eventData["custom_02"] = state::GAState::getCurrentCustomDimension02();
            }
            if (!state::GAState::getCurrentCustomDimension03().empty())
            {
                eventData["custom_03"] = state::GAState::getCurrentCustomDimension03();
            }
        }

        const std::string GAEvents::progressionStatusString(EGAProgressionStatus progressionStatus)
        {
            switch (progressionStatus) {
            case Start:
                return "Start";
            case Complete:
                return "Complete";
            case Fail:
                return "Fail";
            default:
                break;
            }
            return{};
        }

        const std::string GAEvents::errorSeverityString(EGAErrorSeverity errorSeverity)
        {
            switch (errorSeverity) {
            case Info:
                return "info";
            case Debug:
                return "debug";
            case Warning:
                return "warning";
            case Error:
                return "error";
            case Critical:
                return "critical";
            default:
                break;
            }
            return{};
        }

        const std::string GAEvents::resourceFlowTypeString(EGAResourceFlowType flowType)
        {
            switch (flowType) {
            case Source:
                return "Source";
            case Sink:
                return "Sink";
            default:
                break;
            }
            return{};
        }
    }
}
