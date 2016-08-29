//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#define BOOST_NETWORK_ENABLE_HTTPS
#include <boost/network/include/http/client.hpp>
#include <boost/network.hpp>
#include <vector>
#include <string>
#include "Foundation/GASingleton.h"
#include <json/json.h>

using namespace boost::network;
using namespace boost::network::http;

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

        class GAHTTPApi : public GASingleton<GAHTTPApi>
        {
         public:
            GAHTTPApi();

            EGAHTTPApiResponse requestInitReturningDict(Json::Value& dict);
            EGAHTTPApiResponse sendEventsInArray(const std::vector<Json::Value>& eventArray, Json::Value& dict);
            void sendSdkErrorEvent(EGASdkErrorType type);
            static const std::string sdkErrorTypeToString(EGASdkErrorType value);

         private:
            const std::string createPayloadData(const std::string& payload, bool gzip);
            client::request createRequest(const std::string& url, const std::string& payloadData, bool gzip);
            EGAHTTPApiResponse processRequestResponse(client::response& response, const std::string& body, const std::string& requestId);

            std::string protocol;
            std::string hostName;
            std::string version;
            std::string baseUrl;
            std::string initializeUrlPath;
            std::string eventsUrlPath;
            bool useGzip;
        };
    }
}