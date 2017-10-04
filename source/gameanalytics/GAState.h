//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <functional>
#include "Foundation/GASingleton.h"
#include <json/json.h>
#include "GameAnalytics.h"

namespace gameanalytics
{
    namespace state
    {
        // TODO(nikolaj): needed? remove.. if not
        // typedef void(*Callback) ();

        class GAState : public GASingleton<GAState>
        {
         public:
            GAState();

            static void setUserId(const std::string& id);
            static const std::string getIdentifier();
            static bool isInitialized();
            static Json::Int64 getSessionStart();
            static double getSessionNum();
            static double getTransactionNum();
            static const std::string getSessionId();
            static const std::string getCurrentCustomDimension01();
            static const std::string getCurrentCustomDimension02();
            static const std::string getCurrentCustomDimension03();
            static const std::string getGameKey();
            static const std::string getGameSecret();
            static void setAvailableCustomDimensions01(const std::vector<std::string>& dimensions);
            static void setAvailableCustomDimensions02(const std::vector<std::string>& dimensions);
            static void setAvailableCustomDimensions03(const std::vector<std::string>& dimensions);
            static void setAvailableResourceCurrencies(const std::vector<std::string>& availableResourceCurrencies);
            static void setAvailableResourceItemTypes(const std::vector<std::string>& availableResourceItemTypes);
            static void setBuild(const std::string& build);
            static bool isEnabled();
            static void setCustomDimension01(const std::string& dimension);
            static void setCustomDimension02(const std::string& dimension);
            static void setCustomDimension03(const std::string& dimension);
            static void setFacebookId(const std::string& facebookId);
            static void setGender(EGAGender gender);
            static void setBirthYear(int birthYear);
            static void incrementSessionNum();
            static void incrementTransactionNum();
            static void incrementProgressionTries(const std::string& progression);
            static int getProgressionTries(const std::string& progression);
            static void clearProgressionTries(const std::string& progression);
            static bool hasAvailableCustomDimensions01(const std::string& dimension1);
            static bool hasAvailableCustomDimensions02(const std::string& dimension2);
            static bool hasAvailableCustomDimensions03(const std::string& dimension3);
            static bool hasAvailableResourceCurrency(const std::string& currency);
            static bool hasAvailableResourceItemType(const std::string& itemType);
            static void setKeys(const std::string& gameKey, const std::string& gameSecret);
            static void endSessionAndStopQueue();
            static void resumeSessionAndStartQueue();
            static Json::Value getEventAnnotations();
            static Json::Value getSdkErrorEventAnnotations();
            static Json::Value getInitAnnotations();
            static void internalInitialize();
            static Json::Int64 getClientTsAdjusted();
            static void setManualSessionHandling(bool flag);
            static bool useManualSessionHandling();
            static bool sessionIsStarted();
            static const Json::Value validateAndCleanCustomFields(const Json::Value& fields);
            static std::string getConfigurationStringValue(const std::string& key, const std::string& defaultValue);
            static bool isCommandCenterReady();
            static void addCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener);
            static void removeCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener);
            static std::string getConfigurationsContentAsString();

         private:
            static void setDefaultUserId(const std::string& id);
            static Json::Value getSdkConfig();
            static void cacheIdentifier();
            static void ensurePersistedStates();
            static void startNewSession();
            static void validateAndFixCurrentDimensions();
            static const std::string getBuild();
            static const std::string getFacebookId();
            static const std::string getGender();
            static int getBirthYear();
            static Json::Int64 calculateServerTimeOffset(Json::Int64 serverTs);
            static void populateConfigurations(Json::Value sdkConfig);

            std::string _userId;
            std::string _identifier;
            bool _initialized;
            Json::Int64 _sessionStart;
            double _sessionNum;
            double _transactionNum;
            std::string _sessionId;
            std::string _currentCustomDimension01;
            std::string _currentCustomDimension02;
            std::string _currentCustomDimension03;
            std::string _gameKey;
            std::string _gameSecret;
            std::vector<std::string> _availableCustomDimensions01;
            std::vector<std::string> _availableCustomDimensions02;
            std::vector<std::string> _availableCustomDimensions03;
            std::vector<std::string> _availableResourceCurrencies;
            std::vector<std::string> _availableResourceItemTypes;
            std::string _build;
            std::string _facebookId;
            std::string _gender;
            int _birthYear;
            bool _initAuthorized;
            Json::Int64 _clientServerTimeOffset;
            std::string _defaultUserId;
            std::map<std::string, int> _progressionTries;
            Json::Value _sdkConfigDefault;
            Json::Value _sdkConfig;
            Json::Value _sdkConfigCached;
            static const std::string CategorySdkError;
            bool _useManualSessionHandling;
            Json::Value _configurations;
            bool _commandCenterIsReady;
            std::vector<std::shared_ptr<ICommandCenterListener>> _commandCenterListeners;
            std::mutex _mtx;
        };
    }
}
