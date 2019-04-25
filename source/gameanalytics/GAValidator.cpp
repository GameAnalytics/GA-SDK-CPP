//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#include "GAValidator.h"
#include "GAUtilities.h"
#include "GAEvents.h"
#include "GAState.h"
#include "GALogger.h"
#include "GAHTTPApi.h"
#include <string.h>
#include <stdio.h>

#include <map>

namespace gameanalytics
{
    namespace validators
    {
        bool GAValidator::validateBusinessEvent(
            const char* currency,
            long amount,
            const char* cartType,
            const char* itemType,
            const char* itemId)
        {
            // validate currency
            if (!GAValidator::validateCurrency(currency))
            {
                char s[257] = "";
                snprintf(s, sizeof(s), "Validation fail - business event - currency: Cannot be (null) and need to be A-Z, 3 characters and in the standard at openexchangerates.org. Failed currency: %s", currency);
                logging::GALogger::w(s);
                return false;
            }

            if (amount < 0)
            {
                logging::GALogger::w("Validation fail - business event - amount. Cannot be less than 0. String: " + std::to_string(amount));
                return false;
            }

            // validate cartType
            if (!GAValidator::validateShortString(cartType, true))
            {
                char s[257] = "";
                snprintf(s, sizeof(s), "Validation fail - business event - cartType. Cannot be above 32 length. String: %s", cartType);
                logging::GALogger::w(s);
                return false;
            }

            // validate itemType length
            if (!GAValidator::validateEventPartLength(itemType, false))
            {
                char s[257] = "";
                snprintf(s, sizeof(s), "Validation fail - business event - itemType: Cannot be (null), empty or above 64 characters. String: %s", itemType);
                logging::GALogger::w(s);
                return false;
            }

            // validate itemType chars
            if (!GAValidator::validateEventPartCharacters(itemType))
            {
                char s[257] = "";
                snprintf(s, sizeof(s), "Validation fail - business event - itemType: Cannot contain other characters than A-z, 0-9, -_., ()!?. String: %s", itemType);
                logging::GALogger::w(s);
                return false;
            }

            // validate itemId
            if (!GAValidator::validateEventPartLength(itemId, false))
            {
                char s[257] = "";
                snprintf(s, sizeof(s), "Validation fail - business event - itemId. Cannot be (null), empty or above 64 characters. String: %s", itemId);
                logging::GALogger::w(s);
                return false;
            }

            if (!GAValidator::validateEventPartCharacters(itemId))
            {
                char s[257] = "";
                snprintf(s, sizeof(s), "Validation fail - business event - itemId: Cannot contain other characters than A-z, 0-9, -_., ()!?. String: %s", itemId);
                logging::GALogger::w(s);
                return false;
            }

            return true;
        }

