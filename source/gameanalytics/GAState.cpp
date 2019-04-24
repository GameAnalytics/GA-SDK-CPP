//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#include "GAState.h"
#include "GAEvents.h"
#include "GAStore.h"
#include "GAUtilities.h"
#include "GAValidator.h"
#include "GAHTTPApi.h"
#include "GAThreading.h"
#include "GALogger.h"
#include "GADevice.h"
#include <utility>
#include <algorithm>
#include <climits>
#include <string.h>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#define MAX_CUSTOM_FIELDS_COUNT 50
#define MAX_CUSTOM_FIELDS_KEY_LENGTH 64
#define MAX_CUSTOM_FIELDS_VALUE_STRING_LENGTH 256

namespace gameanalytics
{
    namespace state
    {
        const std::string GAState::CategorySdkError = "sdk_error";

        GAState::GAState()
        {
        }

        GAState::~GAState()
        {
            state::GAState::endSessionAndStopQueue(false);
        }

        void GAState::setUserId(const std::string& id)
        {
            sharedInstance()->_userId = id;
            cacheIdentifier();
        }

        const std::string GAState::getIdentifier()
        {
            return GAState::sharedInstance()->_identifier;
        }

        bool GAState::isInitialized()
        {
            return GAState::sharedInstance()->_initialized;
        }

        int64_t GAState::getSessionStart()
        {
            return GAState::sharedInstance()->_sessionStart;
        }

        int GAState::getSessionNum()
        {
            return GAState::sharedInstance()->_sessionNum;
        }

        int GAState::getTransactionNum()
        {
            return GAState::sharedInstance()->_transactionNum;
        }

        const std::string GAState::getSessionId()
        {
            return sharedInstance()->_sessionId;
        }

        const std::string GAState::getCurrentCustomDimension01()
        {
            return GAState::sharedInstance()->_currentCustomDimension01;
        }

        const std::string GAState::getCurrentCustomDimension02()
        {
            return GAState::sharedInstance()->_currentCustomDimension02;
        }

        const std::string GAState::getCurrentCustomDimension03()
        {
            return GAState::sharedInstance()->_currentCustomDimension03;
        }

        void GAState::setAvailableCustomDimensions01(const StringVector& availableCustomDimensions)
        {
            // Validate
            if (!validators::GAValidator::validateCustomDimensions(availableCustomDimensions))
            {
                return;
            }
            sharedInstance()->_availableCustomDimensions01 = availableCustomDimensions;

            // validate current dimension values
            validateAndFixCurrentDimensions();

            logging::GALogger::i("Set available custom01 dimension values: (" + utilities::GAUtilities::joinStringArray(availableCustomDimensions) + ")");
        }

        void GAState::setAvailableCustomDimensions02(const StringVector& availableCustomDimensions)
        {
            // Validate
            if (!validators::GAValidator::validateCustomDimensions(availableCustomDimensions))
            {
                return;
            }
            sharedInstance()->_availableCustomDimensions02 = availableCustomDimensions;

            // validate current dimension values
            validateAndFixCurrentDimensions();

            logging::GALogger::i("Set available custom02 dimension values: (" + utilities::GAUtilities::joinStringArray(availableCustomDimensions) + ")");
        }

        void GAState::setAvailableCustomDimensions03(const StringVector& availableCustomDimensions)
        {
            // Validate
            if (!validators::GAValidator::validateCustomDimensions(availableCustomDimensions))
            {
                return;
            }
            sharedInstance()->_availableCustomDimensions03 = availableCustomDimensions;

            // validate current dimension values
            validateAndFixCurrentDimensions();

            logging::GALogger::i("Set available custom03 dimension values: (" + utilities::GAUtilities::joinStringArray(availableCustomDimensions) + ")");
        }

        void GAState::setAvailableResourceCurrencies(const StringVector& availableResourceCurrencies)
        {
            // Validate
            if (!validators::GAValidator::validateResourceCurrencies(availableResourceCurrencies)) {
                return;
            }
            sharedInstance()->_availableResourceCurrencies = availableResourceCurrencies;

            logging::GALogger::i("Set available resource currencies: (" + utilities::GAUtilities::joinStringArray(availableResourceCurrencies) + ")");
        }

        void GAState::setAvailableResourceItemTypes(const StringVector& availableResourceItemTypes)
        {
            // Validate
            if (!validators::GAValidator::validateResourceItemTypes(availableResourceItemTypes)) {
                return;
            }
            sharedInstance()->_availableResourceItemTypes = availableResourceItemTypes;

            logging::GALogger::i("Set available resource item types: (" + utilities::GAUtilities::joinStringArray(availableResourceItemTypes) + ")");
        }

        void GAState::setBuild(const std::string& build)
        {
            sharedInstance()->_build = build;

            logging::GALogger::i("Set build: " + build);
        }

        void GAState::setDefaultUserId(const std::string& id)
        {
            sharedInstance()->_defaultUserId = id;
            cacheIdentifier();
        }

