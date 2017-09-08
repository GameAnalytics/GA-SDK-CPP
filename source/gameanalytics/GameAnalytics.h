//
// GA-SDK-CPP
// Copyright 2015 CppWrapper. All rights reserved.
//

#pragma once

#if GA_SHARED_LIB && defined(_WIN32)
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "gdi32.lib")
#endif

#include <string>
#include <vector>
#include <json/json.h>
#if USE_TIZEN || GA_SHARED_LIB
#include "GameAnalyticsExtern.h"
#endif

namespace gameanalytics
{

    /*!
     @enum
     @discussion
     This enum is used to specify flow in resource events
     @constant GAResourceFlowTypeSource
     Used when adding to a resource currency
     @constant GAResourceFlowTypeSink
     Used when subtracting from a resource currency
     */
    enum EGAResourceFlowType
    {
        Source = 1,
        Sink = 2
    };

    /*!
     @enum
     @discussion
     his enum is used to specify status for progression event
     @constant GAProgressionStatusStart
     User started progression
     @constant GAProgressionStatusComplete
     User succesfully ended a progression
     @constant GAProgressionStatusFail
     User failed a progression
     */
    enum EGAProgressionStatus
    {
        Start = 1,
        Complete = 2,
        Fail = 3
    };

    /*!
     @enum
     @discussion
     his enum is used to specify severity of an error event
     @constant GAErrorSeverityDebug
     @constant GAErrorSeverityInfo
     @constant GAErrorSeverityWarning
     @constant GAErrorSeverityError
     @constant GAErrorSeverityCritical
     */
    enum EGAErrorSeverity
    {
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Critical = 5
    };

    enum EGAGender
    {
        Male = 1,
        Female = 2
    };

    class GameAnalytics
    {
     public:
        // configure calls should be used before initialize
        static void configureAvailableCustomDimensions01(const std::vector<std::string>& customDimensions);
        static void configureAvailableCustomDimensions02(const std::vector<std::string>& customDimensions);
        static void configureAvailableCustomDimensions03(const std::vector<std::string>& customDimensions);
        static void configureAvailableResourceCurrencies(const std::vector<std::string>& resourceCurrencies);
        static void configureAvailableResourceItemTypes(const std::vector<std::string>& resourceItemTypes);
        static void configureBuild(const std::string& build);
        static void configureWritablePath(const std::string& writablePath);
        static void configureDeviceModel(const std::string& deviceModel);
        static void configureDeviceManufacturer(const std::string& deviceManufacturer);

        // the version of SDK code used in an engine. Used for sdk_version field.
        // !! if set then it will override the SdkWrapperVersion.
        // example "unity 4.6.9"
        static void configureSdkGameEngineVersion(const std::string& sdkGameEngineVersion);
        // the version of the game engine (if used and version is available)
        static void configureGameEngineVersion(const std::string& engineVersion);

        static void configureUserId(const std::string& uId);

        // initialize - starting SDK (need configuration before starting)
        static void initialize(const std::string& gameKey, const std::string& gameSecret);

        // add events
        static void addBusinessEvent(const std::string& currency, int amount, const std::string& itemType, const std::string& itemId, const std::string& cartType);
        static void addBusinessEvent(const std::string& currency, int amount, const std::string& itemType, const std::string& itemId, const std::string& cartType, const Json::Value& fields);

        static void addResourceEvent(EGAResourceFlowType flowType, const std::string& currency, float amount, const std::string& itemType, const std::string& itemId);
        static void addResourceEvent(EGAResourceFlowType flowType, const std::string& currency, float amount, const std::string& itemType, const std::string& itemId, const Json::Value& fields);

        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const std::string& progression01, const std::string& progression02, const std::string& progression03);
        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const std::string& progression01, const std::string& progression02, const std::string& progression03, const Json::Value& fields);

        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const std::string& progression01, const std::string& progression02, const std::string& progression03, int score);
        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const std::string& progression01, const std::string& progression02, const std::string& progression03, int score, const Json::Value& fields);

        static void addDesignEvent(const std::string& eventId);
        static void addDesignEvent(const std::string& eventId, const Json::Value& fields);
        static void addDesignEvent(const std::string& eventId, double value);
        static void addDesignEvent(const std::string& eventId, double value, const Json::Value& fields);
        static void addErrorEvent(EGAErrorSeverity severity, const std::string& message);
        static void addErrorEvent(EGAErrorSeverity severity, const std::string& message, const Json::Value& fields);

        // set calls can be changed at any time (pre- and post-initialize)
        // some calls only work after a configure is called (setCustomDimension)
        static void setEnabledInfoLog(bool flag);
        static void setEnabledVerboseLog(bool flag);
        static void setEnabledManualSessionHandling(bool flag);
        static void setCustomDimension01(const std::string& dimension01);
        static void setCustomDimension02(const std::string& dimension02);
        static void setCustomDimension03(const std::string& dimension03);
        static void setFacebookId(const std::string& facebookId);
        static void setGender(EGAGender gender);
        static void setBirthYear(int birthYear);

        static void startSession();
        static void endSession();

        // game state changes
        // will affect how session is started / ended
        static void onResume();
        static void onStop();

#if USE_UWP
        static void configureAvailableCustomDimensions01(const std::vector<std::wstring>& customDimensions);
        static void configureAvailableCustomDimensions02(const std::vector<std::wstring>& customDimensions);
        static void configureAvailableCustomDimensions03(const std::vector<std::wstring>& customDimensions);
        static void configureAvailableResourceCurrencies(const std::vector<std::wstring>& resourceCurrencies);
        static void configureAvailableResourceItemTypes(const std::vector<std::wstring>& resourceItemTypes);
        static void configureBuild(const std::wstring& build);
        static void configureWritablePath(const std::wstring& writablePath);
        static void configureDeviceModel(const std::wstring& deviceModel);
        static void configureDeviceManufacturer(const std::wstring& deviceManufacturer);
        static void configureSdkGameEngineVersion(const std::wstring& sdkGameEngineVersion);
        static void configureGameEngineVersion(const std::wstring& engineVersion);
        static void configureUserId(const std::wstring& uId);
        static void initialize(const std::wstring& gameKey, const std::wstring& gameSecret);
        static void addBusinessEvent(const std::wstring& currency, int amount, const std::wstring& itemType, const std::wstring& itemId, const std::wstring& cartType);
        static void addResourceEvent(EGAResourceFlowType flowType, const std::wstring& currency, float amount, const std::wstring& itemType, const std::wstring& itemId);
        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const std::wstring& progression01, const std::wstring& progression02, const std::wstring& progression03);
        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const std::wstring& progression01, const std::wstring& progression02, const std::wstring& progression03, int score);
        static void addDesignEvent(const std::wstring& eventId);
        static void addDesignEvent(const std::wstring& eventId, double value);
        static void addErrorEvent(EGAErrorSeverity severity, const std::wstring& message);
        static void setCustomDimension01(const std::wstring& dimension01);
        static void setCustomDimension02(const std::wstring& dimension02);
        static void setCustomDimension03(const std::wstring& dimension03);
        static void setFacebookId(const std::wstring& facebookId);
#endif

     private:
        static bool isSdkReady(bool needsInitialized);
        static bool isSdkReady(bool needsInitialized, bool warn);
        static bool isSdkReady(bool needsInitialized, bool warn, std::string message);
#if USE_UWP
        static void OnAppSuspending(Platform::Object ^sender, Windows::ApplicationModel::SuspendingEventArgs ^e);
        static void OnAppResuming(Platform::Object ^sender, Platform::Object ^args);
#endif
    };
} // namespace gameanalytics
