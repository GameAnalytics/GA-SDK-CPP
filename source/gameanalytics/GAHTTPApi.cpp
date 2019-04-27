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

        void initResponseData(struct ResponseData *s)
        {
            s->len = 0;
            s->ptr = static_cast<char*>(malloc(s->len+1));
            if (s->ptr == NULL)
            {
                exit(EXIT_FAILURE);
            }
            s->ptr[0] = '\0';
        }

        size_t writefunc(void *ptr, size_t size, size_t nmemb, struct ResponseData *s)
        {
            size_t new_len = s->len + size*nmemb;
            s->ptr = static_cast<char*>(realloc(s->ptr, new_len+1));
            if (s->ptr == NULL)
            {
                exit(EXIT_FAILURE);
            }
            memcpy(s->ptr+s->len, ptr, size*nmemb);
            s->ptr[new_len] = '\0';
            s->len = new_len;

            return size*nmemb;
        }

        // Constructor - setup the basic information for HTTP
        GAHTTPApi::GAHTTPApi()
        {
            curl_global_init(CURL_GLOBAL_DEFAULT);

            initBaseUrl();
            // use gzip compression on JSON body
#if defined(_DEBUG)
            useGzip = false;
#else
            useGzip = true;
#endif
        }

        GAHTTPApi::~GAHTTPApi()
        {
            curl_global_cleanup();
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

            CURL *curl;
            CURLcode res;
            curl = curl_easy_init();
            if(!curl)
            {
                response_out = NoResponse;
                json_out.SetNull();
                return;
            }

            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

            struct ResponseData s;
            initResponseData(&s);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
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

            std::string authorization = createRequest(curl, url, payloadData.c_str(), useGzip);

            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            {
                logging::GALogger::d(curl_easy_strerror(res));
                response_out = NoResponse;
                json_out.SetNull();
                return;
            }

            char body[strlen(s.ptr) + 1];
            snprintf(body, strlen(s.ptr) + 1, "%s", s.ptr);
            free(s.ptr);
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            curl_easy_cleanup(curl);

            // process the response
            {
                int ss = strlen(body) + 30;
                char s[ss];
                snprintf(s, ss, "init request content: %s", body);
                logging::GALogger::d(s);
            }

            rapidjson::Document requestJsonDict;
            requestJsonDict.Parse(body);
            EGAHTTPApiResponse requestResponseEnum = processRequestResponse(response_code, body, "Init");

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
                {
                    int ss = strlen(buffer.GetString()) + 50;
                    char s[ss];
                    snprintf(s, ss, "Failed Init Call. Bad request. Response: %s", buffer.GetString());
                    logging::GALogger::d(s);
                }
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
                return;
            }

            auto gameKey = state::GAState::getGameKey();

            // Generate URL
            char url[513] = "";
            snprintf(url, sizeof(url), "%s/%s/%s", baseUrl, gameKey, eventsUrlPath);

            {
                char s[513] = "";
                snprintf(s, sizeof(s), "Sending 'events' URL: %s", url);
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

            CURL *curl;
            CURLcode res;
            curl = curl_easy_init();
            if(!curl)
            {
                response_out = NoResponse;
                json_out.SetNull();
                return;
            }

            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

            struct ResponseData s;
            initResponseData(&s);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
#if USE_TIZEN
            connection_h connection;
            int conn_err;
            conn_err = connection_create(&connection);
            if (conn_err != CONNECTION_ERROR_NONE)
            {
                response_out = NoResponse;
                json_out = rapidjson::Value();
                return;
            }
#endif
            std::string  authorization = createRequest(curl, url, payloadData.c_str(), useGzip);

            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            {
                logging::GALogger::d(curl_easy_strerror(res));
                response_out = NoResponse;
                json_out.SetNull();
                return;
            }

            /*try
            {
                curl.perform();
            }
            catch (curl::curl_easy_exception error)
            {
                auto errors = error.get_traceback();
                logging::GALogger::d(error.what());
                std::for_each(errors.begin(), errors.end(),[](const curl::curlcpp_traceback_object &value)
                {
                    char s[256] = "";
                    snprintf(s, sizeof(s), "ERROR: %s ::::: FUNCTION: %s", value.first.c_str(), value.second.c_str());
                    logging::GALogger::d(s);
                });
            }*/

            char body[strlen(s.ptr) + 1];
            snprintf(body, strlen(s.ptr) + 1, "%s", s.ptr);
            free(s.ptr);
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            curl_easy_cleanup(curl);

            {
                int ss = strlen(body) + 10;
                char s[ss];
                snprintf(s, ss, "body: %s", body);
                logging::GALogger::d(s);
            }

            EGAHTTPApiResponse requestResponseEnum = processRequestResponse(response_code, body, "Events");

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
            requestJsonDict.Parse(body);

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

                int ss = strlen(buffer.GetString()) + 50;
                char s[ss];
                snprintf(s, ss, "Failed Events Call. Bad request. Response: %s", buffer.GetString());
                logging::GALogger::d(s);
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
            const char* payloadJSONString = buffer.GetString();

            if(strlen(payloadJSONString) == 0)
            {
                logging::GALogger::w("sendSdkErrorEvent: JSON encoding failed.");
                return;
            }

            {
                int ss = strlen(payloadJSONString) + 30;
                char s[ss];
                snprintf(s, ss, "sendSdkErrorEvent json: %s", payloadJSONString);
                logging::GALogger::d(s);
            }

            if (countMap[type] >= MaxCount)
            {
                return;
            }

#if !NO_ASYNC
            bool useGzip = this->useGzip;

            std::async(std::launch::async, [url, payloadJSONString, useGzip, type]() -> void
            {
                std::string payloadData = GAHTTPApi::sharedInstance()->createPayloadData(payloadJSONString, useGzip);

                CURL *curl;
                CURLcode res;
                curl = curl_easy_init();
                if(!curl)
                {
                    return;
                }

                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

                struct ResponseData s;
                initResponseData(&s);

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
#if USE_TIZEN
                connection_h connection;
                int conn_err;
                conn_err = connection_create(&connection);
                if (conn_err != CONNECTION_ERROR_NONE)
                {
                    return;
                }
#endif
                std::string authorization = GAHTTPApi::sharedInstance()->createRequest(curl, url, payloadData.c_str(), useGzip);

                res = curl_easy_perform(curl);
                if(res != CURLE_OK)
                {
                    logging::GALogger::d(curl_easy_strerror(res));
                    return;
                }

                char body[strlen(s.ptr) + 1];
                snprintf(body, strlen(s.ptr) + 1, "%s", s.ptr);
                free(s.ptr);
                long statusCode;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
                curl_easy_cleanup(curl);

                // process the response
                {
                    int ss = strlen(body) + 30;
                    char s[ss];
                    snprintf(s, ss, "sdk error content : %s", body);
                    logging::GALogger::d(s);
                }

                // if not 200 result
                if (statusCode != 200)
                {
                    char s[129] = "";
                    snprintf(s, sizeof(s), "sdk error failed. response code not 200. status code: %u", CURLE_OK);
                    logging::GALogger::d(s);
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

                char s[400] = "";
                snprintf(s, sizeof(s), "Gzip stats. Size: %lu, Compressed: %lu", payload.size(), payloadData.size());
                logging::GALogger::d(s);
            }
            else
            {
                payloadData = payload;
            }

            return payloadData;
        }

        const std::string GAHTTPApi::createRequest(CURL *curl, const char* url, const char* payloadData, bool gzip)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            struct curl_slist *header = NULL;

            if (gzip)
            {
                header = curl_slist_append(header, "Content-Encoding: gzip");
            }

            // create authorization hash
            const char* key = state::GAState::getGameSecret();

            std::string authorization = utilities::GAUtilities::hmacWithKey(key, payloadData);
            char auth[129] = "";
            snprintf(auth, sizeof(auth), "Authorization: %s", authorization.c_str());
            header = curl_slist_append(header, auth);

            // always JSON
            header = curl_slist_append(header, "Content-Type: application/json");

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadData);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(payloadData));

            return authorization;
        }

        EGAHTTPApiResponse GAHTTPApi::processRequestResponse(long statusCode, const char* body, const char* requestId)
        {
            // if no result - often no connection
            if (utilities::GAUtilities::isStringNullOrEmpty(body))
            {
                char s[129] = "";
                snprintf(s, sizeof(s), "%s request. failed. Might be no connection. Status code: %ld", requestId, statusCode);
                logging::GALogger::d(s);
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
                char s[129] = "";
                snprintf(s, sizeof(s), "%s request. 401 - Unauthorized.", requestId);
                logging::GALogger::d(s);
                return Unauthorized;
            }

            if (statusCode == 400)
            {
                char s[129] = "";
                snprintf(s, sizeof(s), "%s request. 400 - Bad Request.", requestId);
                logging::GALogger::d(s);
                return BadRequest;
            }

            if (statusCode == 500)
            {
                char s[129] = "";
                snprintf(s, sizeof(s), "%s request. 500 - Internal Server Error.", requestId);
                logging::GALogger::d(s);
                return InternalServerError;
            }
            return UnknownResponseCode;
        }
    }
}
#endif
