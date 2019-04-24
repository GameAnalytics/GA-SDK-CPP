//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include "rapidjson/document.h"
#if USE_UWP
#include <locale>
#include <codecvt>
#endif

namespace gameanalytics
{
    namespace utilities
    {
        class GAUtilities
        {
         public:
            static std::string getPathSeparator();
            static std::string generateUUID();
            static std::string hmacWithKey(const std::string& key, const std::string& data);
            static bool stringMatch(const std::string& string, const std::string& pattern);
            static std::string gzipCompress(const std::string& data);

            // added for C++ port
            static std::string uppercaseString(std::string s);
            static std::string lowercaseString(std::string s);
            static bool stringVectorContainsString(std::vector<std::string> vector, std::string search);
            static int64_t timeIntervalSince1970();
            static std::string joinStringArray(const std::vector<std::string>& v, const std::string& delimiter = ", ");
#if !USE_UWP
            static int base64_needed_encoded_length(int length_of_data);
            static void base64_encode(const unsigned char * src, int src_len, unsigned char *buf_);
#endif

#if USE_UWP
            inline static std::string ws2s(const std::wstring wstr)
            {
                return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
            }

            inline static std::wstring s2ws(const std::string str)
            {
                return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
            }
#endif

            // tries to convert s into T
            template <typename T>
            static T parseString(const std::string& s)
            {
                std::istringstream i(s);
                T parsed_value;
                i >> parsed_value;
                return parsed_value;
            }
        };
    }
}
