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
#include <string.h>
#include <stdio.h>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

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
            if(!state::GAState::isEventSubmissionEnabled())
            {
                return;
            }

            std::string categorySessionStart = GAEvents::CategorySessionStart;

            // Event specific data
            rapidjson::Document eventDict;
            eventDict.SetObject();
            rapidjson::Document::AllocatorType& allocator = eventDict.GetAllocator();
            {
                rapidjson::Value v(categorySessionStart.c_str(), allocator);
                eventDict.AddMember("category", v.Move(), allocator);
            }


            // Increment session number  and persist
            state::GAState::incrementSessionNum();
            char sessionNum[11] = "";
            snprintf(sessionNum, sizeof(sessionNum), "%d", state::GAState::getSessionNum());
            const char* parameters[2] = {"session_num", sessionNum}
            store::GAStore::executeQuerySync("INSERT OR REPLACE INTO ga_state (key, value) VALUES(?, ?);", parameters, 2);

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
            if(!state::GAState::isEventSubmissionEnabled())
            {
                return;
            }

            int64_t session_start_ts = state::GAState::sharedInstance()->getSessionStart();
            int64_t client_ts_adjusted = state::GAState::getClientTsAdjusted();
            int64_t sessionLength = client_ts_adjusted - session_start_ts;

            if(sessionLength < 0)
            {
                // Should never happen.
                // Could be because of edge cases regarding time altering on device.
                logging::GALogger::w("Session length was calculated to be less then 0. Should not be possible. Resetting to 0.");
                sessionLength = 0;
            }

            // Event specific data
            rapidjson::Document eventDict;
            eventDict.SetObject();
            rapidjson::Document::AllocatorType& allocator = eventDict.GetAllocator();

            {
                rapidjson::Value v(GAEvents::CategorySessionEnd.c_str(), allocator);
                eventDict.AddMember("category", v.Move(), allocator);
            }
            eventDict.AddMember("length", sessionLength, allocator);

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
        void GAEvents::addBusinessEvent(const std::string& currency, int amount, const std::string& itemType, const std::string& itemId, const std::string& cartType, const rapidjson::Value& fields)
        {
            if(!state::GAState::isEventSubmissionEnabled())
            {
                return;
            }

            // Validate event params
            if (!validators::GAValidator::validateBusinessEvent(currency, amount, cartType, itemType, itemId))
            {
                http::GAHTTPApi::sharedInstance()->sendSdkErrorEvent(http::EGASdkErrorType::Rejected);
                return;
            }

            // Create empty eventData
            rapidjson::Document eventDict;
            eventDict.SetObject();
            rapidjson::Document::AllocatorType& allocator = eventDict.GetAllocator();

            // Increment transaction number and persist
            state::GAState::incrementTransactionNum();
            store::GAStore::executeQuerySync("INSERT OR REPLACE INTO ga_state (key, value) VALUES(?, ?);", {"transaction_num", std::to_string(state::GAState::getTransactionNum())});

            // Required
            {
                rapidjson::Value v((itemType + ":" + itemId).c_str(), allocator);
                eventDict.AddMember("event_id", v.Move(), allocator);
            }
            {
                rapidjson::Value v(GAEvents::CategoryBusiness.c_str(), allocator);
                eventDict.AddMember("category", v.Move(), allocator);
            }
            {
                rapidjson::Value v(currency.c_str(), allocator);
                eventDict.AddMember("currency", v.Move(), allocator);
            }
            eventDict.AddMember("amount", amount, allocator);
            eventDict.AddMember("transaction_num", state::GAState::getTransactionNum(), allocator);

            // Optional
            if (!cartType.empty())
            {
                rapidjson::Value v(cartType.c_str(), allocator);
                eventDict.AddMember("cart_type", v.Move(), allocator);
            }

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventDict);

            rapidjson::Document cleanedFields;
            cleanedFields.SetObject();
            state::GAState::validateAndCleanCustomFields(fields, cleanedFields);
            GAEvents::addFieldsToEvent(eventDict, cleanedFields);

            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                fields.Accept(writer);
            }

            // Log
            logging::GALogger::i("Add BUSINESS event: {currency:" + currency + ", amount:" + std::to_string(amount) + ", itemType:" + itemType + ", itemId:" + itemId + ", cartType:" + cartType + ", fields:" + std::string(buffer.GetString()) + "}");

            // Send to store
            addEventToStore(eventDict);
        }

        void GAEvents::addResourceEvent(EGAResourceFlowType flowType, const char* currency, double amount, const char* itemType, const char* itemId, const rapidjson::Value& fields)
        {
            if(!state::GAState::isEventSubmissionEnabled())
            {
                return;
            }

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
            rapidjson::Document eventDict;
            eventDict.SetObject();
            rapidjson::Document::AllocatorType& allocator = eventDict.GetAllocator();

            // insert event specific values
            std::string flowTypeString = resourceFlowTypeString(flowType);
            {
                rapidjson::Value v((flowTypeString + ":" + currency + ":" + itemType + ":" + itemId).c_str(), allocator);
                eventDict.AddMember("event_id", v.Move(), allocator);
            }
            {
                rapidjson::Value v(GAEvents::CategoryResource.c_str(), allocator);
                eventDict.AddMember("category", v.Move(), allocator);
            }
            eventDict.AddMember("amount", amount, allocator);

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventDict);

            rapidjson::Document cleanedFields;
            cleanedFields.SetObject();
            state::GAState::validateAndCleanCustomFields(fields, cleanedFields);
            GAEvents::addFieldsToEvent(eventDict, cleanedFields);

            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                fields.Accept(writer);
            }

            // Log
            int size = strlen(currency) + strlen(itemType) + strlen(itemId) + strlen(buffer.GetString()) + 129;
            char s[size];
            snprintf(s, sizeof(s), "Add RESOURCE event: {currency:%s, amount: %d, itemType:%s, itemId:%s, fields:%s}", currency, amount, itemType, itemId, buffer.GetString());
            logging::GALogger::i(s);

            // Send to store
            addEventToStore(eventDict);
        }

        void GAEvents::addProgressionEvent(EGAProgressionStatus progressionStatus, const std::string& progression01, const std::string& progression02, const std::string& progression03, double score, bool sendScore, const rapidjson::Value& fields)
        {
            if(!state::GAState::isEventSubmissionEnabled())
            {
                return;
            }

            std::string progressionStatusString = GAEvents::progressionStatusString(progressionStatus);

            // Validate event params
            if (!validators::GAValidator::validateProgressionEvent(progressionStatus, progression01, progression02, progression03))
            {
                http::GAHTTPApi::sharedInstance()->sendSdkErrorEvent(http::EGASdkErrorType::Rejected);
                return;
            }

            // Create empty eventData
            rapidjson::Document eventDict;
            eventDict.SetObject();
            rapidjson::Document::AllocatorType& allocator = eventDict.GetAllocator();

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
            {
                rapidjson::Value v(GAEvents::CategoryProgression.c_str(), allocator);
                eventDict.AddMember("category", v.Move(), allocator);
            }
            {
                rapidjson::Value v((progressionStatusString + ":" + progressionIdentifier).c_str(), allocator);
                eventDict.AddMember("event_id", v.Move(), allocator);
            }

            // Attempt
            double attempt_num = 0;

            // Add score if specified and status is not start
            if (sendScore && progressionStatus != EGAProgressionStatus::Start)
            {
                eventDict.AddMember("score", score, allocator);
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
                eventDict.AddMember("attempt_num", attempt_num, allocator);

                // Clear
                state::GAState::clearProgressionTries(progressionIdentifier);
            }

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventDict);

            rapidjson::Document cleanedFields;
            cleanedFields.SetObject();
            state::GAState::validateAndCleanCustomFields(fields, cleanedFields);
            GAEvents::addFieldsToEvent(eventDict, cleanedFields);

            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                fields.Accept(writer);
            }

            // Log
            logging::GALogger::i("Add PROGRESSION event: {status:" + progressionStatusString + ", progression01:" + progression01 + ", progression02:" + progression02 + ", progression03:" + progression03 + ", score:" + std::to_string(score) + ", attempt:" + std::to_string(attempt_num) + ", fields:" + std::string(buffer.GetString()) + "}");

            // Send to store
            addEventToStore(eventDict);
        }

        void GAEvents::addDesignEvent(const std::string& eventId, double value, bool sendValue, const rapidjson::Value& fields)
        {
            if(!state::GAState::isEventSubmissionEnabled())
            {
                return;
            }

            // Validate
            if (!validators::GAValidator::validateDesignEvent(eventId, value))
            {
                http::GAHTTPApi::sharedInstance()->sendSdkErrorEvent(http::EGASdkErrorType::Rejected);
                return;
            }

            // Create empty eventData
            rapidjson::Document eventData;
            eventData.SetObject();
            rapidjson::Document::AllocatorType& allocator = eventData.GetAllocator();

            // Append event specifics
            {
                rapidjson::Value v(GAEvents::CategoryDesign.c_str(), allocator);
                eventData.AddMember("category", v.Move(), allocator);
            }
            {
                rapidjson::Value v(eventId.c_str(), allocator);
                eventData.AddMember("event_id", v.Move(), allocator);
            }

            if (sendValue)
            {
                eventData.AddMember("value", value, allocator);
            }

            rapidjson::Document cleanedFields;
            cleanedFields.SetObject();
            state::GAState::validateAndCleanCustomFields(fields, cleanedFields);
            GAEvents::addFieldsToEvent(eventData, cleanedFields);

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventData);

            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                fields.Accept(writer);
            }

            // Log
            logging::GALogger::i("Add DESIGN event: {eventId:" + eventId + ", value:" + std::to_string(value) + ", fields:" + std::string(buffer.GetString()) + "}");

            // Send to store
            addEventToStore(eventData);
        }

        void GAEvents::addErrorEvent(EGAErrorSeverity severity, const std::string& message, const rapidjson::Value& fields)
        {
            if(!state::GAState::isEventSubmissionEnabled())
            {
                return;
            }

            std::string severityString = errorSeverityString(severity);

            // Validate
            if (!validators::GAValidator::validateErrorEvent(severity, message))
            {
                http::GAHTTPApi::sharedInstance()->sendSdkErrorEvent(http::EGASdkErrorType::Rejected);
                return;
            }

            // Create empty eventData
            rapidjson::Document eventData;
            eventData.SetObject();
            rapidjson::Document::AllocatorType& allocator = eventData.GetAllocator();

            // Append event specifics
            {
                rapidjson::Value v(GAEvents::CategoryError.c_str(), allocator);
                eventData.AddMember("category", v.Move(), allocator);
            }
            {
                rapidjson::Value v(severityString.c_str(), allocator);
                eventData.AddMember("severity", v.Move(), allocator);
            }
            {
                rapidjson::Value v(message.c_str(), allocator);
                eventData.AddMember("message", v.Move(), allocator);
            }

            rapidjson::Document cleanedFields;
            cleanedFields.SetObject();
            state::GAState::validateAndCleanCustomFields(fields, cleanedFields);
            GAEvents::addFieldsToEvent(eventData, cleanedFields);

            // Add custom dimensions
            GAEvents::addDimensionsToEvent(eventData);

            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                fields.Accept(writer);
            }

            // Log
            logging::GALogger::i("Add ERROR event: {severity:" + severityString + ", message:" + message + ", fields:" + std::string(buffer.GetString()) + "}");

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
            if(!state::GAState::isEventSubmissionEnabled())
            {
                return;
            }

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
            rapidjson::Value events;
            store::GAStore::executeQuerySync(selectSql, events);

            // Check for errors or empty
            if (events.IsNull())
            {
                logging::GALogger::i("Event queue: No events to send");
                GAEvents::updateSessionTime();
                return;
            }

            // Check number of events and take some action if there are too many?
            if (events.Size() > GAEvents::MaxEventCount)
            {
                // Make a limit request
                selectSql = "SELECT client_ts FROM ga_events WHERE status = 'new' " + andCategory + " ORDER BY client_ts ASC LIMIT 0," + std::to_string(GAEvents::MaxEventCount) + ";";
                store::GAStore::executeQuerySync(selectSql, events);
                if (events.IsNull())
                {
                    return;
                }

                // Get last timestamp
                const rapidjson::Value& lastItem = events[events.Size() - 1];
                const char* lastTimestamp = lastItem["client_ts"].GetString();

                // Select again
                selectSql = "SELECT event FROM ga_events WHERE status = 'new' " + andCategory + " AND client_ts<='" + lastTimestamp + "';";
                store::GAStore::executeQuerySync(selectSql, events);
                if (events.IsNull())
                {
                    return;
                }

                // Update sql
                updateSql = "UPDATE ga_events SET status='" + requestIdentifier + "' WHERE status='new' " + andCategory + " AND client_ts<='" + lastTimestamp + "';";
            }



            // Log
            logging::GALogger::i("Event queue: Sending " + std::to_string(events.Size()) + " events.");

            // Set status of events to 'sending' (also check for error)
            rapidjson::Value updateResult(rapidjson::kObjectType);
            store::GAStore::executeQuerySync(updateSql, updateResult);
            if (updateResult.IsNull())
            {
                return;
            }

            // Create payload data from events
            rapidjson::Document payloadArray;
            payloadArray.SetArray();
            rapidjson::Document::AllocatorType& allocator = payloadArray.GetAllocator();

            for (rapidjson::Value::ConstValueIterator itr = events.Begin(); itr != events.End(); ++itr)
            {
                const char* eventDict = (*itr).HasMember("event") ? (*itr)["event"].GetString() : "";
                if (strlen(eventDict) > 0)
                {
                    rapidjson::Document d;
                    d.Parse(eventDict);
                    payloadArray.PushBack(d.Move(), allocator);
                }
            }

            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                events.Accept(writer);
            }

            // send events
            rapidjson::Value dataDict(rapidjson::kArrayType);
            http::EGAHTTPApiResponse responseEnum;