        void GAState::getSdkConfig(rapidjson::Value& out)
        {
            if (GAState::sharedInstance()->_sdkConfig.IsObject())
            {
                out = GAState::sharedInstance()->_sdkConfig;
            }
            else if (GAState::sharedInstance()->_sdkConfigCached.IsObject())
            {
                out = GAState::sharedInstance()->_sdkConfigCached;
            }

            if(GAState::sharedInstance()->_sdkConfigDefault.IsNull())
            {
                GAState::sharedInstance()->_sdkConfigDefault = rapidjson::Value(rapidjson::kObjectType);
            }

            out = GAState::sharedInstance()->_sdkConfigDefault;
        }

        bool GAState::isEnabled()
        {
            return GAState::sharedInstance()->_enabled;
        }

        void GAState::setCustomDimension01(const std::string& dimension)
        {
            sharedInstance()->_currentCustomDimension01 = dimension;
            if (store::GAStore::sharedInstance()->getTableReady())
            {
                store::GAStore::setState("dimension01", dimension);
            }
            logging::GALogger::i("Set custom01 dimension value: " + dimension);
        }

        void GAState::setCustomDimension02(const std::string& dimension)
        {
            sharedInstance()->_currentCustomDimension02 = dimension;
            if (store::GAStore::sharedInstance()->getTableReady())
            {
                store::GAStore::setState("dimension02", dimension);
            }
            logging::GALogger::i("Set custom02 dimension value: " + dimension);
        }

        void GAState::setCustomDimension03(const std::string& dimension)
        {
            sharedInstance()->_currentCustomDimension03 = dimension;
            if (store::GAStore::sharedInstance()->getTableReady())
            {
                store::GAStore::setState("dimension03", dimension);
            }
            logging::GALogger::i("Set custom03 dimension value: " + dimension);
        }

        void GAState::setFacebookId(const std::string& facebookId)
        {
            sharedInstance()->_facebookId = facebookId;
            if (store::GAStore::sharedInstance()->getTableReady())
            {
                store::GAStore::setState("facebook_id", facebookId);
            }
            logging::GALogger::i("Set facebook id: " + facebookId);
        }

        void GAState::setGender(EGAGender gender)
        {
            switch (gender) {
            case Male:
                sharedInstance()->_gender = "male";
            case Female:
                sharedInstance()->_gender = "female";
            }

            if (store::GAStore::sharedInstance()->getTableReady())
            {
                store::GAStore::setState("gender", sharedInstance()->_gender);
            }
            logging::GALogger::i("Set gender: " + sharedInstance()->_gender);
        }

        void GAState::setBirthYear(int birthYear)
        {
            sharedInstance()->_birthYear = birthYear;
            if (store::GAStore::sharedInstance()->getTableReady())
            {
                store::GAStore::setState("birth_year", std::to_string(birthYear));
            }
            logging::GALogger::i("Set birth year: " + std::to_string(birthYear));
        }

        void GAState::incrementSessionNum()
        {
            auto sessionNumInt = getSessionNum() + 1;
            GAState::sharedInstance()->_sessionNum = sessionNumInt;
        }

        void GAState::incrementTransactionNum()
        {
            auto transactionNumInt = getTransactionNum() + 1;
            GAState::sharedInstance()->_transactionNum = transactionNumInt;
        }

        void GAState::incrementProgressionTries(const std::string& progression)
        {
            auto tries = static_cast<int>(getProgressionTries(progression) + 1);
            GAState::sharedInstance()->_progressionTries[progression] = tries;

            // Persist
            std::vector<std::string> parms;
            parms.push_back(progression);
            parms.push_back(std::to_string(tries));
            store::GAStore::executeQuerySync("INSERT OR REPLACE INTO ga_progression (progression, tries) VALUES(?, ?);", parms);
        }

        int GAState::getProgressionTries(const std::string& progression)
        {
            if (sharedInstance()->_progressionTries.find(progression) != sharedInstance()->_progressionTries.end())
            {
                return sharedInstance()->_progressionTries[progression];
            }
            else
            {
                return 0;
            }
        }

        void GAState::clearProgressionTries(const std::string& progression)
        {
            auto progressionTries = GAState::sharedInstance()->_progressionTries;
            auto searchResult = progressionTries.find(progression);
            if (searchResult != progressionTries.end())
            {
                progressionTries.erase(searchResult->first);
            }

            // Delete
            std::vector<std::string> parms;
            parms.push_back(progression);
            store::GAStore::executeQuerySync("DELETE FROM ga_progression WHERE progression = ?;", parms);
        }

        bool GAState::hasAvailableCustomDimensions01(const char* dimension1)
        {
            return utilities::GAUtilities::stringVectorContainsString(sharedInstance()->_availableCustomDimensions01, dimension1);
        }

        bool GAState::hasAvailableCustomDimensions02(const char* dimension2)
        {
            return utilities::GAUtilities::stringVectorContainsString(sharedInstance()->_availableCustomDimensions02, dimension2);
        }

        bool GAState::hasAvailableCustomDimensions03(const char* dimension3)
        {
            return utilities::GAUtilities::stringVectorContainsString(sharedInstance()->_availableCustomDimensions03, dimension3);
        }

        bool GAState::hasAvailableResourceCurrency(const char* currency)
        {
            return utilities::GAUtilities::stringVectorContainsString(sharedInstance()->_availableResourceCurrencies, currency);
        }

