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
EXPORT void addBusinessEvent(const char *currency, int amount, const char *itemType, const char *itemId, const char *cartType);

EXPORT void addResourceEvent(int flowType, const char *currency, float amount, const char *itemType, const char *itemId);

EXPORT void addProgressionEvent(int progressionStatus, const char *progression01, const char *progression02, const char *progression03);

EXPORT void addProgressionEventWithScore(int progressionStatus, const char *progression01, const char *progression02, const char *progression03, int score);

EXPORT void addDesignEvent(const char *eventId);
EXPORT void addDesignEventWithValue(const char *eventId, double value);
EXPORT void addErrorEvent(int severity, const char *message);

// set calls can be changed at any time (pre- and post-initialize)
// some calls only work after a configure is called (setCustomDimension)
EXPORT void setEnabledInfoLog(double flag);
EXPORT void setEnabledVerboseLog(double flag);
EXPORT void setEnabledManualSessionHandling(double flag);
EXPORT void setCustomDimension01(const char *dimension01);
EXPORT void setCustomDimension02(const char *dimension02);
EXPORT void setCustomDimension03(const char *dimension03);
EXPORT void setFacebookId(const char *facebookId);
EXPORT void setGender(int gender);
EXPORT void setBirthYear(int birthYear);

EXPORT void gameAnalyticsStartSession();
EXPORT void gameAnalyticsEndSession();

// game state changes
// will affect how session is started / ended
EXPORT void onResume();
EXPORT void onStop();

#ifdef __cplusplus
}
#endif
