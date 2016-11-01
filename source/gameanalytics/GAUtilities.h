//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include <json/json.h>
#include "defines.h"

namespace gameanalytics
{
    namespace utilities
    {
        class GAUtilities
        {
         public:
            static STRING_TYPE generateUUID();
            static STRING_TYPE hmacWithKey(const STRING_TYPE& key, const STRING_TYPE& data);
            static STRING_TYPE jsonToString(const Json::Value& obj);
            static STRING_TYPE arrayOfObjectsToJsonString(const std::vector<Json::Value>& arr);
            static Json::Value jsonFromString(const STRING_TYPE& string);
            static bool stringMatch(const STRING_TYPE& string, const STRING_TYPE& pattern);
            static STRING_TYPE gzipCompress(const STRING_TYPE& data);

            // added for C++ port
            static STRING_TYPE uppercaseString(STRING_TYPE s);
            static STRING_TYPE lowercaseString(STRING_TYPE s);
            static bool stringVectorContainsString(std::vector<STRING_TYPE> vector, STRING_TYPE search);
            static Json::Int64 timeIntervalSince1970();
            static STRING_TYPE joinStringArray(const std::vector<STRING_TYPE>& v, const STRING_TYPE& delimiter = TEXT(", "));
            static int base64_needed_encoded_length(int length_of_data);
            static void base64_encode(const unsigned char * src, int src_len, unsigned char *buf_);

            // tries to convert s into T
            template <typename T>
            static T parseString(const STRING_TYPE& s)
            {
                std::istringstream i(s);
                T parsed_value;
                i >> parsed_value;
                return parsed_value;
            }
        };
    }
}