        bool GAValidator::validateResourceEvent(
            EGAResourceFlowType flowType,
            const char* currency,
            double amount,
            const char* itemType,
            const char* itemId
            )
        {
            if (events::GAEvents::resourceFlowTypeString(flowType).empty())
            {
                logging::GALogger::w("Validation fail - resource event - flowType: Invalid flow type.");
                return false;
            }
            if (utilities::GAUtilities::isStringNullOrEmpty(currency))
            {
                logging::GALogger::w("Validation fail - resource event - currency: Cannot be (null)");
                return false;
            }
            if (!state::GAState::hasAvailableResourceCurrency(currency))
            {
                int size = strlen(currency) + 129;
                char s[size];
                snprintf(s, size, "Validation fail - resource event - currency: Not found in list of pre-defined available resource currencies. String: %s", currency);
                logging::GALogger::w(s);
                return false;
            }
            if (!(amount > 0))
            {
                logging::GALogger::w("Validation fail - resource event - amount: Float amount cannot be 0 or negative. Value: " + std::to_string(amount));
                return false;
            }
            if (utilities::GAUtilities::isStringNullOrEmpty(itemType))
            {
                logging::GALogger::w("Validation fail - resource event - itemType: Cannot be (null)");
                return false;
            }
            if (!GAValidator::validateEventPartLength(itemType, false))
            {
                int size = strlen(itemType) + 129;
                char s[size];
                snprintf(s, size, "Validation fail - resource event - itemType: Cannot be (null), empty or above 64 characters. String: %s", itemType);
                logging::GALogger::w(s);
                return false;
            }
            if (!GAValidator::validateEventPartCharacters(itemType))
            {
                int size = strlen(itemType) + 129;
                char s[size];
                snprintf(s, size, "Validation fail - resource event - itemType: Cannot contain other characters than A-z, 0-9, -_., ()!?. String: %s", itemType);
                logging::GALogger::w(s);
                return false;
            }
            if (!state::GAState::hasAvailableResourceItemType(itemType))
            {
                int size = strlen(itemType) + 129;
                char s[size];
                snprintf(s, size, "Validation fail - resource event - itemType: Not found in list of pre-defined available resource itemTypes. String: %s", itemType);
                logging::GALogger::w(s);
                return false;
            }
            if (!GAValidator::validateEventPartLength(itemId, false))
            {
                int size = strlen(itemId) + 129;
                char s[size];
                snprintf(s, size, "Validation fail - resource event - itemId: Cannot be (null), empty or above 64 characters. String: %s", itemId);
                logging::GALogger::w(s);
                return false;
            }
            if (!GAValidator::validateEventPartCharacters(itemId))
            {
                int size = strlen(itemId) + 129;
                char s[size];
                snprintf(s, size, "Validation fail - resource event - itemId: Cannot contain other characters than A-z, 0-9, -_., ()!?. String: %s", itemId);
                logging::GALogger::w(s);
                return false;
            }
            return true;
        }

        bool GAValidator::validateProgressionEvent(
            EGAProgressionStatus progressionStatus,
            const char* progression01,
            const char* progression02,
            const char* progression03
            )
        {
            if (events::GAEvents::progressionStatusString(progressionStatus).empty())
            {
                logging::GALogger::w("Validation fail - progression event: Invalid progression status.");
                return false;
            }

            // Make sure progressions are defined as either 01, 01+02 or 01+02+03
            if (!utilities::GAUtilities::isStringNullOrEmpty(progression03) && !(!utilities::GAUtilities::isStringNullOrEmpty(progression02) || utilities::GAUtilities::isStringNullOrEmpty(progression01)))
            {
                logging::GALogger::w("Validation fail - progression event: 03 found but 01+02 are invalid. Progression must be set as either 01, 01+02 or 01+02+03.");
                return false;
            }
            else if (!utilities::GAUtilities::isStringNullOrEmpty(progression02) && utilities::GAUtilities::isStringNullOrEmpty(progression01))
            {
                logging::GALogger::w("Validation fail - progression event: 02 found but not 01. Progression must be set as either 01, 01+02 or 01+02+03");
                return false;
            }
            else if (utilities::GAUtilities::isStringNullOrEmpty(progression01))
            {
                logging::GALogger::w("Validation fail - progression event: progression01 not valid. Progressions must be set as either 01, 01+02 or 01+02+03");
                return false;
            }

            // progression01 (required)
            if (!GAValidator::validateEventPartLength(progression01, false))
            {
                char s[257] = "";
                snprintf(s, sizeof(s), "Validation fail - progression event - progression01: Cannot be (null), empty or above 64 characters. String: %s", progression01);
                logging::GALogger::w(s);
                return false;
            }
            if (!GAValidator::validateEventPartCharacters(progression01))
            {
                char s[257] = "";
                snprintf(s, sizeof(s), "Validation fail - progression event - progression01: Cannot contain other characters than A-z, 0-9, -_., ()!?. String: %s", progression01);
                logging::GALogger::w(s);
                return false;
            }
            // progression02
            if (strlen(progression02) > 0)
            {
                if (!GAValidator::validateEventPartLength(progression02, true))
                {
                    char s[257] = "";
                    snprintf(s, sizeof(s), "Validation fail - progression event - progression02: Cannot be empty or above 64 characters. String: %s", progression02);
                    logging::GALogger::w(s);
                    return false;
                }
                if (!GAValidator::validateEventPartCharacters(progression02))
                {
                    char s[257] = "";
                    snprintf(s, sizeof(s), "Validation fail - progression event - progression02: Cannot contain other characters than A-z, 0-9, -_., ()!?. String: %s", progression02);
                    logging::GALogger::w(s);
                    return false;
                }
            }
            // progression03
            if (strlen(progression03) > 0)
            {
                if (!GAValidator::validateEventPartLength(progression03, true))
                {
                    char s[257] = "";
                    snprintf(s, sizeof(s), "Validation fail - progression event - progression03: Cannot be empty or above 64 characters. String: %s", progression03);
                    logging::GALogger::w(s);
                    return false;
                }
                if (!GAValidator::validateEventPartCharacters(progression03))
                {
                    char s[257] = "";
                    snprintf(s, sizeof(s), "Validation fail - progression event - progression03: Cannot contain other characters than A-z, 0-9, -_., ()!?. String: %s", progression03);
                    logging::GALogger::w(s);
                    return false;
                }
            }
            return true;
        }


