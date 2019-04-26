//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#if !USE_UWP
#include "GAHTTPApi.h"
#include "GAState.h"
#include "GALogger.h"
#include "GAUtilities.h"
#include "GAValidator.h"
#include "curl_exception.h"
#include "curl_ios.h"
#include <ostream>
#include <future>
#include <utility>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include <string.h>
#include <stdio.h>
#if USE_TIZEN
#include <net_connection.h>
#endif

namespace gameanalytics
{
    namespace http
    {
        // base url settings
        char GAHTTPApi::protocol[6] = "https";
        char GAHTTPApi::hostName[22] = "api.gameanalytics.com";

        char GAHTTPApi::version[3] = "v2";

        // create base url
        char GAHTTPApi::baseUrl[257] = "";

        // route paths
        char GAHTTPApi::initializeUrlPath[5] = "init";
        char GAHTTPApi::eventsUrlPath[7] = "events";

        // Constructor - setup the basic information for HTTP
        GAHTTPApi::GAHTTPApi()
        {
            initBaseUrl();
            // use gzip compression on JSON body
#if defined(_DEBUG)
            useGzip = false;
#else
            useGzip = true;
#endif
        }

        void GAHTTPApi::initBaseUrl()
        {
            snprintf(GAHTTPApi::baseUrl, sizeof(GAHTTPApi::baseUrl), "%s://%s/%s", protocol, hostName, version);
        }

        void GAHTTPApi::requestInitReturningDict(EGAHTTPApiResponse& response_out, rapidjson::Document& json_out)
        {
            const char* gameKey = state::GAState::getGameKey();

            // Generate URL
            char url[513] = "";
            snprintf(url, sizeof(url), "%s/%s/%s", baseUrl, gameKey, initializeUrlPath);
            snprintf(url, sizeof(url), "https://rubick.gameanalytics.com/v2/command_center?game_key=%s&interval_seconds=1000000", gameKey);

            {
                char s[513] = "";
                snprintf(s, sizeof(s), "Sending 'init' URL: %s", url);
                logging::GALogger::d(s);
            }

            rapidjson::Document initAnnotations;
            initAnnotations.SetObject();
            state::GAState::getInitAnnotations(initAnnotations);

            // make JSON string from data
            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                initAnnotations.Accept(writer);
            }
            const char* JSONstring = buffer.GetString();

            if (strlen(JSONstring) == 0)
            {
                response_out = JsonEncodeFailed;
                json_out.SetNull();
                return;
            }

            std::string payloadData = createPayloadData(JSONstring, false);
            std::ostringstream os;
            curl::curl_ios<std::ostringstream> writer(os);
            curl::curl_easy curl(writer);
            curl::curl_header header;
#if USE_TIZEN
            connection_h connection;
            int conn_err;
            conn_err = connection_create(&connection);
            if (conn_err != CONNECTION_ERROR_NONE)
            {
                response_out = NoResponse;
                json_out.SetNull();
                return;
            }
#endif

            std::string authorization = createRequest(curl, header, url, payloadData, useGzip);

            try
            {
                curl.perform();
            }
            catch (curl::curl_easy_exception error)
            {
                error.print_traceback();
                logging::GALogger::d(error.what());
            }

            std::string body = os.str();
            // process the response
            logging::GALogger::d("init request content : " + body);

            rapidjson::Document requestJsonDict;
            requestJsonDict.Parse(body.c_str());
            EGAHTTPApiResponse requestResponseEnum = processRequestResponse(curl, body, "Init");

            // if not 200 result
            if (requestResponseEnum != Ok && requestResponseEnum != BadRequest)
            {
                char s[1025] = "";
                snprintf(s, sizeof(s), "Failed Init Call. URL: %s, JSONString: %s, Authorization: %s", url, JSONstring, authorization.c_str());
                logging::GALogger::d(s);
#if USE_TIZEN
                connection_destroy(connection);
#endif
                response_out = requestResponseEnum;
                json_out.SetNull();
                return;
            }

            if (requestJsonDict.IsNull())
            {
                logging::GALogger::d("Failed Init Call. Json decoding failed");
#if USE_TIZEN
                connection_destroy(connection);
#endif
                response_out = JsonDecodeFailed;
                json_out.SetNull();
                return;
            }

            // print reason if bad request
            if (requestResponseEnum == BadRequest)
            {
                rapidjson::StringBuffer buffer;
                rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
                requestJsonDict.Accept(writer);
                logging::GALogger::d("Failed Init Call. Bad request. Response: " + std::string(buffer.GetString()));
                // return bad request result
#if USE_TIZEN
                connection_destroy(connection);
#endif
                response_out = requestResponseEnum;
                json_out.SetNull();
                return;
            }

            // validate Init call values
            validators::GAValidator::validateAndCleanInitRequestResponse(requestJsonDict, json_out);

            if (json_out.IsNull())
            {
#if USE_TIZEN
                connection_destroy(connection);
#endif
                response_out = BadResponse;
                json_out.SetNull();
                return;
            }

#if USE_TIZEN
            connection_destroy(connection);
#endif

