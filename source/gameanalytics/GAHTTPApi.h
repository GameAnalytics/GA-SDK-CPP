//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include "Foundation/GASingleton.h"
#include <json/json.h>
#include <utility>
#if USE_UWP
#include <ppltasks.h>
#else
#include "curl_easy.h"
#include "curl_header.h"
#endif

namespace gameanalytics
{
    namespace http
    {
        enum EGAHTTPApiResponse
        {
            // client
            NoResponse = 0,
            BadResponse = 1,
            RequestTimeout = 2, // 408
            JsonEncodeFailed = 3,
            JsonDecodeFailed = 4,
            // server
            InternalServerError = 5,
            BadRequest = 6, // 400
            Unauthorized = 7, // 401
            UnknownResponseCode = 8,
            Ok = 9
        };

        enum EGASdkErrorType
        {
            Undefined = 0,
            Rejected = 1
        };
        
        struct CurlFetchStruct 
        {
            char *payload;
            size_t size;
        };

        class GAHTTPApi : public GASingleton<GAHTTPApi>
        {
        public:
            GAHTTPApi();

#if USE_UWP
            concurrency::task<std::pair<EGAHTTPApiResponse, Json::Value>> requestInitReturningDict();
            concurrency::task<std::pair<EGAHTTPApiResponse, Json::Value>> sendEventsInArray(const std::vector<Json::Value>& eventArray);
#else
            std::pair<EGAHTTPApiResponse, Json::Value> requestInitReturningDict();
            std::pair<EGAHTTPApiResponse, Json::Value> sendEventsInArray(const std::vector<Json::Value>& eventArray);
#endif
            void sendSdkErrorEvent(EGASdkErrorType type);

            static const std::string GAHTTPApi::sdkErrorTypeToString(EGASdkErrorType value)
            {
                switch (value) {
                case Rejected:
                    return "rejected";
                default:
                    break;
                }
                return{};
            }

         private:
            const std::string createPayloadData(const std::string& payload, bool gzip);

#if USE_UWP
            const std::string createRequest(Windows::Web::Http::HttpRequestMessage^ message, const std::string& url, const std::string& payloadData, bool gzip);
            EGAHTTPApiResponse processRequestResponse(Windows::Web::Http::HttpResponseMessage^ response, const std::string& requestId);
#else
            const std::string createRequest(curl::curl_easy& curl, curl::curl_header& header, const std::string& url, const std::string& payloadData, bool gzip);
            EGAHTTPApiResponse processRequestResponse(curl::curl_easy& curl, const std::string& body, const std::string& requestId);
#endif

            std::string protocol;
            std::string hostName;
            std::string version;
            std::string baseUrl;
            std::string initializeUrlPath;
            std::string eventsUrlPath;
            bool useGzip;
            static const int MaxCount;
            static std::map<EGASdkErrorType, int> countMap;
#if USE_UWP
            Windows::Web::Http::HttpClient^ httpClient;
#endif
        };
    }
}
