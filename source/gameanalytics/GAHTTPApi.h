//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#define BOOST_NETWORK_ENABLE_HTTPS
#include "curl_easy.h"
#include "curl_header.h"
#include <vector>
#include <string>
#include "Foundation/GASingleton.h"
#include <json/json.h>
#include "defines.h"

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

            EGAHTTPApiResponse requestInitReturningDict(Json::Value& dict);
            EGAHTTPApiResponse sendEventsInArray(const std::vector<Json::Value>& eventArray, Json::Value& dict);
            void sendSdkErrorEvent(EGASdkErrorType type);
            static const STRING_TYPE sdkErrorTypeToString(EGASdkErrorType value);

         private:
            const STRING_TYPE createPayloadData(const STRING_TYPE& payload, bool gzip);
            const STRING_TYPE createRequest(curl::curl_easy& curl, curl::curl_header& header, const STRING_TYPE& url, const STRING_TYPE& payloadData, bool gzip);
            EGAHTTPApiResponse processRequestResponse(curl::curl_easy& curl, const STRING_TYPE& body, const STRING_TYPE& requestId);

            STRING_TYPE protocol;
            STRING_TYPE hostName;
            STRING_TYPE version;
            STRING_TYPE baseUrl;
            STRING_TYPE initializeUrlPath;
            STRING_TYPE eventsUrlPath;
            bool useGzip;
        };
    }
}
