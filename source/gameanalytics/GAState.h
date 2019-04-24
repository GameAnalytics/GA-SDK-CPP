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
            static int64_t getSessionStart();
            static int getSessionNum();
            static int getTransactionNum();
            static const std::string getSessionId();
            static const std::string getCurrentCustomDimension01();
            static const std::string getCurrentCustomDimension02();
            static const std::string getCurrentCustomDimension03();
            static const std::string getGameKey();
            static const std::string getGameSecret();
            static void setAvailableCustomDimensions01(const StringVector& dimensions);
            static void setAvailableCustomDimensions02(const StringVector& dimensions);
            static void setAvailableCustomDimensions03(const StringVector& dimensions);
            static void setAvailableResourceCurrencies(const StringVector& availableResourceCurrencies);
            static void setAvailableResourceItemTypes(const StringVector& availableResourceItemTypes);
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
            static bool hasAvailableCustomDimensions01(const char* dimension1);
            static bool hasAvailableCustomDimensions02(const char* dimension2);
            static bool hasAvailableCustomDimensions03(const char* dimension3);
            static bool hasAvailableResourceCurrency(const char* currency);
            static bool hasAvailableResourceItemType(const char* itemType);
            static void setKeys(const std::string& gameKey, const std::string& gameSecret);
            static void endSessionAndStopQueue(bool endThread);
            static void resumeSessionAndStartQueue();
            static void getEventAnnotations(rapidjson::Document& out);
            static void getSdkErrorEventAnnotations(rapidjson::Document& out);
            static void getInitAnnotations(rapidjson::Document& out);
            static void internalInitialize();
            static int64_t getClientTsAdjusted();
            static void setManualSessionHandling(bool flag);
            static bool useManualSessionHandling();
            static void setEnableErrorReporting(bool flag);
            static bool useErrorReporting();
            static void setEnabledEventSubmission(bool flag);
            static bool isEventSubmissionEnabled();
            static bool sessionIsStarted();
            static void validateAndCleanCustomFields(const rapidjson::Value& fields, rapidjson::Value& out);
            static std::string getConfigurationStringValue(const std::string& key, const std::string& defaultValue);
            static bool isCommandCenterReady();
            static void addCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener);
            static void removeCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener);
            static std::string getConfigurationsContentAsString();

         private:
            static void setDefaultUserId(const std::string& id);
            static void getSdkConfig(rapidjson::Value& out);
            static void cacheIdentifier();
            static void ensurePersistedStates();
            static void startNewSession();
            static void validateAndFixCurrentDimensions();
            static const std::string getBuild();
            static const std::string getFacebookId();
            static const std::string getGender();
            static int getBirthYear();
            static int64_t calculateServerTimeOffset(int64_t serverTs);
            static void populateConfigurations(rapidjson::Value& sdkConfig);

            std::string _userId;
            std::string _identifier;
            bool _initialized = false;
            int64_t _sessionStart = 0;
            int _sessionNum = 0;
            int _transactionNum = 0;
            std::string _sessionId;
            std::string _currentCustomDimension01;
            std::string _currentCustomDimension02;
            std::string _currentCustomDimension03;
            std::string _gameKey;
            std::string _gameSecret;
            StringVector _availableCustomDimensions01;
            StringVector _availableCustomDimensions02;
            StringVector _availableCustomDimensions03;
            StringVector _availableResourceCurrencies;
            StringVector _availableResourceItemTypes;
            std::string _build;
            std::string _facebookId;
            std::string _gender;
            int _birthYear = 0;
            bool _initAuthorized = false;
            bool _enabled = false;
            int64_t _clientServerTimeOffset = 0;
            std::string _defaultUserId;
            std::map<std::string, int> _progressionTries;
            rapidjson::Value _sdkConfigDefault;
            rapidjson::Value _sdkConfig;
            rapidjson::Value _sdkConfigCached;
            static const std::string CategorySdkError;
            bool _useManualSessionHandling = false;
            bool _enableErrorReporting = true;
            bool _enableEventSubmission = true;
            rapidjson::Document _configurations;
            bool _commandCenterIsReady;
            std::vector<std::shared_ptr<ICommandCenterListener>> _commandCenterListeners;
            std::mutex _mtx;
        };
    }
}
