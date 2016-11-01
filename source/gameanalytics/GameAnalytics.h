//
// GA-SDK-CPP
// Copyright 2015 CppWrapper. All rights reserved.
//

#pragma once

#if GA_SHARED_LIB
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "gdi32.lib")
#endif

#include <string>
#include <vector>

#if USE_UWP
typedef std::wstring STRING_TYPE;
#else
typedef std::string STRING_TYPE;
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
        static void configureAvailableCustomDimensions01(const std::vector<STRING_TYPE>& customDimensions);
        static void configureAvailableCustomDimensions02(const std::vector<STRING_TYPE>& customDimensions);
        static void configureAvailableCustomDimensions03(const std::vector<STRING_TYPE>& customDimensions);
        static void configureAvailableResourceCurrencies(const std::vector<STRING_TYPE>& resourceCurrencies);
        static void configureAvailableResourceItemTypes(const std::vector<STRING_TYPE>& resourceItemTypes);
        static void configureBuild(const STRING_TYPE& build);
        static void configureWritablePath(const STRING_TYPE& writablePath);
        static void configureDeviceModel(const STRING_TYPE& deviceModel);
        static void configureDeviceManufacturer(const STRING_TYPE& deviceManufacturer);

        // the version of SDK code used in an engine. Used for sdk_version field.
        // !! if set then it will override the SdkWrapperVersion.
        // example "unity 4.6.9"
        static void configureSdkGameEngineVersion(const STRING_TYPE& sdkGameEngineVersion);
        // the version of the game engine (if used and version is available)
        static void configureGameEngineVersion(const STRING_TYPE& engineVersion);

        static void configureUserId(const STRING_TYPE& uId);

        // initialize - starting SDK (need configuration before starting)
        static void initialize(const STRING_TYPE& gameKey, const STRING_TYPE& gameSecret);

        // add events
        static void addBusinessEvent(const STRING_TYPE& currency,
                                                 int amount,
                                                 const STRING_TYPE& itemType,
                                                 const STRING_TYPE& itemId,
                                                 const STRING_TYPE& cartType);

        static void addResourceEvent(EGAResourceFlowType flowType,
                                                 const STRING_TYPE& currency,
                                                 float amount,
                                                 const STRING_TYPE&itemType,
                                                 const STRING_TYPE& itemId);

        static void addProgressionEvent(EGAProgressionStatus progressionStatus,
                                                             const STRING_TYPE& progression01,
                                                             const STRING_TYPE& progression02,
                                                             const STRING_TYPE& progression03);

        static void addProgressionEvent(EGAProgressionStatus progressionStatus,
                                                             const STRING_TYPE& progression01,
                                                             const STRING_TYPE& progression02,
                                                             const STRING_TYPE& progression03,
                                                             int score);

        static void addDesignEvent(const STRING_TYPE& eventId);
        static void addDesignEvent(const STRING_TYPE& eventId, double value);
        static void addErrorEvent(EGAErrorSeverity severity, const STRING_TYPE& message);

        // set calls can be changed at any time (pre- and post-initialize)
        // some calls only work after a configure is called (setCustomDimension)
        static void setEnabledInfoLog(bool flag);
        static void setEnabledVerboseLog(bool flag);
        static void setEnabledManualSessionHandling(bool flag);
        static void setCustomDimension01(const STRING_TYPE& dimension01);
        static void setCustomDimension02(const STRING_TYPE& dimension02);
        static void setCustomDimension03(const STRING_TYPE& dimension03);
        static void setFacebookId(const STRING_TYPE& facebookId);
        static void setGender(EGAGender gender);
        static void setBirthYear(int birthYear);

        static void startSession();
        static void endSession();

        // game state changes
        // will affect how session is started / ended
        static void onResume();
        static void onStop();

     private:
        static bool isSdkReady(bool needsInitialized);
        static bool isSdkReady(bool needsInitialized, bool warn);
        static bool isSdkReady(bool needsInitialized, bool warn, STRING_TYPE message);
    };
} // namespace gameanalytics