        bool GAState::hasAvailableResourceItemType(const char* itemType)
        {
            return utilities::GAUtilities::stringVectorContainsString(sharedInstance()->_availableResourceItemTypes, itemType);
        }

        void GAState::setKeys(const std::string& gameKey, const std::string& gameSecret)
        {
            GAState::sharedInstance()->_gameKey = gameKey;
            GAState::sharedInstance()->_gameSecret = gameSecret;
        }

        const std::string GAState::getGameKey()
        {
            return sharedInstance()->_gameKey;
        }

        const std::string GAState::getGameSecret()
        {
            return sharedInstance()->_gameSecret;
        }

        void GAState::internalInitialize()
        {
            // Make sure database is ready
            if (!store::GAStore::sharedInstance()->getTableReady())
            {
                return;
            }

            // Make sure persisted states are loaded
            ensurePersistedStates();
            store::GAStore::setState("default_user_id", sharedInstance()->_defaultUserId);
            sharedInstance()->_initialized = true;

            startNewSession();

            if (isEnabled())
            {
                events::GAEvents::ensureEventQueueIsRunning();
            }
        }

        void GAState::resumeSessionAndStartQueue()
        {
            if(!GAState::isInitialized())
            {
                return;
            }
            logging::GALogger::i("Resuming session.");
            if(!GAState::sessionIsStarted())
            {
                startNewSession();
            }
            events::GAEvents::ensureEventQueueIsRunning();
        }

        void GAState::endSessionAndStopQueue(bool endThread)
        {
            if(GAState::isInitialized())
            {
                logging::GALogger::i("Ending session.");
                events::GAEvents::stopEventQueue();
                if (GAState::isEnabled() && GAState::sessionIsStarted())
                {
                    events::GAEvents::addSessionEndEvent();
                    GAState::sharedInstance()->_sessionStart = 0;
                }
            }

            if(endThread)
            {
                threading::GAThreading::endThread();
            }
        }

        void GAState::getEventAnnotations(rapidjson::Document& out)
        {
            out.SetObject();
            rapidjson::Document::AllocatorType& allocator = out.GetAllocator();

            // ---- REQUIRED ---- //

            // collector event API version
            out.AddMember("v", 2, allocator);
            // User identifier

            {
                rapidjson::Value v(getIdentifier().c_str(), allocator);
                out.AddMember("user_id", v.Move(), allocator);
            }
            // Client Timestamp (the adjusted timestamp)
            out.AddMember("client_ts", GAState::getClientTsAdjusted(), allocator);
            // SDK version
            {
                rapidjson::Value v(device::GADevice::getRelevantSdkVersion(), allocator);
                out.AddMember("sdk_version", v.Move(), allocator);
            }
            // Operation system version
            {
                rapidjson::Value v(device::GADevice::getOSVersion(), allocator);
                out.AddMember("os_version", v.Move(), allocator);
            }
            // Device make (hardcoded to apple)
            {
                rapidjson::Value v(device::GADevice::getDeviceManufacturer(), allocator);
                out.AddMember("manufacturer", v.Move(), allocator);
            }
            // Device version
            {
                rapidjson::Value v(device::GADevice::getDeviceModel(), allocator);
                out.AddMember("device", v.Move(), allocator);
            }
            // Platform (operating system)
            {
                rapidjson::Value v(device::GADevice::getBuildPlatform(), allocator);
                out.AddMember("platform", v.Move(), allocator);
            }
            // Session identifier
            {
                rapidjson::Value v(sharedInstance()->_sessionId.c_str(), allocator);
                out.AddMember("session_id", v.Move(), allocator);
            }
            // Session number
            out.AddMember("session_num", getSessionNum(), allocator);

            // type of connection the user is currently on (add if valid)
            std::string connection_type = device::GADevice::getConnectionType();
            if (validators::GAValidator::validateConnectionType(connection_type))
            {
                rapidjson::Value v(connection_type.c_str(), allocator);
                out.AddMember("connection_type", v.Move(), allocator);
            }

            if(strlen(device::GADevice::getGameEngineVersion()) > 0)
            {
                rapidjson::Value v(device::GADevice::getGameEngineVersion(), allocator);
                out.AddMember("engine_version", v.Move(), allocator);
            }

#if USE_UWP
            if (!device::GADevice::getAdvertisingId().empty())
            {
                out.AddMember("uwp_aid", device::GADevice::getAdvertisingId(), allocator);
            }
            else if (!device::GADevice::getDeviceId().empty())
            {
                out.AddMember("uwp_id", device::GADevice::getDeviceId(), allocator);
            }
#elif USE_TIZEN
            if (!device::GADevice::getDeviceId().empty())
            {
                out.AddMember("tizen_id", device::GADevice::getDeviceId(), allocator);
            }
#endif

            // ---- CONDITIONAL ---- //

            // App build version (use if not nil)
            if (!getBuild().empty())
            {
                rapidjson::Value v(getBuild().c_str(), allocator);
                out.AddMember("build", v.Move(), allocator);
            }

            // ---- OPTIONAL cross-session ---- //

            // facebook id (optional)
            if (!getFacebookId().empty())
            {
                rapidjson::Value v(getFacebookId().c_str(), allocator);
                out.AddMember("facebook_id", v.Move(), allocator);
            }
            // gender (optional)
            if (!getGender().empty())
            {
                rapidjson::Value v(getGender().c_str(), allocator);
                out.AddMember("gender", v.Move(), allocator);
            }
            // birth_year (optional)
            if (getBirthYear() != 0)
            {
                out.AddMember("birth_year", getBirthYear(), allocator);
            }
        }

