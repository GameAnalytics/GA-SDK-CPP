//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#if GA_SHARED_LIB && defined(_WIN32)
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "gdi32.lib")
#endif

#include <vector>
#include <memory>
#if USE_TIZEN || GA_SHARED_LIB
#include "GameAnalyticsExtern.h"
#endif
#if !USE_UWP && !USE_TIZEN
#include <ostream>
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

    class ICommandCenterListener
    {
        public:
            virtual void onCommandCenterUpdated() = 0;
    };

    struct CharArray
    {
    public:
        char array[65] = "";
    };

    struct StringVector
    {
    public:
        StringVector& add(const char* s)
        {
            CharArray entry;
            snprintf(entry.array, sizeof(entry.array), "%s", s);
            v.push_back(entry);
            return *this;
        }

        const std::vector<CharArray>& getVector() const
        {
            return v;
        }

    private:
        std::vector<CharArray> v;
    };

    class GameAnalytics
    {
     public:
        // configure calls should be used before initialize
        static void configureAvailableCustomDimensions01(const StringVector& customDimensions);
        static void configureAvailableCustomDimensions01(const char* customDimensions);
        static void configureAvailableCustomDimensions02(const StringVector& customDimensions);
        static void configureAvailableCustomDimensions02(const char* customDimensions);
        static void configureAvailableCustomDimensions03(const StringVector& customDimensions);
        static void configureAvailableCustomDimensions03(const char* customDimensions);
        static void configureAvailableResourceCurrencies(const StringVector& resourceCurrencies);
        static void configureAvailableResourceCurrencies(const char* resourceCurrencies);
        static void configureAvailableResourceItemTypes(const StringVector& resourceItemTypes);
        static void configureAvailableResourceItemTypes(const char* resourceItemTypes);
        static void configureBuild(const char* build);
        static void configureWritablePath(const char* writablePath);
        static void configureDeviceModel(const char* deviceModel);
        static void configureDeviceManufacturer(const char* deviceManufacturer);

        // the version of SDK code used in an engine. Used for sdk_version field.
        // !! if set then it will override the SdkWrapperVersion.
        // example "unity 4.6.9"
        static void configureSdkGameEngineVersion(const char* sdkGameEngineVersion);
        // the version of the game engine (if used and version is available)
        static void configureGameEngineVersion(const char* engineVersion);

        static void configureUserId(const char* uId);

        // initialize - starting SDK (need configuration before starting)
        static void initialize(const char* gameKey, const char* gameSecret);

        // add events
        static void addBusinessEvent(const char* currency, int amount, const char* itemType, const char* itemId, const char* cartType);

        static void addResourceEvent(EGAResourceFlowType flowType, const char* currency, float amount, const char* itemType, const char* itemId);

        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const char* progression01, const char* progression02, const char* progression03);

        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const char* progression01, const char* progression02, const char* progression03, int score);

        static void addDesignEvent(const char* eventId);

        static void addDesignEvent(const char* eventId, double value);
        static void addErrorEvent(EGAErrorSeverity severity, const char* message);

        // set calls can be changed at any time (pre- and post-initialize)
        // some calls only work after a configure is called (setCustomDimension)
        static void setEnabledInfoLog(bool flag);
        static void setEnabledVerboseLog(bool flag);
        static void setEnabledManualSessionHandling(bool flag);
        static void setEnabledErrorReporting(bool flag);
        static void setEnabledEventSubmission(bool flag);
        static void setCustomDimension01(const char* dimension01);
        static void setCustomDimension02(const char* dimension02);
        static void setCustomDimension03(const char* dimension03);
        static void setFacebookId(const char* facebookId);
        static void setGender(EGAGender gender);
        static void setBirthYear(int birthYear);

        static void startSession();
        static void endSession();

        static const char* getCommandCenterValueAsString(const char* key);
        static const char* getCommandCenterValueAsString(const char* key, const char* defaultValue);
        static bool isCommandCenterReady();
        static void addCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener);
        static void removeCommandCenterListener(const std::shared_ptr<ICommandCenterListener>& listener);
        static void getConfigurationsContentAsString(char* out);

        // game state changes
        // will affect how session is started / ended
        static void onResume();
        static void onSuspend();
        static void onQuit();

        static bool isThreadEnding();

#if !USE_UWP && !USE_TIZEN
        static void addCustomLogStream(std::ostream& os);
#endif

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
         static bool _endThread;
#if USE_UWP
        static void addBusinessEvent(const std::wstring& currency, int amount, const std::wstring& itemType, const std::wstring& itemId, const std::wstring& cartType, const std::wstring& fields);
        static void addResourceEvent(EGAResourceFlowType flowType, const std::wstring& currency, float amount, const std::wstring& itemType, const std::wstring& itemId, const std::wstring& fields);
        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const std::wstring& progression01, const std::wstring& progression02, const std::wstring& progression03, const std::wstring& fields);
        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const std::wstring& progression01, const std::wstring& progression02, const std::wstring& progression03, int score, const std::wstring& fields);
        static void addDesignEvent(const std::wstring& eventId, const std::wstring& fields);
        static void addDesignEvent(const std::wstring& eventId, double value, const std::wstring& fields);
        static void addErrorEvent(EGAErrorSeverity severity, const std::wstring& message, const std::wstring& fields);
#endif
        static void addBusinessEvent(const char* currency, int amount, const char* itemType, const char* itemId, const char* cartType, const char* fields);
        static void addResourceEvent(EGAResourceFlowType flowType, const char* currency, float amount, const char* itemType, const char* itemId, const char* fields);
        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const char* progression01, const char* progression02, const char* progression03, const char* fields);
        static void addProgressionEvent(EGAProgressionStatus progressionStatus, const char* progression01, const char* progression02, const char* progression03, int score, const char* fields);
        static void addDesignEvent(const char* eventId, const char* fields);
        static void addDesignEvent(const char* eventId, double value, const char* fields);
        static void addErrorEvent(EGAErrorSeverity severity, const char* message, const char* fields);

        static bool isSdkReady(bool needsInitialized);
        static bool isSdkReady(bool needsInitialized, bool warn);
        static bool isSdkReady(bool needsInitialized, bool warn, const char* message);
#if USE_UWP
        static void OnAppSuspending(Platform::Object ^sender, Windows::ApplicationModel::SuspendingEventArgs ^e);
        static void OnAppResuming(Platform::Object ^sender, Platform::Object ^args);
#endif
    };
} // namespace gameanalytics