            // all ok
            response_out = Ok;
        }

        void GAHTTPApi::sendEventsInArray(EGAHTTPApiResponse& response_out, rapidjson::Value& json_out, const rapidjson::Value& eventArray)
        {
            if (eventArray.Empty())
            {
                logging::GALogger::d("sendEventsInArray called with missing eventArray");
            }

            auto gameKey = state::GAState::getGameKey();

            // Generate URL
            char url[513] = "";
            snprintf(url, sizeof(url), "%s/%s/%s", baseUrl, gameKey, eventsUrlPath);

            {
                char s[513] = "";
                snprintf(url, sizeof(url), "Sending 'events' URL: %s", url);
                logging::GALogger::d(s);
            }


            // make JSON string from data
            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                eventArray.Accept(writer);
            }

            const char* JSONstring = buffer.GetString();

            if (strlen(JSONstring) == 0)
            {
                logging::GALogger::d("sendEventsInArray JSON encoding failed of eventArray");
                response_out = JsonEncodeFailed;
                json_out = rapidjson::Value();
            }

            std::string payloadData = createPayloadData(JSONstring, useGzip);
            std::ostringstream os;
            curl::curl_ios<std::ostringstream> writer(os);
            curl::curl_easy curl(writer);
            curl::curl_header header;
#if USE_TIZEN
            connection_h connection;
            int conn_err;
            conn_err = connection_create(&connection);
            if (conn_err != CONNECTION_ERROR_NONE)
            {
                response_out = NoResponse;
                json_out = rapidjson::Value();
            }
#endif
            std::string  authorization = createRequest(curl, header, url, payloadData, useGzip);

            try
            {
                curl.perform();
            }
            catch (curl::curl_easy_exception error)
            {
                auto errors = error.get_traceback();
                logging::GALogger::d(error.what());
                std::for_each(errors.begin(), errors.end(),[](const curl::curlcpp_traceback_object &value) {
                    logging::GALogger::d("ERROR: " + value.first + " ::::: FUNCTION: " + value.second);
                });
            }

            std::string body = os.str();
            logging::GALogger::d("body: " + body);

            EGAHTTPApiResponse requestResponseEnum = processRequestResponse(curl, body, "Events");

            // if not 200 result
            if (requestResponseEnum != Ok && requestResponseEnum != BadRequest)
            {
                int ss = strlen(url) + strlen(JSONstring) + strlen(authorization.c_str()) + 70;
                char s[ss];
                snprintf(s, ss, "Failed Events Call. URL: %s, JSONString: %s, Authorization: %s", url, JSONstring, authorization.c_str());
                logging::GALogger::d(s);
#if USE_TIZEN
                connection_destroy(connection);
#endif
                response_out = requestResponseEnum;
                json_out = rapidjson::Value();
            }

            // decode JSON
            rapidjson::Document requestJsonDict;
            requestJsonDict.Parse(body.c_str());

            if (requestJsonDict.IsNull())
            {
#if USE_TIZEN
                connection_destroy(connection);
#endif
                response_out = JsonDecodeFailed;
                json_out = rapidjson::Value();
            }

            // print reason if bad request
            if (requestResponseEnum == BadRequest)
            {
                rapidjson::StringBuffer buffer;
                rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
                requestJsonDict.Accept(writer);
                logging::GALogger::d("Failed Events Call. Bad request. Response: " + std::string(buffer.GetString()));
            }

#if USE_TIZEN
            connection_destroy(connection);
#endif

            // return response
            response_out = requestResponseEnum;
            json_out.CopyFrom(requestJsonDict, requestJsonDict.GetAllocator());
        }

        void GAHTTPApi::sendSdkErrorEvent(EGASdkErrorType type)
        {
            if(!state::GAState::isEventSubmissionEnabled())
            {
                return;
            }

            const char* gameKey = state::GAState::getGameKey();
            const char* secretKey = state::GAState::getGameSecret();

            // Validate
            if (!validators::GAValidator::validateSdkErrorEvent(gameKey, secretKey, type))
            {
                return;
            }

            // Generate URL

            char url[257] = "";
            snprintf(url, sizeof(url), "%s/%s/%s", baseUrl, gameKey, eventsUrlPath);

            {
                char s[257] = "";
                snprintf(s, sizeof(s), "Sending 'events' URL: %s", url);
                logging::GALogger::d(s);
            }

            rapidjson::Document json;
            json.SetObject();
            state::GAState::getSdkErrorEventAnnotations(json);

            char typeString[10] = "";
            sdkErrorTypeToString(type, typeString);
            {
                rapidjson::Value v(typeString, json.GetAllocator());
                json.AddMember("type", v.Move(), json.GetAllocator());
            }

            rapidjson::Document eventArray;
            eventArray.SetArray();
            rapidjson::Document::AllocatorType& allocator = eventArray.GetAllocator();
            eventArray.PushBack(json, allocator);
            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                eventArray.Accept(writer);
            }
            std::string payloadJSONString = buffer.GetString();

            if(payloadJSONString.empty())
            {
                logging::GALogger::w("sendSdkErrorEvent: JSON encoding failed.");
                return;
            }

            logging::GALogger::d("sendSdkErrorEvent json: " + payloadJSONString);
            // TODO: add async call
            /*byte[] payloadData = Encoding.UTF8.GetBytes(payloadJSONString);
            SdkErrorTask sdkErrorTask = new SdkErrorTask(type, payloadData, secretKey);
            sdkErrorTask.Execute(url);*/

            if (payloadJSONString.empty())
            {
                logging::GALogger::w("sendSdkErrorEvent: JSON encoding failed.");
                return;
            }

            logging::GALogger::d("sendSdkErrorEvent json: " + payloadJSONString);

            if (countMap[type] >= MaxCount)
            {
                return;
            }