        void GAState::getSdkErrorEventAnnotations(rapidjson::Document& out)
        {
            out.SetObject();
            rapidjson::Document::AllocatorType& allocator = out.GetAllocator();

            // ---- REQUIRED ---- //

            // collector event API version
            out.AddMember("v", 2, allocator);

            // Category
            {
                rapidjson::Value v(GAState::CategorySdkError.c_str(), allocator);
                out.AddMember("category", v.Move(), allocator);
            }
            // SDK version
            {
                rapidjson::Value v(device::GADevice::getRelevantSdkVersion(), allocator);
                out.AddMember("sdk_version", v.Move(), allocator);
            }
            // Operation system version
            {
                rapidjson::Value v(device::GADevice::getOSVersion(), allocator);
                out.AddMember("os_version", v.Move(), allocator);
            }
            // Device make (hardcoded to apple)
            {
                rapidjson::Value v(device::GADevice::getDeviceManufacturer(), allocator);
                out.AddMember("manufacturer", v.Move(), allocator);
            }
            // Device version
            {
                rapidjson::Value v(device::GADevice::getDeviceModel(), allocator);
                out.AddMember("device", v.Move(), allocator);
            }
            // Platform (operating system)
            {
                rapidjson::Value v(device::GADevice::getBuildPlatform(), allocator);
                out.AddMember("platform", v.Move(), allocator);
            }

            // type of connection the user is currently on (add if valid)
            std::string connection_type = device::GADevice::getConnectionType();
            if (validators::GAValidator::validateConnectionType(connection_type))
            {
                rapidjson::Value v(connection_type.c_str(), allocator);
                out.AddMember("connection_type", v.Move(), allocator);
            }

            if(strlen(device::GADevice::getGameEngineVersion()) > 0)
            {
                rapidjson::Value v(device::GADevice::getGameEngineVersion(), allocator);
                out.AddMember("engine_version", v.Move(), allocator);
            }
        }

        void GAState::getInitAnnotations(rapidjson::Document& out)
        {
            out.SetObject();
            rapidjson::Document::AllocatorType& allocator = out.GetAllocator();

            {
                rapidjson::Value v(getIdentifier().c_str(), allocator);
                out.AddMember("user_id", v.Move(), allocator);
            }
            // SDK version
            {
                rapidjson::Value v(device::GADevice::getRelevantSdkVersion(), allocator);
                out.AddMember("sdk_version", v.Move(), allocator);
            }
            // Operation system version
            {
                rapidjson::Value v(device::GADevice::getOSVersion(), allocator);
                out.AddMember("os_version", v.Move(), allocator);
            }

            // Platform (operating system)
            {
                rapidjson::Value v(device::GADevice::getBuildPlatform(), allocator);
                out.AddMember("platform", v.Move(), allocator);
            }
        }

        void GAState::cacheIdentifier()
        {
            if (!GAState::sharedInstance()->_userId.empty())
            {
                GAState::sharedInstance()->_identifier = GAState::sharedInstance()->_userId;
            }
#if USE_UWP
            else if (!device::GADevice::getAdvertisingId().empty())
            {
                GAState::sharedInstance()->_identifier = device::GADevice::getAdvertisingId();
            }
            else if (!device::GADevice::getDeviceId().empty())
            {
                GAState::sharedInstance()->_identifier = device::GADevice::getDeviceId();
            }
#elif USE_TIZEN
            else if (!device::GADevice::getDeviceId().empty())
            {
                GAState::sharedInstance()->_identifier = device::GADevice::getDeviceId();
            }
#endif
            else if (!GAState::sharedInstance()->_defaultUserId.empty())
            {
                GAState::sharedInstance()->_identifier = GAState::sharedInstance()->_defaultUserId;
            }

            logging::GALogger::d("identifier, {clean:" + GAState::sharedInstance()->_identifier + "}");
        }

