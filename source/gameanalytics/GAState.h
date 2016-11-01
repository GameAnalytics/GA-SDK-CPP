//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include <map>
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

            static void setUserId(const STRING_TYPE& id);
            static const STRING_TYPE getIdentifier();
            static bool isInitialized();
            static Json::Int64 getSessionStart();
            static double getSessionNum();
            static double getTransactionNum();
            static const STRING_TYPE getSessionId();
            static const STRING_TYPE getCurrentCustomDimension01();
            static const STRING_TYPE getCurrentCustomDimension02();
            static const STRING_TYPE getCurrentCustomDimension03();
            static const STRING_TYPE getGameKey();
            static const STRING_TYPE getGameSecret();
            static void setAvailableCustomDimensions01(const std::vector<STRING_TYPE>& dimensions);
            static void setAvailableCustomDimensions02(const std::vector<STRING_TYPE>& dimensions);
            static void setAvailableCustomDimensions03(const std::vector<STRING_TYPE>& dimensions);
            static void setAvailableResourceCurrencies(const std::vector<STRING_TYPE>& availableResourceCurrencies);
            static void setAvailableResourceItemTypes(const std::vector<STRING_TYPE>& availableResourceItemTypes);
            static void setBuild(const STRING_TYPE& build);
            static bool isEnabled();
            static void setCustomDimension01(const STRING_TYPE& dimension);
            static void setCustomDimension02(const STRING_TYPE& dimension);
            static void setCustomDimension03(const STRING_TYPE& dimension);
            static void setFacebookId(const STRING_TYPE& facebookId);
            static void setGender(EGAGender gender);
            static void setBirthYear(int birthYear);
            static void incrementSessionNum();
            static void incrementTransactionNum();
            static void incrementProgressionTries(const STRING_TYPE& progression);
            static int getProgressionTries(const STRING_TYPE& progression);
            static void clearProgressionTries(const STRING_TYPE& progression);
            static bool hasAvailableCustomDimensions01(const STRING_TYPE& dimension1);
            static bool hasAvailableCustomDimensions02(const STRING_TYPE& dimension2);
            static bool hasAvailableCustomDimensions03(const STRING_TYPE& dimension3);
            static bool hasAvailableResourceCurrency(const STRING_TYPE& currency);
            static bool hasAvailableResourceItemType(const STRING_TYPE& itemType);
            static void setKeys(const STRING_TYPE& gameKey, const STRING_TYPE& gameSecret);
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

         private:
            static void setDefaultUserId(const STRING_TYPE& id);
            static Json::Value getSdkConfig();
            static void cacheIdentifier();
            static void ensurePersistedStates();
            static void startNewSession();
            static void validateAndFixCurrentDimensions();
            static const STRING_TYPE getBuild();
            static const STRING_TYPE getFacebookId();
            static const STRING_TYPE getGender();
            static int getBirthYear();
            static Json::Int64 calculateServerTimeOffset(Json::Int64 serverTs);

            STRING_TYPE _userId;
            STRING_TYPE _identifier;
            bool _initialized;
            Json::Int64 _sessionStart;
            double _sessionNum;
            double _transactionNum;
            STRING_TYPE _sessionId;
            STRING_TYPE _currentCustomDimension01;
            STRING_TYPE _currentCustomDimension02;
            STRING_TYPE _currentCustomDimension03;
            STRING_TYPE _gameKey;
            STRING_TYPE _gameSecret;
            std::vector<STRING_TYPE> _availableCustomDimensions01;
            std::vector<STRING_TYPE> _availableCustomDimensions02;
            std::vector<STRING_TYPE> _availableCustomDimensions03;
            std::vector<STRING_TYPE> _availableResourceCurrencies;
            std::vector<STRING_TYPE> _availableResourceItemTypes;
            STRING_TYPE _build;
            STRING_TYPE _facebookId;
            STRING_TYPE _gender;
            int _birthYear;
            bool _initAuthorized;
            Json::Int64 _clientServerTimeOffset;
            STRING_TYPE _defaultUserId;
            std::map<STRING_TYPE, int> _progressionTries;
            Json::Value _sdkConfigDefault;
            Json::Value _sdkConfig;
            Json::Value _sdkConfigCached;
            static const STRING_TYPE CategorySdkError;
            bool _useManualSessionHandling;
        };
    }
} 
