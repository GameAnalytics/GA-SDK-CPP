//
// GA-SDK-CPP
// Copyright 2015 CppWrapper. All rights reserved.
//

#include "GameAnalytics.h"

#include "GAThreading.h"
#include "GALogger.h"
#include "GAState.h"
#include "GADevice.h"
#include "GAValidator.h"
#include "GAEvents.h"
#include "GAUtilities.h"
#include "GAStore.h"

namespace gameanalytics
{   
    // ----------------------- CONFIGURE ---------------------- //

    void GameAnalytics::configureAvailableCustomDimensions01(const std::vector<std::string>& customDimensions)
    {
        threading::GAThreading::performTaskOnGAThread([customDimensions]()
        {
            if (isSdkReady(true, false))
            {
                logging::GALogger::w("Available custom dimensions must be set before SDK is initialized");
                return;
            }
            state::GAState::setAvailableCustomDimensions01(customDimensions);
        });
    }

    void GameAnalytics::configureAvailableCustomDimensions02(const std::vector<std::string>& customDimensions)
    {
        threading::GAThreading::performTaskOnGAThread([customDimensions]()
        {
            if (isSdkReady(true, false))
            {
                logging::GALogger::w("Available custom dimensions must be set before SDK is initialized");
                return;
            }
            state::GAState::setAvailableCustomDimensions02(customDimensions);
        });
    }

    void GameAnalytics::configureAvailableCustomDimensions03(const std::vector<std::string>& customDimensions)
    {
        threading::GAThreading::performTaskOnGAThread([customDimensions]()
        {
            if (isSdkReady(true, false))
            {
                logging::GALogger::w("Available custom dimensions must be set before SDK is initialized");
                return;
            }
            state::GAState::setAvailableCustomDimensions03(customDimensions);
        });
    }

    void GameAnalytics::configureAvailableResourceCurrencies(const std::vector<std::string>& resourceCurrencies)
    {
        threading::GAThreading::performTaskOnGAThread([resourceCurrencies]()
        {
            if (isSdkReady(true, false))
            {
                logging::GALogger::w("Available resource currencies must be set before SDK is initialized");
                return;
            }
            state::GAState::setAvailableResourceCurrencies(resourceCurrencies);
        });
    }

    void GameAnalytics::configureAvailableResourceItemTypes(const std::vector<std::string>& resourceItemTypes)
    {
        threading::GAThreading::performTaskOnGAThread([resourceItemTypes]()
        {
            if (isSdkReady(true, false))
            {
                logging::GALogger::w("Available resource item types must be set before SDK is initialized");
                return;
            }
            state::GAState::setAvailableResourceItemTypes(resourceItemTypes);
        });
    }

    void GameAnalytics::configureBuild(const std::string& build)
    {
        threading::GAThreading::performTaskOnGAThread([build]()
        {
            if (isSdkReady(true, false))
            {
                logging::GALogger::w("Build version must be set before SDK is initialized.");
                return;
            }
            if (!validators::GAValidator::validateBuild(build))
            {
                logging::GALogger::i("Validation fail - configure build: Cannot be null, empty or above 32 length. String: " + build);
                return;
            }
            state::GAState::setBuild(build);
        });
    }

    void GameAnalytics::configureWritablePath(const std::string& writablePath)
    {
        threading::GAThreading::performTaskOnGAThread([writablePath]()
        {
            if (isSdkReady(true, false))
            {
                logging::GALogger::w("Writable path must be set before SDK is initialized.");
                return;
            }
            device::GADevice::setWritablePath(writablePath);

            logging::GALogger::addFileLog(writablePath);
        });
    }

    void GameAnalytics::configureDeviceModel(const std::string& deviceModel)
    {
        threading::GAThreading::performTaskOnGAThread([deviceModel]()
        {
            if (isSdkReady(true, false))
            {
                logging::GALogger::w("Device model must be set before SDK is initialized.");
                return;
            }
            if (!validators::GAValidator::validateString(deviceModel, true))
            {
                logging::GALogger::i("Validation fail - configure device model: Cannot be null, empty or above 64 length. String: " + deviceModel);
                return;
            }
            device::GADevice::setDeviceModel(deviceModel);
        });
    }