        void GAState::ensurePersistedStates()
        {
            // get and extract stored states
            rapidjson::Document state_dict;
            state_dict.SetObject();
            rapidjson::Document::AllocatorType& allocator = state_dict.GetAllocator();
            rapidjson::Value results_ga_state(rapidjson::kArrayType);
            store::GAStore::executeQuerySync("SELECT * FROM ga_state;", results_ga_state);

            if (!results_ga_state.IsNull() && !results_ga_state.Empty())
            {
                for (rapidjson::Value::ConstValueIterator itr = results_ga_state.Begin(); itr != results_ga_state.End(); ++itr)
                {
                    if(itr->HasMember("key") && itr->HasMember("value"))
                    {
                        rapidjson::Value v((*itr)["key"].GetString(), allocator);
                        rapidjson::Value v1((*itr)["value"].GetString(), allocator);
                        state_dict.AddMember(v.Move(), v1.Move(), allocator);
                    }
                }
            }

            // insert into GAState instance
            GAState *instance = GAState::sharedInstance();

            std::string defaultId = state_dict.HasMember("default_user_id") ? state_dict["default_user_id"].GetString() : "";
            if(defaultId.empty())
            {
                instance->setDefaultUserId(utilities::GAUtilities::generateUUID());
            }
            else
            {
                instance->setDefaultUserId(defaultId);
            }

            instance->_sessionNum = utilities::GAUtilities::parseString<int>(state_dict.HasMember("session_num") ? state_dict["session_num"].GetString() : "0");

            instance->_transactionNum = utilities::GAUtilities::parseString<int>(state_dict.HasMember("transaction_num") ? state_dict["transaction_num"].GetString() : "0");

            // restore cross session user values
            if (!instance->_facebookId.empty())
            {
                store::GAStore::setState("facebook_id", instance->_facebookId);
            }
            else
            {
                instance->_facebookId = state_dict.HasMember("facebook_id") ? state_dict["facebook_id"].GetString() : "";
                if (!instance->_facebookId.empty()) {
                    logging::GALogger::d("facebookid found in DB: " + instance->_facebookId);
                }
            }

            if (!instance->_gender.empty())
            {
                store::GAStore::setState("gender", instance->_gender);
            }
            else
            {
                instance->_gender = state_dict.HasMember("gender") ? state_dict["gender"].GetString() : "";
                if (!instance->_gender.empty()) {
                    logging::GALogger::d("gender found in DB: " + instance->_gender);
                }
            }

            if (instance->_birthYear != 0)
            {
                store::GAStore::setState("birth_year", std::to_string(instance->_birthYear));
            }
            else
            {
                instance->_birthYear = utilities::GAUtilities::parseString<int>(state_dict.HasMember("birth_year") ? state_dict["birth_year"].GetString() : "0");
                if (instance->_birthYear != 0) {
                    logging::GALogger::d("birthYear found in DB: " + std::to_string(instance->_birthYear));
                }
            }

            // restore dimension settings
            if (!instance->_currentCustomDimension01.empty())
            {
                store::GAStore::setState("dimension01", instance->_currentCustomDimension01);
            }
            else
            {
                instance->_currentCustomDimension01 = state_dict.HasMember("dimension01") ? state_dict["dimension01"].GetString() : "";
                if (!instance->_currentCustomDimension01.empty()) {
                    logging::GALogger::d("Dimension01 found in cache: " + instance->_currentCustomDimension01);
                }
            }

            if (!instance->_currentCustomDimension02.empty())
            {
                store::GAStore::setState("dimension02", instance->_currentCustomDimension02);
            }
            else
            {
                instance->_currentCustomDimension02 = state_dict.HasMember("dimension02") ? state_dict["dimension02"].GetString() : "";
                if (!instance->_currentCustomDimension02.empty()) {
                    logging::GALogger::d("Dimension02 found cache: " + instance->_currentCustomDimension02);
                }
            }

            if (!instance->_currentCustomDimension03.empty())
            {
                store::GAStore::setState("dimension03", instance->_currentCustomDimension03);
            }
            else
            {
                instance->_currentCustomDimension03 = state_dict.HasMember("dimension03") ? state_dict["dimension03"].GetString() : "";
                if (!instance->_currentCustomDimension03.empty()) {
                    logging::GALogger::d("Dimension03 found in cache: " + instance->_currentCustomDimension03);
                }
            }

            // get cached init call values
            std::string sdkConfigCachedString = state_dict.HasMember("sdk_config_cached") ? state_dict["sdk_config_cached"].GetString() : "";
            if (!sdkConfigCachedString.empty())
            {
                // decode JSON
                rapidjson::Document d;
                d.Parse(sdkConfigCachedString.c_str());
                if (!d.IsNull())
                {
                    instance->_sdkConfigCached = d.GetObject();
                }
            }

            rapidjson::Value results_ga_progression(rapidjson::kArrayType);
            store::GAStore::executeQuerySync("SELECT * FROM ga_progression;", results_ga_progression);

            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                results_ga_progression.Accept(writer);
            }
            logging::GALogger::d(buffer.GetString());

            if (!results_ga_progression.IsNull() && !results_ga_progression.Empty())
            {
                for (rapidjson::Value::ConstValueIterator itr = results_ga_progression.Begin(); itr != results_ga_progression.End(); ++itr)
                {
                    sharedInstance()->_progressionTries[(*itr)["progression"].GetString()] = utilities::GAUtilities::parseString<int>((*itr).HasMember("tries") ? (*itr)["tries"].GetString() : "0");
                }
            }

            return;
        }

