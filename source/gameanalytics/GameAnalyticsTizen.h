#ifdef __cplusplus
extern "C" {
#endif

extern void configureAvailableCustomDimensions01(const char *customDimensionsJson);
extern void configureAvailableCustomDimensions02(const char *customDimensionsJson);
extern void configureAvailableCustomDimensions03(const char *customDimensionsJson);
extern void configureAvailableResourceCurrencies(const char *resourceCurrenciesJson);
extern void configureAvailableResourceItemTypes(const char *resourceItemTypesJson);
extern void configureBuild(const char *build);
extern void configureWritablePath(const char *writablePath);
extern void configureDeviceModel(const char *deviceModel);
extern void configureDeviceManufacturer(const char *deviceManufacturer);

// the version of SDK code used in an engine. Used for sdk_version field.
// !! if set then it will override the SdkWrapperVersion.
// example "unity 4.6.9"
extern void configureSdkGameEngineVersion(const char *sdkGameEngineVersion);
// the version of the game engine (if used and version is available)
extern void configureGameEngineVersion(const char *engineVersion);

extern void configureUserId(const char *uId);

// initialize - starting SDK (need configuration before starting)
extern void initialize(const char *gameKey, const char *gameSecret);

// add events
extern void addBusinessEvent(const char *currency, int amount, const char *itemType, const char *itemId, const char *cartType);

extern void addResourceEvent(int flowType, const char *currency, float amount, const char *itemType, const char *itemId);

extern void addProgressionEvent(int progressionStatus, const char *progression01, const char *progression02, const char *progression03);

extern void addProgressionEventWithScore(int progressionStatus, const char *progression01, const char *progression02, const char *progression03, int score);

extern void addDesignEvent(const char *eventId);
extern void addDesignEventWithValue(const char *eventId, double value);
extern void addErrorEvent(int severity, const char *message);

// set calls can be changed at any time (pre- and post-initialize)
// some calls only work after a configure is called (setCustomDimension)
extern void setEnabledInfoLog(bool flag);
extern void setEnabledVerboseLog(bool flag);
extern void setEnabledManualSessionHandling(bool flag);
extern void setCustomDimension01(const char *dimension01);
extern void setCustomDimension02(const char *dimension02);
extern void setCustomDimension03(const char *dimension03);
extern void setFacebookId(const char *facebookId);
extern void setGender(int gender);
extern void setBirthYear(int birthYear);

extern void gameAnalyticsStartSession();
extern void gameAnalyticsEndSession();

// game state changes
// will affect how session is started / ended
extern void onResume();
extern void onStop();

#ifdef __cplusplus
}
#endif
