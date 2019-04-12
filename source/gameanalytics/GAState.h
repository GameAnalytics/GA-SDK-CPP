//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <functional>
#include "Foundation/GASingleton.h"
#include "rapidjson/document.h"
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
            ~GAState();

            static void setUserId(const std::string& id);
            static const std::string getIdentifier();
            static bool isInitialized();
            static long getSessionStart();
            static int getSessionNum();
            static int getTransactionNum();
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
            static void endSessionAndStopQueue(bool endThread);
            static void resumeSessionAndStartQueue();
            static void getEventAnnotations(rapidjson::Value& out);
            static void getSdkErrorEventAnnotations(rapidjson::Value& out);
            static void getInitAnnotations(rapidjson::Value& out);
            static void internalInitialize();
            static long getClientTsAdjusted();
            static void setManualSessionHandling(bool flag);
            static bool useManualSessionHandling();
            static void setEnableErrorReporting(bool flag);
            static bool useErrorReporting();
            static void setEnabledEventSubmission(bool flag);
            static bool isEventSubmissionEnabled();
            static bool sessionIsStarted();
            static const rapidjson::Value& validateAndCleanCustomFields(const rapidjson::Value& fields);
            static std::string getConfigurationStringValue(const std::string& key, const std::string& defaultValue);
            static bool isCommandCenterReady();
            static void addCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener);
            static void removeCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener);
            static std::string getConfigurationsContentAsString();

         private:
            static void setDefaultUserId(const std::string& id);
            static const rapidjson::Value& getSdkConfig();
            static void cacheIdentifier();
            static void ensurePersistedStates();
            static void startNewSession();
            static void validateAndFixCurrentDimensions();
            static const std::string getBuild();
            static const std::string getFacebookId();
            static const std::string getGender();
            static int getBirthYear();
            static long calculateServerTimeOffset(long serverTs);
            static void populateConfigurations(rapidjson::Value& sdkConfig);

            std::string _userId;
            std::string _identifier;
            bool _initialized = false;
            long _sessionStart = 0;
            int _sessionNum = 0;
            int _transactionNum = 0;
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
            int _birthYear = 0;
            bool _initAuthorized = false;
            bool _enabled = false;
            long _clientServerTimeOffset = 0;
            std::string _defaultUserId;
            std::map<std::string, int> _progressionTries;
            rapidjson::Value _sdkConfigDefault;
            rapidjson::Value _sdkConfig;
            rapidjson::Value _sdkConfigCached;
            static const std::string CategorySdkError;
            bool _useManualSessionHandling = false;
            bool _enableErrorReporting = true;
            bool _enableEventSubmission = true;
            rapidjson::Value _configurations;
            bool _commandCenterIsReady;
            std::vector<std::shared_ptr<ICommandCenterListener>> _commandCenterListeners;
            std::mutex _mtx;
        };
    }
}