        void GAState::startNewSession()
        {
            logging::GALogger::i("Starting a new session.");

            // make sure the current custom dimensions are valid
            GAState::validateAndFixCurrentDimensions();

            // call the init call
            http::GAHTTPApi *httpApi = http::GAHTTPApi::sharedInstance();
            rapidjson::Document initResponseDict;
            initResponseDict.SetObject();
            rapidjson::Document::AllocatorType& allocator = initResponseDict.GetAllocator();
            http::EGAHTTPApiResponse initResponse;
#if USE_UWP
            std::pair<http::EGAHTTPApiResponse, Json::Value> pair;
            try
            {
                pair = httpApi->requestInitReturningDict().get();
            }
            catch(Platform::COMException^ e)
            {
                pair = std::pair<http::EGAHTTPApiResponse, Json::Value>(http::NoResponse, Json::Value());
            }
#else

            httpApi->requestInitReturningDict(initResponse, initResponseDict);
#endif

            // init is ok
            if (initResponse == http::Ok && !initResponseDict.IsNull())
            {
                // set the time offset - how many seconds the local time is different from servertime
                int64_t timeOffsetSeconds = 0;
                int64_t server_ts = initResponseDict.HasMember("server_ts") ? initResponseDict["server_ts"].GetInt64() : -1;
                if (server_ts > 0)
                {
                    timeOffsetSeconds = calculateServerTimeOffset(server_ts);
                }
                // insert timeOffset in received init config (so it can be used when offline)
                initResponseDict.AddMember("time_offset", timeOffsetSeconds, allocator);

                rapidjson::StringBuffer buffer;
                {
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    initResponseDict.Accept(writer);
                }

                // insert new config in sql lite cross session storage
                store::GAStore::setState("sdk_config_cached", buffer.GetString());

                // set new config and cache in memory
                GAState::sharedInstance()->_sdkConfigCached.CopyFrom(initResponseDict, allocator);
                GAState::sharedInstance()->_sdkConfig.CopyFrom(initResponseDict, allocator);

                GAState::sharedInstance()->_initAuthorized = true;
            }
            else if (initResponse == http::Unauthorized) {
                logging::GALogger::w("Initialize SDK failed - Unauthorized");
                GAState::sharedInstance()->_initAuthorized = false;
            }
            else
            {
                // log the status if no connection
                if (initResponse == http::NoResponse || initResponse == http::RequestTimeout)
                {
                    logging::GALogger::i("Init call (session start) failed - no response. Could be offline or timeout.");
                }
                else if (initResponse == http::BadResponse || initResponse == http::JsonEncodeFailed || initResponse == http::JsonDecodeFailed)
                {
                    logging::GALogger::i("Init call (session start) failed - bad response. Could be bad response from proxy or GA servers.");
                }
                else if (initResponse == http::BadRequest || initResponse == http::UnknownResponseCode)
                {
                    logging::GALogger::i("Init call (session start) failed - bad request or unknown response.");
                }

                // init call failed (perhaps offline)
                if (GAState::sharedInstance()->_sdkConfig.IsNull())
                {
                    if (!GAState::sharedInstance()->_sdkConfigCached.IsNull())
                    {
                        logging::GALogger::i("Init call (session start) failed - using cached init values.");
                        // set last cross session stored config init values
                        GAState::sharedInstance()->_sdkConfig = GAState::sharedInstance()->_sdkConfigCached;
                    }
                    else
                    {
                        logging::GALogger::i("Init call (session start) failed - using default init values.");
                        // set default init values

                        if(GAState::sharedInstance()->_sdkConfigDefault.IsNull())
                        {
                            GAState::sharedInstance()->_sdkConfigDefault = rapidjson::Value(rapidjson::kObjectType);
                        }

                        GAState::sharedInstance()->_sdkConfig = GAState::sharedInstance()->_sdkConfigDefault;
                    }
                }
                else
                {
                    logging::GALogger::i("Init call (session start) failed - using cached init values.");
                }
                GAState::sharedInstance()->_initAuthorized = true;
            }

            rapidjson::Value currentSdkConfig(rapidjson::kObjectType);
            GAState::getSdkConfig(currentSdkConfig);
            {
                if (currentSdkConfig.IsObject() && ((currentSdkConfig.HasMember("enabled") && currentSdkConfig["enabled"].IsBool()) ? currentSdkConfig["enabled"].GetBool() : true) == false)
                {
                    GAState::sharedInstance()->_enabled = false;
                }
                else if (!GAState::sharedInstance()->_initAuthorized)
                {
                    GAState::sharedInstance()->_enabled = false;
                }
                else
                {
                    GAState::sharedInstance()->_enabled = true;
                }
            }

            // set offset in state (memory) from current config (config could be from cache etc.)

            GAState::sharedInstance()->_clientServerTimeOffset = utilities::GAUtilities::parseString<int64_t>(currentSdkConfig.HasMember("time_offset") ? currentSdkConfig["time_offset"].GetString() : "0.0");

            // populate configurations
            populateConfigurations(currentSdkConfig);

            // if SDK is disabled in config
            if (!GAState::isEnabled())
            {
                logging::GALogger::w("Could not start session: SDK is disabled.");
                // stop event queue
                // + make sure it's able to restart if another session detects it's enabled again
                events::GAEvents::stopEventQueue();
                return;
            }
            else
            {
                events::GAEvents::ensureEventQueueIsRunning();
            }

            // generate the new session
            std::string newSessionId = utilities::GAUtilities::generateUUID();
            std::string newSessionIdLowercase = utilities::GAUtilities::lowercaseString(newSessionId);

            // Set session id
            GAState::sharedInstance()->_sessionId = newSessionIdLowercase;

            // Set session start
            GAState::sharedInstance()->_sessionStart = getClientTsAdjusted();

            // Add session start event
            events::GAEvents::addSessionStartEvent();
        }