#if !NO_ASYNC
            bool useGzip = this->useGzip;

            std::async(std::launch::async, [url, payloadJSONString, useGzip, type]() -> void
            {
                std::string payloadData = GAHTTPApi::sharedInstance()->createPayloadData(payloadJSONString, useGzip);
                std::ostringstream os;
                curl::curl_ios<std::ostringstream> writer(os);
                curl::curl_easy curl(writer);
                curl::curl_header header;
#if USE_TIZEN
                connection_h connection;
                int conn_err;
                conn_err = connection_create(&connection);
                if (conn_err != CONNECTION_ERROR_NONE)
                {
                    return;
                }
#endif
                std::string authorization = GAHTTPApi::sharedInstance()->createRequest(curl, header, url, payloadData, useGzip);

                try
                {
                    curl.perform();
                }
                catch (curl::curl_easy_exception error)
                {
                    error.print_traceback();
                    logging::GALogger::d(error.what());
                }

                std::string body = os.str();
                // process the response
                logging::GALogger::d("sdk error content : " + body);

                long statusCode = curl.get_info<CURLINFO_RESPONSE_CODE>().get();

                // if not 200 result
                if (statusCode != 200)
                {
                    logging::GALogger::d("sdk error failed. response code not 200. status code: " + std::to_string(statusCode));
#if USE_TIZEN
                    connection_destroy(connection);
#endif
                    return;
                }

#if USE_TIZEN
                connection_destroy(connection);
#endif

                countMap[type] = countMap[type] + 1;
            });
#endif
        }

        const int GAHTTPApi::MaxCount = 10;
        std::map<EGASdkErrorType, int> GAHTTPApi::countMap = std::map<EGASdkErrorType, int>();

        const std::string GAHTTPApi::createPayloadData(const std::string& payload, bool gzip)
        {
            std::string payloadData;

            if (gzip)
            {
                payloadData = utilities::GAUtilities::gzipCompress(payload);
                logging::GALogger::d("Gzip stats. Size: " + std::to_string(payload.size()) + ", Compressed: " + std::to_string(payloadData.size()));
            }
            else
            {
                payloadData = payload;
            }

            return payloadData;
        }

        const std::string GAHTTPApi::createRequest(curl::curl_easy& curl, curl::curl_header& header, const std::string& url, const std::string& payloadData, bool gzip)
        {
            curl.add<CURLOPT_URL>(url.c_str());

            if (gzip)
            {
                header.add(std::string("Content-Encoding: gzip"));
            }

            // create authorization hash
            const char* key = state::GAState::getGameSecret();

            std::string authorization = utilities::GAUtilities::hmacWithKey(key, payloadData);
            header.add(std::string("Authorization: " + authorization));

            // always JSON
            header.add("Content-Type: application/json");

            curl.add<CURLOPT_HTTPHEADER>(header.get());
            curl.add<CURLOPT_POSTFIELDS>(payloadData.c_str());
            curl.add<CURLOPT_SSL_VERIFYPEER>(false);
            curl.add<CURLOPT_POSTFIELDSIZE>(payloadData.size());

            return authorization;
        }

        EGAHTTPApiResponse GAHTTPApi::processRequestResponse(curl::curl_easy& curl, const std::string& body, const std::string& requestId)
        {
            long statusCode = curl.get_info<CURLINFO_RESPONSE_CODE>().get();

            // if no result - often no connection
            if (body.empty())
            {
                logging::GALogger::d(requestId + " request. failed. Might be no connection. Status code: " + std::to_string(statusCode));
                return NoResponse;
            }

            // ok
            if (statusCode == 200)
            {
                return Ok;
            }

            // 401 can return 0 status
            if (statusCode == 0 || statusCode == 401)
            {
                logging::GALogger::d(requestId + " request. 401 - Unauthorized.");
                return Unauthorized;
            }

            if (statusCode == 400)
            {
                logging::GALogger::d(requestId + " request. 400 - Bad Request.");
                return BadRequest;
            }

            if (statusCode == 500)
            {
                logging::GALogger::d(requestId + " request. 500 - Internal Server Error.");
                return InternalServerError;
            }
            return UnknownResponseCode;
        }
    }
}
#endif