        bool GAValidator::validateDesignEvent(const char* eventId, double value)
        {
            if (!GAValidator::validateEventIdLength(eventId))
            {
                char s[257] = "";
                snprintf(s, sizeof(s), "Validation fail - design event - eventId: Cannot be (null) or empty. Only 5 event parts allowed seperated by :. Each part need to be 32 characters or less. String: %s", eventId);
                logging::GALogger::w(s);
                return false;
            }
            if (!GAValidator::validateEventIdCharacters(eventId))
            {
                char s[257] = "";
                snprintf(s, sizeof(s), "Validation fail - design event - eventId: Non valid characters. Only allowed A-z, 0-9, -_., ()!?. String: %s", eventId);
                logging::GALogger::w(s);
                return false;
            }
            // value: allow 0, negative and nil (not required)
            return true;
        }


        bool GAValidator::validateErrorEvent(EGAErrorSeverity severity, const std::string& message)
        {
            if (events::GAEvents::errorSeverityString(severity).empty())
            {
                logging::GALogger::w("Validation fail - error event - severity: Severity was unsupported value.");
                return false;
            }
            if (!GAValidator::validateLongString(message, true))
            {
                logging::GALogger::w("Validation fail - error event - message: Message cannot be above 8192 characters. message=" + message);
                return false;
            }
            return true;
        }

        bool GAValidator::validateSdkErrorEvent(const char* gameKey, const char* gameSecret, http::EGASdkErrorType type)
        {
            if(!validateKeys(gameKey, gameSecret))
            {
                return false;
            }

            if (http::GAHTTPApi::sdkErrorTypeToString(type).empty())
            {
                logging::GALogger::w("Validation fail - sdk error event - type: Type was unsupported value.");
                return false;
            }
            return true;
        }


        // event params
        bool GAValidator::validateKeys(const char* gameKey, const char* gameSecret)
        {
            if (utilities::GAUtilities::stringMatch(gameKey, "^[A-z0-9]{32}$"))
            {
                if (utilities::GAUtilities::stringMatch(gameSecret, "^[A-z0-9]{40}$"))
                {
                    return true;
                }
            }
            return false;
        }