    void GameAnalytics::configureDeviceManufacturer(const std::string& deviceManufacturer)
    {
        threading::GAThreading::performTaskOnGAThread([deviceManufacturer]()
        {
            if (isSdkReady(true, false))
            {
                logging::GALogger::w("Device manufacturer must be set before SDK is initialized.");
                return;
            }
            if (!validators::GAValidator::validateString(deviceManufacturer, true))
            {
                logging::GALogger::i("Validation fail - configure device manufacturer: Cannot be null, empty or above 64 length. String: " + deviceManufacturer);
                return;
            }
            device::GADevice::setDeviceManufacturer(deviceManufacturer);
        });
    }

    void GameAnalytics::configureSdkGameEngineVersion(const std::string& sdkGameEngineVersion)
    {
        threading::GAThreading::performTaskOnGAThread([sdkGameEngineVersion]()
        {
            if (isSdkReady(true, false))
            {
                return;
            }
            if (!validators::GAValidator::validateSdkWrapperVersion(sdkGameEngineVersion))
            {
                logging::GALogger::i("Validation fail - configure sdk version: Sdk version not supported. String: " + sdkGameEngineVersion);
                return;
            }
            device::GADevice::setSdkGameEngineVersion(sdkGameEngineVersion);
        });
    }

    void GameAnalytics::configureGameEngineVersion(const std::string& gameEngineVersion)
    {
        threading::GAThreading::performTaskOnGAThread([gameEngineVersion]()
        {
            if (isSdkReady(true, false))
            {
                return;
            }
            if (!validators::GAValidator::validateEngineVersion(gameEngineVersion))
            {
                logging::GALogger::i("Validation fail - configure engine: Engine version not supported. String: " + gameEngineVersion);
                return;
            }
            device::GADevice::setGameEngineVersion(gameEngineVersion);
        });
    }

    void GameAnalytics::configureUserId(const std::string& uId)
    {
        threading::GAThreading::performTaskOnGAThread([uId]()
        {
            if (!isSdkReady(true, false))
            {
                logging::GALogger::w("A custom user id must be set before SDK is initialized.");
                return;
            }
            if (validators::GAValidator::validateUserId(uId))
            {
                logging::GALogger::i("Validation fail - configure user_id: Cannot be null, empty or above 64 length. Will use default user_id method. Used string: " + uId);
                return;
            }

            state::GAState::setUserId(uId);
        });
    }

    // ----------------------- INITIALIZE ---------------------- //

    void GameAnalytics::initialize(const std::string& gameKey, const std::string& gameSecret)
    {
        threading::GAThreading::performTaskOnGAThread([gameKey, gameSecret]()
        {
            if (isSdkReady(true, false))
            {
                logging::GALogger::w("SDK already initialized. Can only be called once.");
                return;
            }
            if (!validators::GAValidator::validateKeys(gameKey, gameSecret))
            {
                logging::GALogger::w("SDK failed initialize. Game key or secret key is invalid. Can only contain characters A-z 0-9, gameKey is 32 length, gameSecret is 40 length. Failed keys - gameKey: " + gameKey + ", secretKey: " + gameSecret);
                return;
            }

            state::GAState::setKeys(gameKey, gameSecret);

            if (!store::GAStore::ensureDatabase(false))
            {
                logging::GALogger::w("Could not ensure/validate local event database: " + device::GADevice::getWritablePath());
            }

            state::GAState::internalInitialize();
        });
    }

    // ----------------------- ADD EVENTS ---------------------- //


    void GameAnalytics::addBusinessEvent(
        const std::string& currency,
        int amount,
        const std::string& itemType,
        const std::string& itemId,
        const std::string& cartType)
    {
        threading::GAThreading::performTaskOnGAThread([currency, amount, itemType, itemId, cartType]()
        {
            if (!isSdkReady(true, true, "Could not add business event"))
            {
                return;
            }
            // Send to events
            events::GAEvents::addBusinessEvent(currency, amount, itemType, itemId, cartType);
        });
    }


    void GameAnalytics::addResourceEvent(EGAResourceFlowType flowType, const std::string& currency, float amount, const std::string&itemType, const std::string& itemId)
    {
        threading::GAThreading::performTaskOnGAThread([flowType, currency, amount, itemType, itemId]()
        {
            if (!isSdkReady(true, true, "Could not add resource event"))
            {
                return;
            }

            events::GAEvents::addResourceEvent(flowType, currency, amount, itemType, itemId);
        });
    }