        void GAState::validateAndFixCurrentDimensions()
        {
            // validate that there are no current dimension01 not in list
            if (!validators::GAValidator::validateDimension01(sharedInstance()->_currentCustomDimension01))
            {
                logging::GALogger::d("Invalid dimension01 found in variable. Setting to nil. Invalid dimension: " + sharedInstance()->_currentCustomDimension01);
                setCustomDimension01("");
            }
            // validate that there are no current dimension02 not in list
            if (!validators::GAValidator::validateDimension02(sharedInstance()->_currentCustomDimension02))
            {
                logging::GALogger::d("Invalid dimension02 found in variable. Setting to nil. Invalid dimension: " + sharedInstance()->_currentCustomDimension02);
                setCustomDimension02("");
            }
            // validate that there are no current dimension03 not in list
            if (!validators::GAValidator::validateDimension03(sharedInstance()->_currentCustomDimension03))
            {
                logging::GALogger::d("Invalid dimension03 found in variable. Setting to nil. Invalid dimension: " + sharedInstance()->_currentCustomDimension03);
                setCustomDimension03("");
            }
        }

        bool GAState::sessionIsStarted()
        {
            return GAState::sharedInstance()->_sessionStart != 0;
        }

        std::string GAState::getConfigurationStringValue(const std::string& key, const std::string& defaultValue)
        {
            std::lock_guard<std::mutex> lg(GAState::sharedInstance()->_mtx);
            return GAState::sharedInstance()->_configurations.HasMember(key.c_str()) ? GAState::sharedInstance()->_configurations[key.c_str()].GetString() : defaultValue;
        }

        bool GAState::isCommandCenterReady()
        {
            return GAState::sharedInstance()->_commandCenterIsReady;
        }

