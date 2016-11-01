//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include <json/json.h>
#include "GameAnalytics.h"
#include "GAHttpApi.h"

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
                const STRING_TYPE& currency,
                long amount,
                const STRING_TYPE& cartType,
                const STRING_TYPE& itemType,
                const STRING_TYPE& itemId
                );

            static bool validateResourceEvent(
                EGAResourceFlowType flowType,
                const STRING_TYPE& currency,
                double amount,
                const STRING_TYPE& itemType,
                const STRING_TYPE& itemId
                );


            static bool validateProgressionEvent(
                EGAProgressionStatus progressionStatus,
                const STRING_TYPE& progression01,
                const STRING_TYPE& progression02,
                const STRING_TYPE& progression03
                );

            static bool validateDesignEvent(
                const STRING_TYPE& eventId,
                double value
                );

            static bool validateErrorEvent(
                EGAErrorSeverity severity,
                const STRING_TYPE& message
                );

            static bool validateSdkErrorEvent(const STRING_TYPE& gameKey, const STRING_TYPE& gameSecret, http::EGASdkErrorType type);


            // -------------------- HELPERS --------------------- //

            // event params
            static bool validateKeys(const STRING_TYPE& gameKey, const STRING_TYPE& gameSecret);
            static bool validateCurrency(const STRING_TYPE& currency);
            static bool validateEventPartLength(const STRING_TYPE& eventPart, bool allowNull);
            static bool validateEventPartCharacters(const STRING_TYPE& eventPart);
            static bool validateEventIdLength(const STRING_TYPE& eventId);
            static bool validateEventIdCharacters(const STRING_TYPE& eventId);
            static bool validateShortString(const STRING_TYPE& shortString, bool canBeEmpty);
            static bool validateString(const STRING_TYPE& string, bool canBeEmpty);
            static bool validateLongString(const STRING_TYPE& longString, bool canBeEmpty);

            // validate wrapper version, build, engine version, store
            static bool validateSdkWrapperVersion(const STRING_TYPE& wrapperVersion);
            static bool validateBuild(const STRING_TYPE& build);
            static bool validateEngineVersion(const STRING_TYPE& engineVersion);
            static bool validateStore(const STRING_TYPE& store);
            static bool validateConnectionType(const STRING_TYPE& connectionType);

            // dimensions
            static bool validateCustomDimensions(const std::vector<STRING_TYPE>& customDimensions);

            // resource
            static bool validateResourceCurrencies(const std::vector<STRING_TYPE>& resourceCurrencies);
            static bool validateResourceItemTypes(const std::vector<STRING_TYPE>& resourceItemTypes);

            //static  bool validateCustomDimensionsWithDimension01:const STRING_TYPE& dimension01 andDimension02:const STRING_TYPE& dimension02 andDimension03:const STRING_TYPE& dimension03 andEventTag:const STRING_TYPE& eventTag;
            static bool validateDimension01(const STRING_TYPE& dimension01);
            static bool validateDimension02(const STRING_TYPE& dimension02);
            static bool validateDimension03(const STRING_TYPE& dimension03);

            static Json::Value validateAndCleanInitRequestResponse(Json::Value& initResponse);

            // array of strings
            static bool validateArrayOfStrings(
                const std::vector<STRING_TYPE>& arrayOfStrings,
                unsigned long maxCount,
                unsigned long maxStringLength,
                bool allowNoValues,
                const STRING_TYPE& arrayTag
                );
            // facebook id, gender, birthyear
            static bool validateFacebookId(const STRING_TYPE& facebookId);
            static bool validateGender(EGAGender gender);
            static bool validateBirthyear(long birthYear);
            static bool validateClientTs(Json::Int64 clientTs);

            static bool validateUserId(const STRING_TYPE& uId);
        };
    }
} 
