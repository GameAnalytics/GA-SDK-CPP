#if USE_TIZEN || GA_SHARED_LIB

#include "GameAnalytics.h"
#include "GAUtilities.h"
#include <json/json.h>

void configureAvailableCustomDimensions01(const char *customDimensionsJson)
{
    Json::Value json = gameanalytics::utilities::GAUtilities::jsonFromString(customDimensionsJson);
    std::vector<std::string> list;

    for (unsigned int index = 0; index < json.size(); ++index)
    {
        list.push_back(json[index].asString());
    }

    gameanalytics::GameAnalytics::configureAvailableCustomDimensions01(list);
}

void configureAvailableCustomDimensions02(const char *customDimensionsJson)
{
    Json::Value json = gameanalytics::utilities::GAUtilities::jsonFromString(customDimensionsJson);
    std::vector<std::string> list;

    for (unsigned int index = 0; index < json.size(); ++index)
    {
        list.push_back(json[index].asString());
    }

    gameanalytics::GameAnalytics::configureAvailableCustomDimensions02(list);
}

void configureAvailableCustomDimensions03(const char *customDimensionsJson)
{
    Json::Value json = gameanalytics::utilities::GAUtilities::jsonFromString(customDimensionsJson);
    std::vector<std::string> list;

    for (unsigned int index = 0; index < json.size(); ++index)
    {
        list.push_back(json[index].asString());
    }

    gameanalytics::GameAnalytics::configureAvailableCustomDimensions03(list);
}

void configureAvailableResourceCurrencies(const char *resourceCurrenciesJson)
{
    Json::Value json = gameanalytics::utilities::GAUtilities::jsonFromString(resourceCurrenciesJson);
    std::vector<std::string> list;

    for (unsigned int index = 0; index < json.size(); ++index)
    {
        list.push_back(json[index].asString());
    }

    gameanalytics::GameAnalytics::configureAvailableResourceCurrencies(list);
}

void configureAvailableResourceItemTypes(const char *resourceItemTypesJson)
{
    Json::Value json = gameanalytics::utilities::GAUtilities::jsonFromString(resourceItemTypesJson);
    std::vector<std::string> list;

    for (unsigned int index = 0; index < json.size(); ++index )
    {
        list.push_back(json[index].asString());
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
{
    gameanalytics::GameAnalytics::initialize(gameKey, gameSecret);
}

// add events
void addBusinessEvent(const char *currency, double amount, const char *itemType, const char *itemId, const char *cartType)
{
    gameanalytics::GameAnalytics::addBusinessEvent(currency, (int)amount, itemType, itemId, cartType);
}

void addBusinessEventJson(const char *jsonArgs)
{
    Json::Value json = gameanalytics::utilities::GAUtilities::jsonFromString(jsonArgs);

    if(json.size() == 5)
    {
        gameanalytics::GameAnalytics::addBusinessEvent(json[0].asString(), (int)(json[1].asDouble()), json[2].asString(), json[3].asString(), json[4].asString());
    }
}

void addResourceEvent(double flowType, const char *currency, double amount, const char *itemType, const char *itemId)
{
    int flowTypeInt = (int)flowType;
    gameanalytics::GameAnalytics::addResourceEvent((gameanalytics::EGAResourceFlowType)flowTypeInt, currency, (float)amount, itemType, itemId);
}

void addResourceEventJson(const char *jsonArgs)
{
    Json::Value json = gameanalytics::utilities::GAUtilities::jsonFromString(jsonArgs);

    if(json.size() == 5)
    {
        int flowTypeInt = (int)(int)(json[0].asDouble());
        gameanalytics::GameAnalytics::addResourceEvent((gameanalytics::EGAResourceFlowType)flowTypeInt, json[1].asString(), (float)(json[2].asDouble()), json[3].asString(), json[4].asString());
    }
}

void addProgressionEvent(double progressionStatus, const char *progression01, const char *progression02, const char *progression03)
{
    int progressionStatusInt = (int)progressionStatus;
    gameanalytics::GameAnalytics::addProgressionEvent((gameanalytics::EGAProgressionStatus)progressionStatusInt, progression01, progression02, progression03);
}

void addProgressionEventWithScore(double progressionStatus, const char *progression01, const char *progression02, const char *progression03, double score)
{
    int progressionStatusInt = (int)progressionStatus;
    gameanalytics::GameAnalytics::addProgressionEvent((gameanalytics::EGAProgressionStatus)progressionStatusInt, progression01, progression02, progression03, (int)score);
}

void addProgressionEventWithScoreJson(const char *jsonArgs)
{
    Json::Value json = gameanalytics::utilities::GAUtilities::jsonFromString(jsonArgs);

    if(json.size() == 5)
    {
        int progressionStatusInt = (int)(int)(json[0].asDouble());
        gameanalytics::GameAnalytics::addResourceEvent((gameanalytics::EGAProgressionStatus)progressionStatusInt, json[1].asString(), json[2].asString(), json[3].asString(), (int)(json[4].asDouble()));
    }
}

void addDesignEvent(const char *eventId)
{
    gameanalytics::GameAnalytics::addDesignEvent(eventId);
}

void addDesignEventWithValue(const char *eventId, double value)
{
    gameanalytics::GameAnalytics::addDesignEvent(eventId, value);
}

void addErrorEvent(double severity, const char *message)
{
    int severityInt = (int)severity;
    gameanalytics::GameAnalytics::addErrorEvent((gameanalytics::EGAErrorSeverity)severityInt, message);
}

// set calls can be changed at any time (pre- and post-initialize)
// some calls only work after a configure is called (setCustomDimension)

void setEnabledInfoLog(double flag)
{
    gameanalytics::GameAnalytics::setEnabledInfoLog(flag != 0.0);
}

void setEnabledVerboseLog(double flag)
{
    gameanalytics::GameAnalytics::setEnabledVerboseLog(flag != 0.0);
}

void setEnabledManualSessionHandling(double flag)
{
    gameanalytics::GameAnalytics::setEnabledManualSessionHandling(flag != 0.0);
}

void setCustomDimension01(const char *dimension01)
{
    gameanalytics::GameAnalytics::setCustomDimension01(dimension01);
}

void setCustomDimension02(const char *dimension02)
{
    gameanalytics::GameAnalytics::setCustomDimension02(dimension02);
}

void setCustomDimension03(const char *dimension03)
{
    gameanalytics::GameAnalytics::setCustomDimension03(dimension03);
}

void setFacebookId(const char *facebookId)
{
    gameanalytics::GameAnalytics::setFacebookId(facebookId);
}

void setGender(double gender)
{
    int genderInt = (int)gender;
    gameanalytics::GameAnalytics::setGender((gameanalytics::EGAGender)genderInt);
}

void setBirthYear(double birthYear)
{
    gameanalytics::GameAnalytics::setBirthYear((int)birthYear);
}

void gameAnalyticsStartSession()
{
    gameanalytics::GameAnalytics::startSession();
}

void gameAnalyticsEndSession()
{
    gameanalytics::GameAnalytics::endSession();
}

// game state changes
// will affect how session is started / ended
void onResume()
{
    gameanalytics::GameAnalytics::onResume();
}

void onStop()
{
    gameanalytics::GameAnalytics::onStop();
}

#endif