    void GameAnalytics::addProgressionEvent(EGAProgressionStatus progressionStatus, const std::string& progression01, const std::string& progression02, const std::string& progression03)
    {
        threading::GAThreading::performTaskOnGAThread([progressionStatus, progression01, progression02, progression03]()
        {
            if (!isSdkReady(true, true, "Could not add progression event"))
            {
                return;
            }

            // Send to events
            // TODO(nikolaj): check if passing 0 / null as the last argument is OK
            events::GAEvents::addProgressionEvent(progressionStatus, progression01, progression02, progression03, 0.0);
        });
    }

    void GameAnalytics::addProgressionEvent(EGAProgressionStatus progressionStatus, const std::string& progression01, const std::string& progression02, const std::string& progression03, int score)
    {
        threading::GAThreading::performTaskOnGAThread([progressionStatus, progression01, progression02, progression03, score]()
        {
            if (!isSdkReady(true, true, "Could not add progression event"))
            {
                return;
            }

            // Send to events
            // TODO(nikolaj): check if this cast from int to double is OK
            events::GAEvents::addProgressionEvent(progressionStatus, progression01, progression02, progression03, score);  
        });
    }

    void GameAnalytics::addDesignEvent(const std::string& eventId)
    {
         // TODO(nikolaj): check if passing 0.0 instead of nil is ok here
        addDesignEvent(eventId, 0.0);
    }

    void GameAnalytics::addDesignEvent(const std::string& eventId, double value)
    {
        threading::GAThreading::performTaskOnGAThread([eventId, value]()
        {
            if (!isSdkReady(true, true, "Could not add design event"))
            {
                return;
            }
            events::GAEvents::addDesignEvent(eventId, value);
        });
    }

    void GameAnalytics::addErrorEvent(EGAErrorSeverity severity, const std::string& message)
    {
        threading::GAThreading::performTaskOnGAThread([severity, message]()
        {
            if (!isSdkReady(true, true, "Could not add error event"))
            {
                return;
            }
            events::GAEvents::addErrorEvent(severity, message);
        });
    }

    // ------------- SET STATE CHANGES WHILE RUNNING ----------------- //

    void GameAnalytics::setEnabledInfoLog(bool flag)
    {
        threading::GAThreading::performTaskOnGAThread([flag]()
        {
            if (flag)
            {
                logging::GALogger::setInfoLog(flag);
                logging::GALogger::i("Info logging enabled");
            }
            else
            {
                logging::GALogger::i("Info logging disabled");
                logging::GALogger::setInfoLog(flag);
            }
        });
    }

    void GameAnalytics::setEnabledVerboseLog(bool flag)
    {
        threading::GAThreading::performTaskOnGAThread([flag]()
        {
            if (flag)
            {
                logging::GALogger::setVerboseInfoLog(flag);
                logging::GALogger::i("Verbose logging enabled");
            }
            else
            {
                logging::GALogger::i("Verbose logging disabled");
                logging::GALogger::setVerboseInfoLog(flag);
            }
        });
    }

    void GameAnalytics::setEnabledManualSessionHandling(bool flag)
    {
        threading::GAThreading::performTaskOnGAThread([flag]()
        {
            state::GAState::setManualSessionHandling(flag);
        });
    }

    void GameAnalytics::setCustomDimension01(const std::string& dimension)
    {
        threading::GAThreading::performTaskOnGAThread([dimension]()
        {
            if (!isSdkReady(false))
            {
                return;
            }

            if (!validators::GAValidator::validateDimension01(dimension))
            {
                logging::GALogger::w("Could not set custom01 dimension value to '" + dimension + "'. Value not found in available custom01 dimension values");
                return;
            }
            state::GAState::setCustomDimension01(dimension);
        });
    }

    void GameAnalytics::setCustomDimension02(const std::string& dimension)
    {
        threading::GAThreading::performTaskOnGAThread([dimension]()
        {
            if (!isSdkReady(false))
            {
                return;
            }

            if (!validators::GAValidator::validateDimension02(dimension))
            {
                logging::GALogger::w("Could not set custom02 dimension value to '" + dimension + "'. Value not found in available custom01 dimension values");
                return;
            }
            state::GAState::setCustomDimension02(dimension);
        });
    }

