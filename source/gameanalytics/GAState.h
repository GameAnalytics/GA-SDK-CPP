//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <vector>
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

        struct CStringCmp
        {
            bool operator()(const char* first, const char* second) const
            {
                return strcmp(first, second) < 0;
            }
        };

        class GAState : public GASingleton<GAState>
        {
         public:
            GAState();
            ~GAState();

            static void setUserId(const char* id);
            static const char* getIdentifier();
            static bool isInitialized();
            static int64_t getSessionStart();
            static int getSessionNum();
            static int getTransactionNum();
            static const char* getSessionId();
            static const char* getCurrentCustomDimension01();
            static const char* getCurrentCustomDimension02();
            static const char* getCurrentCustomDimension03();
            static const char* getGameKey();
            static const char* getGameSecret();
            static void setAvailableCustomDimensions01(const StringVector& dimensions);
            static void setAvailableCustomDimensions02(const StringVector& dimensions);
            static void setAvailableCustomDimensions03(const StringVector& dimensions);
            static void setAvailableResourceCurrencies(const StringVector& availableResourceCurrencies);
            static void setAvailableResourceItemTypes(const StringVector& availableResourceItemTypes);
            static void setBuild(const char* build);
            static bool isEnabled();
            static void setCustomDimension01(const char* dimension);
            static void setCustomDimension02(const char* dimension);
            static void setCustomDimension03(const char* dimension);
            static void setFacebookId(const char* facebookId);
            static void setGender(EGAGender gender);
            static void setBirthYear(int birthYear);
            static void incrementSessionNum();
            static void incrementTransactionNum();
            static void incrementProgressionTries(const char* progression);
            static int getProgressionTries(const char* progression);
            static void clearProgressionTries(const char* progression);
            static bool hasAvailableCustomDimensions01(const char* dimension1);
            static bool hasAvailableCustomDimensions02(const char* dimension2);
            static bool hasAvailableCustomDimensions03(const char* dimension3);
            static bool hasAvailableResourceCurrency(const char* currency);
            static bool hasAvailableResourceItemType(const char* itemType);
            static void setKeys(const char* gameKey, const char* gameSecret);
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
            static void getConfigurationStringValue(const char* key, const char* defaultValue, char* out);
            static bool isCommandCenterReady();
            static void addCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener);
            static void removeCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener);
            static void getConfigurationsContentAsString(char* out);

         private:
            static void setDefaultUserId(const char* id);
            static void getSdkConfig(rapidjson::Value& out);
            static void cacheIdentifier();
            static void ensurePersistedStates();
            static void startNewSession();
            static void validateAndFixCurrentDimensions();
            static const char* getBuild();
            static const char* getFacebookId();
            static const char* getGender();
            static int getBirthYear();
            static int64_t calculateServerTimeOffset(int64_t serverTs);
            static void populateConfigurations(rapidjson::Value& sdkConfig);

            char _userId[129] = {'\0'};
            char _identifier[129] = {'\0'};
            bool _initialized = false;
            int64_t _sessionStart = 0;
            int _sessionNum = 0;
            int _transactionNum = 0;
            char _sessionId[65] = {'\0'};
            char _currentCustomDimension01[65] = {'\0'};
            char _currentCustomDimension02[65] = {'\0'};
            char _currentCustomDimension03[65] = {'\0'};
            char _gameKey[65] = {'\0'};
            char _gameSecret[65] = {'\0'};
            StringVector _availableCustomDimensions01;
            StringVector _availableCustomDimensions02;
            StringVector _availableCustomDimensions03;
            StringVector _availableResourceCurrencies;
            StringVector _availableResourceItemTypes;
            char _build[65] = {'\0'};
            char _facebookId[65] = {'\0'};
            char _gender[9] = {'\0'};
            int _birthYear = 0;
            bool _initAuthorized = false;
            bool _enabled = false;
            int64_t _clientServerTimeOffset = 0;
            char _defaultUserId[129] = {'\0'};
            std::map<const char*, int, CStringCmp> _progressionTries;
            rapidjson::Value _sdkConfigDefault;
            rapidjson::Value _sdkConfig;
            rapidjson::Value _sdkConfigCached;
            static const char* CategorySdkError;
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