        void GAState::addCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener)
        {
            GAState* instance = GAState::sharedInstance();

            if(std::find(instance->_commandCenterListeners.begin(), instance->_commandCenterListeners.end(), listener) == instance->_commandCenterListeners.end())
            {
                instance->_commandCenterListeners.push_back(listener);
            }
        }

        void GAState::removeCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener)
        {
            GAState* instance = GAState::sharedInstance();

            if(std::find(instance->_commandCenterListeners.begin(), instance->_commandCenterListeners.end(), listener) != GAState::sharedInstance()->_commandCenterListeners.end())
            {
                instance->_commandCenterListeners.erase(std::remove(instance->_commandCenterListeners.begin(), instance->_commandCenterListeners.end(), listener), instance->_commandCenterListeners.end());
            }
        }

        std::string GAState::getConfigurationsContentAsString()
        {
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            GAState::sharedInstance()->_configurations.Accept(writer);
            return buffer.GetString();
        }

        void GAState::populateConfigurations(rapidjson::Value& sdkConfig)
        {
            GAState::sharedInstance()->_mtx.lock();

            GAState::sharedInstance()->_configurations.SetObject();
            rapidjson::Document::AllocatorType& allocator = GAState::sharedInstance()->_configurations.GetAllocator();

            if(sdkConfig.HasMember("configurations") && sdkConfig["configurations"].IsArray())
            {
                rapidjson::Value& configurations = sdkConfig["configurations"];

                for (rapidjson::Value::ConstValueIterator itr = configurations.Begin(); itr != configurations.End(); ++itr)
                {
                    const rapidjson::Value& configuration = *itr;

                    if(!configuration.IsNull())
                    {
                        std::string key = (configuration.HasMember("key") && configuration["key"].IsString()) ? configuration["key"].GetString() : "";
                        int64_t start_ts = (configuration.HasMember("start") && configuration["start"].IsInt64()) ? configuration["start"].GetInt64() : LONG_MIN;
                        int64_t end_ts = (configuration.HasMember("end") && configuration["end"].IsInt64()) ? configuration["start"].GetInt64() : LONG_MAX;
                        int64_t client_ts_adjusted = getClientTsAdjusted();

                        if(!key.empty() && configuration.HasMember("value") && client_ts_adjusted > start_ts && client_ts_adjusted < end_ts)
                        {
                            if(configuration["value"].IsString())
                            {
                                rapidjson::Value v(key.c_str(), allocator);
                                rapidjson::Value v1(configuration["value"].GetString(), allocator);
                                GAState::sharedInstance()->_configurations.AddMember(v.Move(), v1.Move(), allocator);
                            }
                            else if(configuration["value"].IsNumber())
                            {
                                rapidjson::Value v(key.c_str(), allocator);
                                GAState::sharedInstance()->_configurations.AddMember(v.Move(), configuration["value"].GetDouble(), allocator);
                            }

                            rapidjson::StringBuffer buffer;
                            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
                            configuration.Accept(writer);
                            logging::GALogger::d("configuration added: " + std::string(buffer.GetString()));
                        }
                    }
                }
            }

            GAState::sharedInstance()->_commandCenterIsReady = true;
            for(auto& listener : GAState::sharedInstance()->_commandCenterListeners)
            {
                listener->onCommandCenterUpdated();
            }

            GAState::sharedInstance()->_mtx.unlock();
        }

        void GAState::validateAndCleanCustomFields(const rapidjson::Value& fields, rapidjson::Value& out)
        {
            rapidjson::Document result;
            result.SetObject();
            rapidjson::Document::AllocatorType& allocator = result.GetAllocator();

            if (fields.IsObject() && !fields.Empty())
            {
                int count = 0;

                for (rapidjson::Value::ConstMemberIterator itr = fields.MemberBegin(); itr != fields.MemberEnd(); ++itr)
                {
                    const char* key = itr->name.GetString();
                    if(fields[key].IsNull())
                    {
                        logging::GALogger::w("validateAndCleanCustomFields: entry with key=" + std::string(key) + ", value=" + std::string(fields[key].GetString()) +
                            " has been omitted because its key or value is null");
                    }
                    else if(count < MAX_CUSTOM_FIELDS_COUNT)
                    {
                        if(utilities::GAUtilities::stringMatch(key, "^[a-zA-Z0-9_]{1," + std::to_string(MAX_CUSTOM_FIELDS_KEY_LENGTH) + "}$"))
                        {
                            const rapidjson::Value& value = fields[key];

                            if(value.IsNumber())
                            {
                                rapidjson::Value v(key, allocator);
                                result.AddMember(v.Move(), value.GetDouble(), allocator);
                                ++count;
                            }
                            else if(value.IsString())
                            {
                                std::string valueAsString = value.GetString();

                                if(valueAsString.length() <= MAX_CUSTOM_FIELDS_VALUE_STRING_LENGTH && valueAsString.length() > 0)
                                {
                                    rapidjson::Value v(key, allocator);
                                    rapidjson::Value v1(value.GetString(), allocator);
                                    result.AddMember(v.Move(), v1.Move(), allocator);
                                    ++count;
                                }
                                else
                                {
                                    logging::GALogger::w("validateAndCleanCustomFields: entry with key=" + std::string(key) + ", value=" + std::string(fields[key].GetString()) +
                                        " has been omitted because its value is an empty string or exceeds the max number of characters (" + std::to_string(MAX_CUSTOM_FIELDS_VALUE_STRING_LENGTH) + ")");
                                }
                            }
                            else
                            {
                                logging::GALogger::w("validateAndCleanCustomFields: entry with key=" + std::string(key) + ", value=" + std::string(fields[key].GetString()) +
                                    " has been omitted because its value is not a string or number");
                            }
                        }
                        else
                        {
                            logging::GALogger::w("validateAndCleanCustomFields: entry with key=" + std::string(key) + ", value=" + std::string(fields[key].GetString()) +
                                " has been omitted because its key contains illegal character, is empty or exceeds the max number of characters (" + std::to_string(MAX_CUSTOM_FIELDS_KEY_LENGTH) + ")");
                        }
                    }
                    else
                    {
                        logging::GALogger::w("validateAndCleanCustomFields: entry with key=" + std::string(key) + ", value=" + std::string(fields[key].GetString()) +
                            " has been omitted because it exceeds the max number of custom fields (" + std::to_string(MAX_CUSTOM_FIELDS_COUNT) + ")");
                    }
                }
            }

            out.CopyFrom(result, allocator);
        }

        int64_t GAState::getClientTsAdjusted()
        {
            int64_t clientTs = utilities::GAUtilities::timeIntervalSince1970();
            int64_t clientTsAdjustedInteger = clientTs + GAState::sharedInstance()->_clientServerTimeOffset;

            if (validators::GAValidator::validateClientTs(clientTsAdjustedInteger))
            {
                return clientTsAdjustedInteger;
            }
            else
            {
                return clientTs;
            }
        }

        const std::string GAState::getBuild()
        {
            return GAState::sharedInstance()->_build;
        }

        const std::string GAState::getFacebookId()
        {
            return sharedInstance()->_facebookId;
        }

        const std::string GAState::getGender()
        {
            return sharedInstance()->_gender;
        }

        int GAState::getBirthYear()
        {
            return sharedInstance()->_birthYear;
        }

        int64_t GAState::calculateServerTimeOffset(int64_t serverTs)
        {
            int64_t clientTs = utilities::GAUtilities::timeIntervalSince1970();
            return serverTs - clientTs;
        }

        void GAState::setManualSessionHandling(bool flag)
        {
            sharedInstance()->_useManualSessionHandling = flag;
            if(flag)
            {
                logging::GALogger::i("Use manual session handling: true");
            }
            else
            {
                logging::GALogger::i("Use manual session handling: false");
            }
        }

        bool GAState::useManualSessionHandling()
        {
            return sharedInstance()->_useManualSessionHandling;
        }

        void GAState::setEnableErrorReporting(bool flag)
        {
            sharedInstance()->_enableErrorReporting = flag;
            if(flag)
            {
                logging::GALogger::i("Use error reporting: true");
            }
            else
            {
                logging::GALogger::i("Use error reporting: false");
            }
        }

        bool GAState::useErrorReporting()
        {
            return sharedInstance()->_enableErrorReporting;
        }

        void GAState::setEnabledEventSubmission(bool flag)
        {
            sharedInstance()->_enableEventSubmission = flag;
        }

        bool GAState::isEventSubmissionEnabled()
        {
            return sharedInstance()->_enableEventSubmission;
        }
    }
}
