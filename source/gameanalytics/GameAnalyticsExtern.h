#ifdef __cplusplus
extern "C" {
#endif

#if GA_SHARED_LIB
#if defined(_WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif
#else
#define EXPORT
#endif

EXPORT void configureAvailableCustomDimensions01(const char *customDimensionsJson);
EXPORT void configureAvailableCustomDimensions02(const char *customDimensionsJson);
EXPORT void configureAvailableCustomDimensions03(const char *customDimensionsJson);
EXPORT void configureAvailableResourceCurrencies(const char *resourceCurrenciesJson);
EXPORT void configureAvailableResourceItemTypes(const char *resourceItemTypesJson);
EXPORT void configureBuild(const char *build);
EXPORT void configureWritablePath(const char *writablePath);
EXPORT void configureDeviceModel(const char *deviceModel);
EXPORT void configureDeviceManufacturer(const char *deviceManufacturer);

// the version of SDK code used in an engine. Used for sdk_version field.
// !! if set then it will override the SdkWrapperVersion.
// example "unity 4.6.9"
EXPORT void configureSdkGameEngineVersion(const char *sdkGameEngineVersion);
// the version of the game engine (if used and version is available)
EXPORT void configureGameEngineVersion(const char *engineVersion);

EXPORT void configureUserId(const char *uId);

// initialize - starting SDK (need configuration before starting)
EXPORT void initialize(const char *gameKey, const char *gameSecret);

// add events
EXPORT void addBusinessEvent(const char *currency, double amount, const char *itemType, const char *itemId, const char *cartType/*, const char *fields*/);
EXPORT void addBusinessEventJson(const char *jsonArgs);

EXPORT void addResourceEvent(double flowType, const char *currency, double amount, const char *itemType, const char *itemId/*, const char *fields*/);
EXPORT void addResourceEventJson(const char *jsonArgs);

EXPORT void addProgressionEvent(double progressionStatus, const char *progression01, const char *progression02, const char *progression03/*, const char *fields*/);
EXPORT void addProgressionEventJson(const char *jsonArgs);

EXPORT void addProgressionEventWithScore(double progressionStatus, const char *progression01, const char *progression02, const char *progression03, double score/*, const char *fields*/);
EXPORT void addProgressionEventWithScoreJson(const char *jsonArgs);

EXPORT void addDesignEvent(const char *eventId/*, const char *fields*/);
EXPORT void addDesignEventWithValue(const char *eventId, double value/*, const char *fields*/);
EXPORT void addErrorEvent(double severity, const char *message/*, const char *fields*/);

// set calls can be changed at any time (pre- and post-initialize)
// some calls only work after a configure is called (setCustomDimension)
EXPORT void setEnabledInfoLog(double flag);
EXPORT void setEnabledVerboseLog(double flag);
EXPORT void setEnabledManualSessionHandling(double flag);
EXPORT void setCustomDimension01(const char *dimension01);
EXPORT void setCustomDimension02(const char *dimension02);
EXPORT void setCustomDimension03(const char *dimension03);
EXPORT void setFacebookId(const char *facebookId);
EXPORT void setGender(double gender);
EXPORT void setBirthYear(double birthYear);

// EXPORT const char* getCommandCenterValueAsString(const char *key);
// EXPORT const char* getCommandCenterValueAsStringWithDefaultValue(const char *key, const char *defaultValue);
// EXPORT bool isCommandCenterReady();

EXPORT void gameAnalyticsStartSession();
EXPORT void gameAnalyticsEndSession();

// game state changes
// will affect how session is started / ended
EXPORT void onResume();
EXPORT void onSuspend();
EXPORT void onQuit();

EXPORT const char* getCommandCenterValueAsString(const char *key);
EXPORT const char* getCommandCenterValueAsStringWithDefaultValue(const char *key, const char *defaultValue);
EXPORT double isCommandCenterReady();
EXPORT const char* getConfigurationsContentAsString();

#ifdef __cplusplus
}
#endif
