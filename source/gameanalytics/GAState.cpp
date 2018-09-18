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

        Json::Int64 GAState::getSessionStart()
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

        void GAState::setAvailableCustomDimensions01(const std::vector<std::string>& availableCustomDimensions)
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

        void GAState::setAvailableCustomDimensions02(const std::vector<std::string>& availableCustomDimensions)
        {
            // Validate
            if (!validators::GAValidator::validateCustomDimensions(availableCustomDimensions))
            {
                return;
            }
            sharedInstance()->_availableCustomDimensions02 = availableCustomDimensions;

            // validate current dimension values
            validateAndFixCurrentDimensions();

            logging::GALogger::i("Set available custom01 dimension values: (" + utilities::GAUtilities::joinStringArray(availableCustomDimensions) + ")");
        }

        void GAState::setAvailableCustomDimensions03(const std::vector<std::string>& availableCustomDimensions)
        {
            // Validate
            if (!validators::GAValidator::validateCustomDimensions(availableCustomDimensions))
            {
                return;
            }
            sharedInstance()->_availableCustomDimensions03 = availableCustomDimensions;

            // validate current dimension values
            validateAndFixCurrentDimensions();

            logging::GALogger::i("Set available custom01 dimension values: (" + utilities::GAUtilities::joinStringArray(availableCustomDimensions) + ")");
        }

        void GAState::setAvailableResourceCurrencies(const std::vector<std::string>& availableResourceCurrencies)
        {
            // Validate
            if (!validators::GAValidator::validateResourceCurrencies(availableResourceCurrencies)) {
                return;
            }
            sharedInstance()->_availableResourceCurrencies = availableResourceCurrencies;

            logging::GALogger::i("Set available resource currencies: (" + utilities::GAUtilities::joinStringArray(availableResourceCurrencies) + ")");
        }

        void GAState::setAvailableResourceItemTypes(const std::vector<std::string>& availableResourceItemTypes)
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

        Json::Value GAState::getSdkConfig()
        {
            if (GAState::sharedInstance()->_sdkConfig.isObject())
            {
                return GAState::sharedInstance()->_sdkConfig;
            }
            else if (GAState::sharedInstance()->_sdkConfigCached.isObject())
            {
                return GAState::sharedInstance()->_sdkConfigCached;
            }

            return GAState::sharedInstance()->_sdkConfigDefault;
        }

        bool GAState::isEnabled()
        {
            Json::Value currentSdkConfig = GAState::getSdkConfig();

            if (currentSdkConfig.isObject() && currentSdkConfig["enabled"].isBool() && currentSdkConfig.get("enabled", true).asBool() == false)
            {
                return false;
            }
            else if (!GAState::sharedInstance()->_initAuthorized)
            {
                return false;
            }
            else
            {
                return true;
            }
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

        bool GAState::hasAvailableCustomDimensions01(const std::string& dimension1)
        {
            return utilities::GAUtilities::stringVectorContainsString(sharedInstance()->_availableCustomDimensions01, dimension1);
        }

        bool GAState::hasAvailableCustomDimensions02(const std::string& dimension2)
        {
            return utilities::GAUtilities::stringVectorContainsString(sharedInstance()->_availableCustomDimensions02, dimension2);
        }

        bool GAState::hasAvailableCustomDimensions03(const std::string& dimension3)
        {
            return utilities::GAUtilities::stringVectorContainsString(sharedInstance()->_availableCustomDimensions03, dimension3);
        }

        bool GAState::hasAvailableResourceCurrency(const std::string& currency)
        {
            return utilities::GAUtilities::stringVectorContainsString(sharedInstance()->_availableResourceCurrencies, currency);
        }

        bool GAState::hasAvailableResourceItemType(const std::string& itemType)
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

        Json::Value GAState::getEventAnnotations()
        {
            Json::Value annotations;

            // ---- REQUIRED ---- //

            // collector event API version
            annotations["v"] = 2;
            // User identifier
            annotations["user_id"] = getIdentifier();

            // Client Timestamp (the adjusted timestamp)
            annotations["client_ts"] = GAState::getClientTsAdjusted();
            // SDK version
            annotations["sdk_version"] = device::GADevice::getRelevantSdkVersion();
            // Operation system version
            annotations["os_version"] = device::GADevice::getOSVersion();
            // Device make (hardcoded to apple)
            annotations["manufacturer"] = device::GADevice::getDeviceManufacturer();
            // Device version
            annotations["device"] = device::GADevice::getDeviceModel();
            // Platform (operating system)
            annotations["platform"] = device::GADevice::getBuildPlatform();
            // Session identifier
            annotations["session_id"] = sharedInstance()->_sessionId;
            // Session number
            annotations["session_num"] = getSessionNum();

            // type of connection the user is currently on (add if valid)
            std::string connection_type = device::GADevice::getConnectionType();
            if (validators::GAValidator::validateConnectionType(connection_type))
            {
                annotations["connection_type"] = connection_type;
            }

            if (!device::GADevice::getGameEngineVersion().empty())
            {
                annotations["engine_version"] = device::GADevice::getGameEngineVersion();
            }

#if USE_UWP
            if (!device::GADevice::getAdvertisingId().empty())
            {
                annotations["uwp_aid"] = device::GADevice::getAdvertisingId();
            }
            else if (!device::GADevice::getDeviceId().empty())
            {
                annotations["uwp_id"] = device::GADevice::getDeviceId();
            }
#elif USE_TIZEN
            if (!device::GADevice::getDeviceId().empty())
            {
                annotations["tizen_id"] = device::GADevice::getDeviceId();
            }
#endif

            // ---- CONDITIONAL ---- //

            // App build version (use if not nil)
            if (!getBuild().empty())
            {
                annotations["build"] = getBuild();
            }

            // ---- OPTIONAL cross-session ---- //

            // facebook id (optional)
            if (!getFacebookId().empty())
            {
                annotations["facebook_id"] = getFacebookId();
            }
            // gender (optional)
            if (!getGender().empty())
            {
                annotations["gender"] = getGender();
            }
            // birth_year (optional)
            if (getBirthYear() != 0)
            {
                annotations["birth_year"] = getBirthYear();
            }

            return annotations;
        }

        Json::Value GAState::getSdkErrorEventAnnotations()
        {
            Json::Value annotations;

            // ---- REQUIRED ---- //

            // collector event API version
            annotations["v"] = 2;

            // Category
            annotations["category"] = GAState::CategorySdkError;
            // SDK version
            annotations["sdk_version"] = device::GADevice::getRelevantSdkVersion();
            // Operation system version
            annotations["os_version"] = device::GADevice::getOSVersion();
            // Device make (hardcoded to apple)
            annotations["manufacturer"] = device::GADevice::getDeviceManufacturer();
            // Device version
            annotations["device"] = device::GADevice::getDeviceModel();
            // Platform (operating system)
            annotations["platform"] = device::GADevice::getBuildPlatform();

            // type of connection the user is currently on (add if valid)
            std::string connection_type = device::GADevice::getConnectionType();
            if (validators::GAValidator::validateConnectionType(connection_type))
            {
                annotations["connection_type"] = connection_type;
            }

            if (!device::GADevice::getGameEngineVersion().empty())
            {
                annotations["engine_version"] = device::GADevice::getGameEngineVersion();
            }

            return annotations;
        }

        Json::Value GAState::getInitAnnotations()
        {
            Json::Value initAnnotations;
            initAnnotations["user_id"] = getIdentifier();
            // SDK version
            initAnnotations["sdk_version"] = device::GADevice::getRelevantSdkVersion();
            // Operation system version
            initAnnotations["os_version"] = device::GADevice::getOSVersion();

            // Platform (operating system)
            initAnnotations["platform"] = device::GADevice::getBuildPlatform();
            return initAnnotations;
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
            Json::Value state_dict;
            Json::Value results_ga_state = store::GAStore::executeQuerySync("SELECT * FROM ga_state;");

            if (!results_ga_state.empty()) {
                for (auto result : results_ga_state) {
                    state_dict[result["key"].asString()] = result["value"];
                }
            }

            // insert into GAState instance
            GAState *instance = GAState::sharedInstance();

            std::string defaultId = state_dict.get("default_user_id", "").asString();
            if(defaultId.empty())
            {
                instance->setDefaultUserId(utilities::GAUtilities::generateUUID());
            }
            else
            {
                instance->setDefaultUserId(defaultId);
            }

            instance->_sessionNum = utilities::GAUtilities::parseString<int>(state_dict.get("session_num", "0").asString());

            instance->_transactionNum = utilities::GAUtilities::parseString<int>(state_dict.get("transaction_num", "0").asString());

            // restore cross session user values
            if (!instance->_facebookId.empty())
            {
                store::GAStore::setState("facebook_id", instance->_facebookId);
            }
            else
            {
                instance->_facebookId = state_dict.get("facebook_id", "").asString();
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
                instance->_gender = state_dict.get("gender", "").asString();
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
                instance->_birthYear = utilities::GAUtilities::parseString<int>(state_dict.get("birth_year", "0").asString());
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
                instance->_currentCustomDimension01 = state_dict.get("dimension01", "").asString();
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
                instance->_currentCustomDimension02 = state_dict.get("dimension02", "").asString();
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
                instance->_currentCustomDimension03 = state_dict.get("dimension03", "").asString();
                if (!instance->_currentCustomDimension03.empty()) {
                    logging::GALogger::d("Dimension03 found in cache: " + instance->_currentCustomDimension03);
                }
            }

            // get cached init call values
            std::string sdkConfigCachedString = state_dict.get("sdk_config_cached", "").asString();
            if (!sdkConfigCachedString.empty()) {
                // decode JSON
                Json::Value sdkConfigCached = utilities::GAUtilities::jsonFromString(sdkConfigCachedString);
                if (!sdkConfigCached.isNull()) {
                    instance->_sdkConfigCached = sdkConfigCached;
                }
            }

            Json::Value results_ga_progression = store::GAStore::executeQuerySync("SELECT * FROM ga_progression;");

            if (!results_ga_progression.empty()) {
                for (Json::Value result : results_ga_progression) {
                    sharedInstance()->_progressionTries[result["progression"].asString()] = utilities::GAUtilities::parseString<int>(result["tries"].asString());
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
            std::pair<http::EGAHTTPApiResponse, Json::Value> pair = httpApi->requestInitReturningDict();
#endif
            Json::Value initResponseDict = pair.second;
            http::EGAHTTPApiResponse initResponse = pair.first;

            // init is ok
            if (initResponse == http::Ok && !initResponseDict.isNull()) {
                // set the time offset - how many seconds the local time is different from servertime
                Json::Int64 timeOffsetSeconds = 0;
                if (initResponseDict.get("server_ts", -1.0).asInt64() > 0) {
                    Json::Int64 serverTs = initResponseDict.get("server_ts", -1).asInt64();
                    timeOffsetSeconds = calculateServerTimeOffset(serverTs);
                }
                // insert timeOffset in received init config (so it can be used when offline)
                initResponseDict["time_offset"] = timeOffsetSeconds;

                // insert new config in sql lite cross session storage
                store::GAStore::setState("sdk_config_cached", utilities::GAUtilities::jsonToString(initResponseDict));

                // set new config and cache in memory
                GAState::sharedInstance()->_sdkConfigCached = initResponseDict;
                GAState::sharedInstance()->_sdkConfig = initResponseDict;

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
                if (GAState::sharedInstance()->_sdkConfig.isNull())
                {
                    if (!GAState::sharedInstance()->_sdkConfigCached.isNull())
                    {
                        logging::GALogger::i("Init call (session start) failed - using cached init values.");
                        // set last cross session stored config init values
                        GAState::sharedInstance()->_sdkConfig = GAState::sharedInstance()->_sdkConfigCached;
                    }
                    else
                    {
                        logging::GALogger::i("Init call (session start) failed - using default init values.");
                        // set default init values
                        GAState::sharedInstance()->_sdkConfig = GAState::sharedInstance()->_sdkConfigDefault;
                    }
                }
                else
                {
                    logging::GALogger::i("Init call (session start) failed - using cached init values.");
                }
                GAState::sharedInstance()->_initAuthorized = true;
            }

            // set offset in state (memory) from current config (config could be from cache etc.)
            GAState::sharedInstance()->_clientServerTimeOffset = utilities::GAUtilities::parseString<Json::Int64>(GAState::getSdkConfig().get("time_offset", "0.0").asString());

            // populate configurations
            populateConfigurations(GAState::getSdkConfig());

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
            return GAState::sharedInstance()->_configurations.isMember(key) ? GAState::sharedInstance()->_configurations[key].asString() : defaultValue;
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

            auto it = std::find(instance->_commandCenterListeners.begin(), instance->_commandCenterListeners.end(), listener);

            if(std::find(instance->_commandCenterListeners.begin(), instance->_commandCenterListeners.end(), listener) != GAState::sharedInstance()->_commandCenterListeners.end())
            {
                instance->_commandCenterListeners.erase(std::remove(instance->_commandCenterListeners.begin(), instance->_commandCenterListeners.end(), listener), instance->_commandCenterListeners.end());
            }
        }

        std::string GAState::getConfigurationsContentAsString()
        {
            return GAState::sharedInstance()->_configurations.toStyledString();
        }

        void GAState::populateConfigurations(Json::Value sdkConfig)
        {
            GAState::sharedInstance()->_mtx.lock();

            if(sdkConfig.isMember("configurations") && sdkConfig["configurations"].isArray())
            {
                Json::Value configurations = sdkConfig["configurations"];

                for(unsigned int i = 0; i < configurations.size(); ++i)
                {
                    Json::Value configuration = configurations[i];

                    if(!configuration.isNull())
                    {
                        std::string key = (configuration.isMember("key") && configuration["key"].isString()) ? configuration["key"].asString() : "";
                        Json::Int64 start_ts = (configuration.isMember("start") && configuration["start"].isInt64()) ? configuration["start"].asInt64() : LONG_MIN;
                        Json::Int64 end_ts = (configuration.isMember("end") && configuration["end"].isInt64()) ? configuration["start"].asInt64() : LONG_MAX;
                        Json::Int64 client_ts_adjusted = getClientTsAdjusted();

                        if(!key.empty() && configuration.isMember("value") && (configuration["value"].isString() || configuration["value"].isNumeric()) && client_ts_adjusted > start_ts && client_ts_adjusted && end_ts)
                        {
                            GAState::sharedInstance()->_configurations[key] = configuration["value"];
                            logging::GALogger::d("configuration added: " + configuration.toStyledString());
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

        const Json::Value GAState::validateAndCleanCustomFields(const Json::Value& fields)
        {
            Json::Value result;

            if (fields.isObject() && !fields.empty())
            {
                int count = 0;

                for (std::string key : fields.getMemberNames())
                {
                    if(fields[key].isNull())
                    {
                        logging::GALogger::w("validateAndCleanCustomFields: entry with key=" + key + ", value=" + fields[key].asString() +
                            " has been omitted because its key or value is null");
                    }
                    else if(count < MAX_CUSTOM_FIELDS_COUNT)
                    {
                        if(utilities::GAUtilities::stringMatch(key, "^[a-zA-Z0-9_]{1," + std::to_string(MAX_CUSTOM_FIELDS_KEY_LENGTH) + "}$"))
                        {
                            auto value = fields[key];

                            if(value.isNumeric())
                            {
                                result[key] = value;
                                ++count;
                            }
                            else if(value.isString())
                            {
                                std::string valueAsString = value.asString();

                                if(valueAsString.length() <= MAX_CUSTOM_FIELDS_VALUE_STRING_LENGTH && valueAsString.length() > 0)
                                {
                                    result[key] = value;
                                    ++count;
                                }
                                else
                                {
                                    logging::GALogger::w("validateAndCleanCustomFields: entry with key=" + key + ", value=" + fields[key].asString() +
                                        " has been omitted because its value is an empty string or exceeds the max number of characters (" + std::to_string(MAX_CUSTOM_FIELDS_VALUE_STRING_LENGTH) + ")");
                                }
                            }
                            else
                            {
                                logging::GALogger::w("validateAndCleanCustomFields: entry with key=" + key + ", value=" + fields[key].asString() +
                                    " has been omitted because its value is not a string or number");
                            }
                        }
                        else
                        {
                            logging::GALogger::w("validateAndCleanCustomFields: entry with key=" + key + ", value=" + fields[key].asString() +
                                " has been omitted because its key contains illegal character, is empty or exceeds the max number of characters (" + std::to_string(MAX_CUSTOM_FIELDS_KEY_LENGTH) + ")");
                        }
                    }
                    else
                    {
                        logging::GALogger::w("validateAndCleanCustomFields: entry with key=" + key + ", value=" + fields[key].asString() +
                            " has been omitted because it exceeds the max number of custom fields (" + std::to_string(MAX_CUSTOM_FIELDS_COUNT) + ")");
                    }
                }
            }

            return result;
        }

        Json::Int64 GAState::getClientTsAdjusted()
        {
            Json::Int64 clientTs = utilities::GAUtilities::timeIntervalSince1970();
            Json::Int64 clientTsAdjustedInteger = clientTs + GAState::sharedInstance()->_clientServerTimeOffset;

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

        Json::Int64 GAState::calculateServerTimeOffset(Json::Int64 serverTs)
        {
            Json::Int64 clientTs = utilities::GAUtilities::timeIntervalSince1970();
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
    }
}