        bool GAValidator::validateCurrency(const char* currency)
        {
            if (strlen(currency) == 0)
            {
                return false;
            }
            if (!utilities::GAUtilities::stringMatch(currency, "^[A-Z]{3}$"))
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateEventPartLength(const char* eventPart, bool allowNull)
        {
            int size = strlen(eventPart);
            if (allowNull == true && size == 0)
            {
                return true;
            }

            if (size == 0)
            {
                return false;
            }

            if (size > 64)
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateEventPartCharacters(const char* eventPart)
        {
            if (!utilities::GAUtilities::stringMatch(eventPart, "^[A-Za-z0-9\\s\\-_\\.\\(\\)\\!\\?]{1,64}$"))
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateEventIdLength(const char* eventId)
        {
            if (strlen(eventId) == 0)
            {
                return false;
            }

            if (!utilities::GAUtilities::stringMatch(eventId, "^[^:]{1,64}(?::[^:]{1,64}){0,4}$"))
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateEventIdCharacters(const char* eventId)
        {
            if (strlen(eventId) == 0)
            {
                return false;
            }

            if (!utilities::GAUtilities::stringMatch(eventId, "^[A-Za-z0-9\\s\\-_\\.\\(\\)\\!\\?]{1,64}(:[A-Za-z0-9\\s\\-_\\.\\(\\)\\!\\?]{1,64}){0,4}$"))
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateShortString(const std::string& shortString, bool canBeEmpty = false)
        {
            // String is allowed to be empty or nil
            if (canBeEmpty && shortString.empty())
            {
                return true;
            }

            if (shortString.empty() || shortString.length() > 32)
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateString(const std::string& string, bool canBeEmpty = false)
        {
            // String is allowed to be empty or nil
            if (canBeEmpty && string.empty())
            {
                return true;
            }

            if (string.empty() || string.length() > 64)
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateLongString(const std::string& longString, bool canBeEmpty = false)
        {
            // String is allowed to be empty
            if (canBeEmpty && longString.empty())
            {
                return true;
            }

            if (longString.empty() || longString.length() > 8192)
            {
                return false;
            }
            return true;
        }

        // validate wrapper version, build, engine version, store
        bool GAValidator::validateSdkWrapperVersion(const std::string& wrapperVersion)
        {
            if (!utilities::GAUtilities::stringMatch(wrapperVersion, "^(unreal|corona|cocos2d|lumberyard|air|gamemaker|defold) [0-9]{0,5}(\\.[0-9]{0,5}){0,2}$"))
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateBuild(const std::string& build)
        {
            if (!GAValidator::validateShortString(build))
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateEngineVersion(const std::string& engineVersion)
        {
            if (!utilities::GAUtilities::stringMatch(engineVersion, "^(unreal|corona|cocos2d|lumberyard|gamemaker|defold) [0-9]{0,5}(\\.[0-9]{0,5}){0,2}$"))
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateStore(const std::string& store)
        {
            return utilities::GAUtilities::stringMatch(store, "^(apple|google_play)$");
        }

        bool GAValidator::validateConnectionType(const std::string& connectionType)
        {
            return utilities::GAUtilities::stringMatch(connectionType, "^(wwan|wifi|lan|offline)$");
        }

        // dimensions
        bool GAValidator::validateCustomDimensions(const StringVector& customDimensions)
        {
            return GAValidator::validateArrayOfStrings(customDimensions, 20, 32, false, "custom dimensions");

        }

        bool GAValidator::validateResourceCurrencies(const StringVector& resourceCurrencies)
        {
            if (!GAValidator::validateArrayOfStrings(resourceCurrencies, 20, 64, false, "resource currencies"))
            {
                return false;
            }

            // validate each string for regex
            for (CharArray resourceCurrency : resourceCurrencies.getVector())
            {
                if (!utilities::GAUtilities::stringMatch(resourceCurrency, "^[A-Za-z]+$"))
                {
                    logging::GALogger::w("resource currencies validation failed: a resource currency can only be A-Z, a-z. String was: " + resourceCurrency);
                    return false;
                }
            }
            return true;
        }

        bool GAValidator::validateResourceItemTypes(const StringVector& resourceItemTypes)
        {
            if (!GAValidator::validateArrayOfStrings(resourceItemTypes, 20, 32, false, "resource item types"))
            {
                return false;
            }

            // validate each resourceItemType for eventpart validation
            for (std::string resourceItemType : resourceItemTypes)
            {
                if (!GAValidator::validateEventPartCharacters(resourceItemType))
                {
                    logging::GALogger::w("resource item types validation failed: a resource item type cannot contain other characters than A-z, 0-9, -_., ()!?. String was: " + resourceItemType);
                    return false;
                }
            }
            return true;
        }


        bool GAValidator::validateDimension01(const std::string& dimension01)
        {
            // allow nil
            if (dimension01.empty())
            {
                return true;
            }
            if (!state::GAState::hasAvailableCustomDimensions01(dimension01))
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateDimension02(const std::string& dimension02)
        {
            // allow nil
            if (dimension02.empty())
            {
                return true;
            }
            if (!state::GAState::hasAvailableCustomDimensions02(dimension02))
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateDimension03(const std::string& dimension03)
        {
            // allow nil
            if (dimension03.empty())
            {
                return true;
            }
            if (!state::GAState::hasAvailableCustomDimensions03(dimension03))
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateArrayOfStrings(
            const StringVector& arrayOfStrings,
            unsigned long maxCount,
            unsigned long maxStringLength,
            bool allowNoValues,
            const std::string& logTag
            )
        {
            std::string arrayTag = logTag;

            // use arrayTag to annotate warning log
            if (arrayTag.empty())
            {
                arrayTag = "Array";
            }

            // check if empty
            if (allowNoValues == false && arrayOfStrings.size() == 0)
            {
                logging::GALogger::w(arrayTag + " validation failed: array cannot be empty. ");
                return false;
            }

            // check if exceeding max count
            if (maxCount && maxCount > static_cast<int>(0) && arrayOfStrings.size() > maxCount)
            {
                logging::GALogger::w(arrayTag + " validation failed: array cannot exceed " + std::to_string(maxCount) + " values. It has " + std::to_string(arrayOfStrings.size()) + " values.");
                return false;
            }

            // validate each string
            for (std::string arrayString : arrayOfStrings)
            {
                auto stringLength = arrayString.length();
                // check if empty (not allowed)
                if (stringLength == 0)
                {
                    logging::GALogger::w(arrayTag + " validation failed: contained an empty string.");
                    return false;
                }

                // check if exceeding max length
                if (maxStringLength && maxStringLength > static_cast<int>(0) && stringLength > maxStringLength)
                {
                    logging::GALogger::w(arrayTag + " validation failed: a string exceeded max allowed length (which is: " + std::to_string(maxStringLength) + "). String was: " + arrayString);
                    return false;
                }
            }
            return true;
        }

        bool GAValidator::validateFacebookId(const std::string& facebookId)
        {
            if (!GAValidator::validateString(facebookId))
            {
                logging::GALogger::w("Validation fail - facebook id: id cannot be (null), empty or above 64 characters.");
                return false;
            }
            return true;
        }

        bool GAValidator::validateGender(EGAGender gender)
        {
            switch (gender) {
            case Male:
                return true;
            case Female:
                return true;
            }

            logging::GALogger::w("Validation fail - gender: Has to be 'male' or 'female'.");
            return false;
        }

        bool GAValidator::validateBirthyear(int64_t birthYear)
        {
            if (birthYear < 0 || birthYear > 9999)
            {
                logging::GALogger::w("Validation fail - birthYear: Cannot be (null) or invalid range.");
                return false;
            }
            return true;
        }

        bool GAValidator::validateClientTs(int64_t clientTs)
        {
            // server regex: ^([0-9]{10,11})$
            if (clientTs < 1000000000 || clientTs > 9999999999)
            {
                return false;
            }
            return true;
        }

        bool GAValidator::validateUserId(const std::string& uId)
        {
            if (uId.empty())
            {
                logging::GALogger::w("Validation fail - user id cannot be empty.");
                return false;
            }
            return true;
        }

        void GAValidator::validateAndCleanInitRequestResponse(const rapidjson::Value& initResponse, rapidjson::Document& out)
        {
            // make sure we have a valid dict
            if (initResponse.IsNull())
            {
                logging::GALogger::w("validateInitRequestResponse failed - no response dictionary.");
                rapidjson::Value v(rapidjson::kObjectType);
                out.SetNull();
                return;
            }

            out.SetObject();
            rapidjson::Document::AllocatorType& allocator = out.GetAllocator();

            // validate enabled field
            if(initResponse.HasMember("enabled") && initResponse["enabled"].IsBool())
            {
                out.AddMember("enabled", initResponse["enabled"].GetBool(), allocator);
            }

            // validate server_ts
            if (initResponse.HasMember("server_ts") && initResponse["server_ts"].IsNumber())
            {
                int64_t serverTsNumber = initResponse["server_ts"].GetInt64();
                if (serverTsNumber > 0)
                {
                    out.AddMember("server_ts", serverTsNumber, allocator);
                }
            }

            if (initResponse.HasMember("configurations") && initResponse["configurations"].IsArray())
            {
                rapidjson::Value configurations = rapidjson::Value(initResponse["configurations"], allocator);
                out.AddMember("configurations", configurations, allocator);
            }
        }
    }
}
