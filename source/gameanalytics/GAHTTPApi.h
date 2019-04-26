//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include <map>
#include "Foundation/GASingleton.h"
#include "rapidjson/document.h"
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
            const std::string createPayloadData(const std::string& payload, bool gzip);

#if USE_UWP
            const std::string createRequest(Windows::Web::Http::HttpRequestMessage^ message, const std::string& url, const std::string& payloadData, bool gzip);
            EGAHTTPApiResponse processRequestResponse(Windows::Web::Http::HttpResponseMessage^ response, const std::string& requestId);
            concurrency::task<Windows::Storage::Streams::InMemoryRandomAccessStream^> createStream(std::string data);
#else
            const std::string createRequest(curl::curl_easy& curl, curl::curl_header& header, const std::string& url, const std::string& payloadData, bool gzip);
            EGAHTTPApiResponse processRequestResponse(curl::curl_easy& curl, const std::string& body, const std::string& requestId);
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