    void GameAnalytics::setCustomDimension03(const std::string& dimension)
    {
        threading::GAThreading::performTaskOnGAThread([dimension]()
        {
            if (!isSdkReady(false))
            {
                return;
            }

            if (!validators::GAValidator::validateDimension03(dimension))
            {
                logging::GALogger::w("Could not set custom03 dimension value to '" + dimension + "'. Value not found in available custom01 dimension values");
                return;
            }
            state::GAState::setCustomDimension03(dimension);
        });
    }

    void GameAnalytics::setFacebookId(const std::string& facebookId)
    {
        threading::GAThreading::performTaskOnGAThread([facebookId]()
        {
            if (!isSdkReady(false))
            {
                return;
            }

            if (validators::GAValidator::validateFacebookId(facebookId))
            {
                state::GAState::setFacebookId(facebookId);
            }
        });
    }

    void GameAnalytics::setGender(EGAGender gender)
    {
        threading::GAThreading::performTaskOnGAThread([gender]()
        {
            if (!isSdkReady(false))
            {
                return;
            }

            if (validators::GAValidator::validateGender(gender))
            {
                state::GAState::setGender(gender);
            }
        });
    }

    void GameAnalytics::setBirthYear(int birthYear)
    {
        threading::GAThreading::performTaskOnGAThread([birthYear]()
        {
            if (!isSdkReady(false))
            {
                return;
            }
            if (validators::GAValidator::validateBirthyear(birthYear))
            {
                state::GAState::setBirthYear(birthYear);
            }
        });
    }

    void GameAnalytics::startSession()
    {
        threading::GAThreading::performTaskOnGAThread([]() 
        {
            if(state::GAState::useManualSessionHandling())
            {
                if (!state::GAState::isInitialized())
                {
                    return;
                }

                if(state::GAState::isEnabled() && state::GAState::sessionIsStarted())
                {
                    state::GAState::endSessionAndStopQueue();
                }

                state::GAState::resumeSessionAndStartQueue();
            }
        });
    }

    void GameAnalytics::endSession()
    {
        threading::GAThreading::performTaskOnGAThread([]() 
        {
            if(state::GAState::useManualSessionHandling())
            {
                state::GAState::endSessionAndStopQueue();
            }
        });
    }


    // -------------- SET GAME STATE CHANGES --------------- //

    void GameAnalytics::onResume()
    {
        threading::GAThreading::performTaskOnGAThread([]() 
        {
            if(!state::GAState::useManualSessionHandling())
            {
                state::GAState::resumeSessionAndStartQueue();
            }
        });
    }

    void GameAnalytics::onStop()
    {
        threading::GAThreading::performTaskOnGAThread([]() 
        {
            if(!state::GAState::useManualSessionHandling())
            {
                state::GAState::endSessionAndStopQueue();
            }
        });
    }

    // --------------PRIVATE HELPERS -------------- //

    bool GameAnalytics::isSdkReady(bool needsInitialized)
    {
        return isSdkReady(needsInitialized, true);
    }

    bool GameAnalytics::isSdkReady(bool needsInitialized, bool warn)
    {
        return isSdkReady(needsInitialized, warn, "");
    }

    bool GameAnalytics::isSdkReady(bool needsInitialized, bool warn, std::string message)
    {
        if (!message.empty())
        {
            message = message + ": ";
        }

        // Make sure database is ready
        if (!store::GAStore::sharedInstance()->getTableReady())
        {
            if (warn)
            {
                logging::GALogger::w(message + "Datastore not initialized");
            }
            return false;
        }
        // Is SDK initialized
        if (needsInitialized && !state::GAState::isInitialized())
        {
            if (warn)
            {
                logging::GALogger::w(message + "SDK is not initialized");
            }
            return false;
        }
        // Is SDK enabled
        if (needsInitialized && !state::GAState::isEnabled())
        {
            if (warn)
            {
                logging::GALogger::w(message + "SDK is disabled");
            }
            return false;
        }
        return true;
    }

} // namespace gameanalytics
