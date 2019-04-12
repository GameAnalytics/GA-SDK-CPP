//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include "rapidjson/document.h"
#include "GameAnalytics.h"
#include "GAHTTPApi.h"

namespace gameanalytics
{
    namespace validators
    {
        class GAValidator
        {
         public:
            // ---------- EVENTS -------------- //

            // user created events
            static bool validateBusinessEvent(
                const std::string& currency,
                long amount,
                const std::string& cartType,
                const std::string& itemType,
                const std::string& itemId
                );

            static bool validateResourceEvent(
                EGAResourceFlowType flowType,
                const std::string& currency,
                double amount,
                const std::string& itemType,
                const std::string& itemId
                );


            static bool validateProgressionEvent(
                EGAProgressionStatus progressionStatus,
                const std::string& progression01,
                const std::string& progression02,
                const std::string& progression03
                );

            static bool validateDesignEvent(
                const std::string& eventId,
                double value
                );

            static bool validateErrorEvent(
                EGAErrorSeverity severity,
                const std::string& message
                );

            static bool validateSdkErrorEvent(const std::string& gameKey, const std::string& gameSecret, http::EGASdkErrorType type);


            // -------------------- HELPERS --------------------- //

            // event params
            static bool validateKeys(const std::string& gameKey, const std::string& gameSecret);
            static bool validateCurrency(const std::string& currency);
            static bool validateEventPartLength(const std::string& eventPart, bool allowNull);
            static bool validateEventPartCharacters(const std::string& eventPart);
            static bool validateEventIdLength(const std::string& eventId);
            static bool validateEventIdCharacters(const std::string& eventId);
            static bool validateShortString(const std::string& shortString, bool canBeEmpty);
            static bool validateString(const std::string& string, bool canBeEmpty);
            static bool validateLongString(const std::string& longString, bool canBeEmpty);

            // validate wrapper version, build, engine version, store
            static bool validateSdkWrapperVersion(const std::string& wrapperVersion);
            static bool validateBuild(const std::string& build);
            static bool validateEngineVersion(const std::string& engineVersion);
            static bool validateStore(const std::string& store);
            static bool validateConnectionType(const std::string& connectionType);

            // dimensions
            static bool validateCustomDimensions(const std::vector<std::string>& customDimensions);

            // resource
            static bool validateResourceCurrencies(const std::vector<std::string>& resourceCurrencies);
            static bool validateResourceItemTypes(const std::vector<std::string>& resourceItemTypes);

            //static  bool validateCustomDimensionsWithDimension01:const std::string& dimension01 andDimension02:const std::string& dimension02 andDimension03:const std::string& dimension03 andEventTag:const std::string& eventTag;
            static bool validateDimension01(const std::string& dimension01);
            static bool validateDimension02(const std::string& dimension02);
            static bool validateDimension03(const std::string& dimension03);

            static rapidjson::Value& validateAndCleanInitRequestResponse(const rapidjson::Value& initResponse);

            // array of strings
            static bool validateArrayOfStrings(
                const std::vector<std::string>& arrayOfStrings,
                unsigned long maxCount,
                unsigned long maxStringLength,
                bool allowNoValues,
                const std::string& arrayTag
                );
            // facebook id, gender, birthyear
            static bool validateFacebookId(const std::string& facebookId);
            static bool validateGender(EGAGender gender);
            static bool validateBirthyear(long birthYear);
            static bool validateClientTs(long clientTs);

            static bool validateUserId(const std::string& uId);
        };
    }
}