#if USE_UWP
            std::pair<http::EGAHTTPApiResponse, Json::Value> pair;

            try
            {
                pair = http::GAHTTPApi::sharedInstance()->sendEventsInArray(payloadArray).get();
            }
            catch(Platform::COMException^ e)
            {
                pair = std::pair<http::EGAHTTPApiResponse, Json::Value>(http::NoResponse, Json::Value());
            }
#else
            http::GAHTTPApi::sharedInstance()->sendEventsInArray(responseEnum, dataDict, payloadArray);
#endif

            if (responseEnum == http::Ok)
            {
                // Delete events
                store::GAStore::executeQuerySync(deleteSql);
                logging::GALogger::i("Event queue: " + std::to_string(events.Size()) + " events sent.");
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
                    if (responseEnum == http::BadRequest && dataDict.IsArray())
                    {
                        logging::GALogger::w("Event queue: " + std::to_string(events.Size()) + " events sent. " + std::to_string(dataDict.Size()) + " events failed GA server validation.");
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
                rapidjson::Document ev;
                ev.SetObject();
                state::GAState::getEventAnnotations(ev);
                rapidjson::StringBuffer buffer;
                {
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    ev.Accept(writer);
                }
                std::string jsonDefaults = buffer.GetString();
                std::string sql = "INSERT OR REPLACE INTO ga_session(session_id, timestamp, event) VALUES(?, ?, ?);";
                std::vector<std::string> parameters = { ev["session_id"].GetString(), std::to_string(static_cast<int>(state::GAState::sharedInstance()->getSessionStart())), jsonDefaults};
                store::GAStore::executeQuerySync(sql, parameters);
            }
        }

        void GAEvents::cleanupEvents()
        {
            store::GAStore::executeQuerySync("UPDATE ga_events SET status = 'new';");
        }

        void GAEvents::fixMissingSessionEndEvents()
        {
            if(!state::GAState::isEventSubmissionEnabled())
            {
                return;
            }

            // Get all sessions that are not current
            std::vector<std::string> parameters = { state::GAState::getSessionId() };

            std::string sql = "SELECT timestamp, event FROM ga_session WHERE session_id != ?;";
            rapidjson::Value sessions(rapidjson::kArrayType);
            store::GAStore::executeQuerySync(sql, parameters, sessions);

            if (sessions.Empty())
            {
                return;
            }

            logging::GALogger::i(std::to_string(sessions.Size()) + " session(s) located with missing session_end event.");

            // Add missing session_end events
            for (rapidjson::Value::ConstValueIterator itr = sessions.Begin(); itr != sessions.End(); ++itr)
            {
                const rapidjson::Value& session = *itr;
                rapidjson::StringBuffer buffer;
                {
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    session.Accept(writer);
                }
                rapidjson::Document sessionEndEvent;
                sessionEndEvent.Parse(buffer.GetString());
                rapidjson::Document::AllocatorType& allocator = sessionEndEvent.GetAllocator();
                int64_t event_ts = sessionEndEvent.HasMember("client_ts") ? sessionEndEvent["client_ts"].GetInt64() : 0;
                int64_t start_ts = utilities::GAUtilities::parseString<int64_t>(session.HasMember("timestamp") ? session["timestamp"].GetString() : "0");

                int64_t length = event_ts - start_ts;

                logging::GALogger::d("fixMissingSessionEndEvents length calculated: " + std::to_string(static_cast<int>(length)));

                {
                    rapidjson::Value v(GAEvents::CategorySessionEnd.c_str(), allocator);
                    sessionEndEvent.AddMember("category", v.Move(), allocator);
                }
                sessionEndEvent.AddMember("length", length, allocator);

                // Add to store
                addEventToStore(sessionEndEvent);
            }
        }

        // GENERAL
        void GAEvents::addEventToStore(const rapidjson::Value& eventData)
        {
            if(!state::GAState::isEventSubmissionEnabled())
            {
                return;
            }

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
            if (store::GAStore::isDbTooLargeForEvents() && !utilities::GAUtilities::stringMatch(eventData["category"].GetString(), "^(user|session_end|business)$"))
            {
                logging::GALogger::w("Database too large. Event has been blocked.");
                return;
            }

            // Get default annotations
            rapidjson::Document ev;
            ev.SetObject();
            rapidjson::Document::AllocatorType& allocator = ev.GetAllocator();
            state::GAState::getEventAnnotations(ev);

            // Create json with only default annotations
            std::string jsonDefaults;
            {
                rapidjson::StringBuffer buffer;
                {
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    ev.Accept(writer);
                }
                jsonDefaults = buffer.GetString();
            }

            // Merge with eventData
            for (rapidjson::Value::ConstMemberIterator itr = eventData.MemberBegin(); itr != eventData.MemberEnd(); ++itr)
            {
                const char* key = itr->name.GetString();
                const rapidjson::Value& value = eventData[key];

                if(value.IsNumber())
                {
                    rapidjson::Value v(key, allocator);
                    ev.AddMember(v.Move(), value.GetDouble(), allocator);
                }
                else if(value.IsString())
                {
                    rapidjson::Value v(key, allocator);
                    rapidjson::Value v1(value.GetString(), allocator);
                    ev.AddMember(v.Move(), v1.Move(), allocator);
                }
            }

            // Create json string representation
            std::string json;
            {
                rapidjson::StringBuffer buffer;
                {
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    ev.Accept(writer);
                }
                json = buffer.GetString();
            }

            // output if VERBOSE LOG enabled
            logging::GALogger::ii("Event added to queue: " + json);

            // Add to store
            std::vector<std::string> parameters = { "new", ev["category"].GetString(), ev["session_id"].GetString(), std::to_string(ev["client_ts"].GetInt64()), json };
            std::string sql = "INSERT INTO ga_events (status, category, session_id, client_ts, event) VALUES(?, ?, ?, ?, ?);";

            store::GAStore::executeQuerySync(sql, parameters);

            // Add to session store if not last
            if (eventData["category"].GetString() == GAEvents::CategorySessionEnd)
            {
                parameters = { ev["session_id"].GetString() };
                sql = "DELETE FROM ga_session WHERE session_id = ?;";
                store::GAStore::executeQuerySync(sql, parameters);
            }
            else
            {
                sql = "INSERT OR REPLACE INTO ga_session(session_id, timestamp, event) VALUES(?, ?, ?);";
                parameters = { ev["session_id"].GetString(), std::to_string(static_cast<int>(state::GAState::sharedInstance()->getSessionStart())), jsonDefaults};
                store::GAStore::executeQuerySync(sql, parameters);
            }
        }

        void GAEvents::addDimensionsToEvent(rapidjson::Document& eventData)
        {
            if (eventData.IsNull())
            {
                return;
            }

            rapidjson::Document::AllocatorType& allocator = eventData.GetAllocator();

            // add to dict (if not nil)
            if (!state::GAState::getCurrentCustomDimension01().empty())
            {
                rapidjson::Value v(state::GAState::getCurrentCustomDimension01().c_str(), allocator);
                eventData.AddMember("custom_01", v.Move(), allocator);
            }
            if (!state::GAState::getCurrentCustomDimension02().empty())
            {
                rapidjson::Value v(state::GAState::getCurrentCustomDimension02().c_str(), allocator);
                eventData.AddMember("custom_02", v.Move(), allocator);
            }
            if (!state::GAState::getCurrentCustomDimension03().empty())
            {
                rapidjson::Value v(state::GAState::getCurrentCustomDimension03().c_str(), allocator);
                eventData.AddMember("custom_03", v.Move(), allocator);
            }
        }

        void GAEvents::addFieldsToEvent(rapidjson::Document& eventData, rapidjson::Document& fields)
        {
            if(eventData.IsNull())
            {
                return;
            }

            if(!fields.ObjectEmpty())
            {
                rapidjson::Value v(rapidjson::kObjectType);
                v.CopyFrom(fields, fields.GetAllocator());
                eventData.AddMember("custom_fields", v, eventData.GetAllocator());
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
