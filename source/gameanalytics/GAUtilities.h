//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <vector>
#include "rapidjson/document.h"
#include "GameAnalytics.h"
#if USE_UWP
#include <string>
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
            static const char* getPathSeparator();
            static void generateUUID(char* out);
            static void hmacWithKey(const char* key, const char* data, char* out);
            static bool stringMatch(const char* string, const char* pattern);
            static std::string gzipCompress(const std::string& data);

            // added for C++ port
            static bool isStringNullOrEmpty(const char* s);
            static void uppercaseString(char* s);
            static void lowercaseString(char* s);
            static bool stringVectorContainsString(const StringVector& vector, const char* search);
            static int64_t timeIntervalSince1970();
            static void printJoinStringArray(const StringVector& v, const char* format, const char* delimiter = ", ");
            static void setJsonKeyValue(rapidjson::Document& json, const char* key, const char* newValue);
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
        private:
            static char pathSeparator[];
        };
    }
}
