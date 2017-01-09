#if USE_TIZEN

#include "GameAnalytics.h"
#include "GAUtilities.h"
#include <json/json.h>

void configureAvailableCustomDimensions01(const char *customDimensionsJson)
{
    Json::Value json = utilities::GAUtilities::jsonFromString(customDimensionsJson);
    const std::vector<std::string>& list;

    for (auto entry : json)
    {
        list.push_back(entry.asString());
    }

    gameanalytics::GameAnalytics::configureAvailableCustomDimensions01(list);
}

void configureAvailableCustomDimensions02(const char *customDimensionsJson)
{
    Json::Value json = utilities::GAUtilities::jsonFromString(customDimensionsJson);
    const std::vector<std::string>& list;

    for (auto entry : json)
    {
        list.push_back(entry.asString());
    }

    gameanalytics::GameAnalytics::configureAvailableCustomDimensions02(list);
}

void configureAvailableCustomDimensions03(const char *customDimensionsJson)
{
    Json::Value json = utilities::GAUtilities::jsonFromString(customDimensionsJson);
    const std::vector<std::string>& list;

    for (auto entry : json)
    {
        list.push_back(entry.asString());
    }

    gameanalytics::GameAnalytics::configureAvailableCustomDimensions03(list);
}

void configureAvailableResourceCurrencies(const char *resourceCurrenciesJson)
{
    Json::Value json = utilities::GAUtilities::jsonFromString(resourceCurrenciesJson);
    const std::vector<std::string>& list;

    for (auto entry : json)
    {
        list.push_back(entry.asString());
    }

    gameanalytics::GameAnalytics::configureAvailableResourceCurrencies(list);
}

void configureAvailableResourceItemTypes(const char *resourceItemTypesJson)
{
    Json::Value json = utilities::GAUtilities::jsonFromString(resourceItemTypesJson);
    const std::vector<std::string>& list;

    for (auto entry : json)
    {
        list.push_back(entry.asString());
    }

    gameanalytics::GameAnalytics::configureAvailableResourceItemTypes(list);
}

void configureBuild(const char *build)
{
    gameanalytics::GameAnalytics::configureBuild(build);
}

void configureWritablePath(const char *writablePath)
{
    gameanalytics::GameAnalytics::configureWritablePath(writablePath);
}

void configureDeviceModel(const char *deviceModel)
{
    gameanalytics::GameAnalytics::configureDeviceModel(deviceModel);
}

void configureDeviceManufacturer(const char *deviceManufacturer)
{
    gameanalytics::GameAnalytics::configureDeviceManufacturer(deviceManufacturer);
}

// the version of SDK code used in an engine. Used for sdk_version field.
// !! if set then it will override the SdkWrapperVersion.
// example "unity 4.6.9"
void configureSdkGameEngineVersion(const char *sdkGameEngineVersion)
{
    gameanalytics::GameAnalytics::configureSdkGameEngineVersion(sdkGameEngineVersion);
}

// the version of the game engine (if used and version is available)
void configureGameEngineVersion(const char *engineVersion)
{
    gameanalytics::GameAnalytics::configureGameEngineVersion(engineVersion);
}

void configureUserId(const char *uId)
{
    gameanalytics::GameAnalytics::configureUserId(uId);
}

// initialize - starting SDK (need configuration before starting)
void initialize(const char *gameKey, const char *gameSecret)

// add events
extern void addBusinessEvent(const char *currency, int amount, const char *itemType, const char *itemId, const char *cartType);

extern void addResourceEvent(int flowType, const char *currency, float amount, const char *itemType, const char *itemId);

extern void addProgressionEvent(int progressionStatus, const char *progression01, const char *progression02, const char *progression03);

extern void addProgressionEvent(int progressionStatus, const char *progression01, const char *progression02, const char *progression03, int score);

extern void addDesignEvent(const char *eventId);
extern void addDesignEvent(const char *eventId, double value);
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

extern void startSession();
extern void endSession();

// game state changes
// will affect how session is started / ended
extern void onResume();
extern void onStop();

#endif
