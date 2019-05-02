//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <vector>
#include <map>
#include "Foundation/GASingleton.h"
#include "rapidjson/document.h"
#if USE_UWP
#include <ppltasks.h>
#else
#include <curl/curl.h>
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

        struct ResponseData
        {
            char *ptr;
            size_t len;
        };

        class GAHTTPApi : public GASingleton<GAHTTPApi>
        {
        public:
            GAHTTPApi();
            ~GAHTTPApi();

#if USE_UWP
            concurrency::task<Void> requestInitReturningDict(EGAHTTPApiResponse& response_out, rapidjson::Document& json_out);
            concurrency::task<Void> sendEventsInArray(EGAHTTPApiResponse& response_out, rapidjson::Value& json_out, const rapidjson::Value& eventArray);
#else
            void requestInitReturningDict(EGAHTTPApiResponse& response_out, rapidjson::Document& json_out);
            void sendEventsInArray(EGAHTTPApiResponse& response_out, rapidjson::Value& json_out, const rapidjson::Value& eventArray);
#endif
            void sendSdkErrorEvent(EGASdkErrorType type);

            static void sdkErrorTypeToString(EGASdkErrorType value, char* out)
            {
                switch (value) {
                case Rejected:
                    snprintf(out, 9, "%s", "rejected");
                    return;
                default:
                    break;
                }
                snprintf(out, 9, "%s", "");
            }

         private:
            std::vector<char> createPayloadData(const char* payload, bool gzip);

#if USE_UWP
            std::vector<char> createRequest(Windows::Web::Http::HttpRequestMessage^ message, const std::string& url, const std::string& payloadData, bool gzip);
            EGAHTTPApiResponse processRequestResponse(Windows::Web::Http::HttpResponseMessage^ response, const std::string& requestId);
            concurrency::task<Windows::Storage::Streams::InMemoryRandomAccessStream^> createStream(std::string data);
#else
            std::vector<char> createRequest(CURL *curl, const char* url, const std::vector<char>& payloadData, bool gzip);
            EGAHTTPApiResponse processRequestResponse(long statusCode, const char* body, const char* requestId);
#endif

            static void initBaseUrl();

            static char protocol[];
            static char hostName[];
            static char version[];
            static char baseUrl[];
            static char initializeUrlPath[];
            static char eventsUrlPath[];
            bool useGzip;
            static const int MaxCount;
            static std::map<EGASdkErrorType, int> countMap;
#if USE_UWP
            Windows::Web::Http::HttpClient^ httpClient;
#endif
        };

#if USE_UWP
        ref class GANetworkStatus sealed
        {
        internal:
            static void NetworkInformationOnNetworkStatusChanged(Platform::Object^ sender);
            static void CheckInternetAccess();
            static bool hasInternetAccess;
        };
#endif
    }
}
