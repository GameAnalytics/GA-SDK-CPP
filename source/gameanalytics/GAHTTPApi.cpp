//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
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
#if USE_TIZEN
#include <net_connection.h>
#endif

namespace gameanalytics
{
    namespace http
    {
        // Constructor - setup the basic information for HTTP
        GAHTTPApi::GAHTTPApi()
        {
            // base url settings
            protocol = "https";
            hostName = "api.gameanalytics.com";

            version = "v2";

            // create base url
            baseUrl = protocol + "://" + hostName + "/" + version;

            // route paths
            initializeUrlPath = "init";
            eventsUrlPath = "events";

            // use gzip compression on JSON body
#if defined(_DEBUG)
            useGzip = false;
#else
            useGzip = true;
#endif
        }

        std::pair<EGAHTTPApiResponse, Json::Value> GAHTTPApi::requestInitReturningDict()
        {
            std::string gameKey = state::GAState::getGameKey();

            // Generate URL
            std::string url = baseUrl + "/" + gameKey + "/" + initializeUrlPath;
            logging::GALogger::d("Sending 'init' URL: " + url);

            Json::Value initAnnotations = state::GAState::getInitAnnotations();

            // make JSON string from data
            std::string JSONstring = utilities::GAUtilities::jsonToString(initAnnotations);

            if (JSONstring.empty())
            {
                return std::pair<EGAHTTPApiResponse, Json::Value>(JsonEncodeFailed, Json::Value());
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
                return std::pair<EGAHTTPApiResponse, Json::Value>(NoResponse, Json::Value());
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

            Json::Value requestJsonDict = utilities::GAUtilities::jsonFromString(body);
            EGAHTTPApiResponse requestResponseEnum = processRequestResponse(curl, body, "Init");

            // if not 200 result
            if (requestResponseEnum != Ok && requestResponseEnum != BadRequest)
            {
                logging::GALogger::d("Failed Init Call. URL: " + url + ", JSONString: " + JSONstring + ", Authorization: " + authorization);
#if USE_TIZEN
                connection_destroy(connection);
#endif
                return std::pair<EGAHTTPApiResponse, Json::Value>(requestResponseEnum, Json::Value());
            }

            if (requestJsonDict.isNull())
            {
                logging::GALogger::d("Failed Init Call. Json decoding failed");
#if USE_TIZEN
                connection_destroy(connection);
#endif
                return std::pair<EGAHTTPApiResponse, Json::Value>(JsonDecodeFailed, Json::Value());
            }

            // print reason if bad request
            if (requestResponseEnum == BadRequest)
            {
                logging::GALogger::d("Failed Init Call. Bad request. Response: " + requestJsonDict.toStyledString());
                // return bad request result
#if USE_TIZEN
                connection_destroy(connection);
#endif
                return std::pair<EGAHTTPApiResponse, Json::Value>(requestResponseEnum, Json::Value());
            }

            // validate Init call values
            Json::Value validatedInitValues = validators::GAValidator::validateAndCleanInitRequestResponse(requestJsonDict);

            if (!validatedInitValues)
            {
#if USE_TIZEN
                connection_destroy(connection);
#endif
                return std::pair<EGAHTTPApiResponse, Json::Value>(BadResponse, Json::Value());
            }

#if USE_TIZEN
            connection_destroy(connection);
#endif

            // all ok
            return std::pair<EGAHTTPApiResponse, Json::Value>(Ok, validatedInitValues);
        }

        std::pair<EGAHTTPApiResponse, Json::Value> GAHTTPApi::sendEventsInArray(const std::vector<Json::Value>& eventArray)
        {
            if (eventArray.empty())
            {
                logging::GALogger::d("sendEventsInArray called with missing eventArray");
            }

            auto gameKey = state::GAState::getGameKey();

            // Generate URL
            auto url = baseUrl + "/" + gameKey + "/" + eventsUrlPath;
            logging::GALogger::d("Sending 'events' URL: " + url);

            // make JSON string from data
            auto JSONstring = utilities::GAUtilities::arrayOfObjectsToJsonString(eventArray);

            if (JSONstring.empty())
            {
                logging::GALogger::d("sendEventsInArray JSON encoding failed of eventArray");
                return std::pair<EGAHTTPApiResponse, Json::Value>(JsonEncodeFailed, Json::Value());
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
                return std::pair<EGAHTTPApiResponse, Json::Value>(NoResponse, Json::Value());
            }
#endif
            std::string  authorization = createRequest(curl, header, url, payloadData, useGzip);

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
            logging::GALogger::d("body: " + body);

            EGAHTTPApiResponse requestResponseEnum = processRequestResponse(curl, body, "Events");

            // if not 200 result
            if (requestResponseEnum != Ok && requestResponseEnum != BadRequest)
            {
                logging::GALogger::d("Failed Events Call. URL: " + url + ", JSONString: " + JSONstring + ", Authorization: " + authorization);
#if USE_TIZEN
                connection_destroy(connection);
#endif
                return std::pair<EGAHTTPApiResponse, Json::Value>(requestResponseEnum, Json::Value());
            }

            // decode JSON
            Json::Value requestJsonDict = utilities::GAUtilities::jsonFromString(body);

            if (requestJsonDict.isNull())
            {
#if USE_TIZEN
                connection_destroy(connection);
#endif
                return std::pair<EGAHTTPApiResponse, Json::Value>(JsonDecodeFailed, Json::Value());
            }

            // print reason if bad request
            if (requestResponseEnum == BadRequest)
            {
                logging::GALogger::d("Failed Events Call. Bad request. Response: " + requestJsonDict.toStyledString());
            }

#if USE_TIZEN
            connection_destroy(connection);
#endif

            // return response
            return std::pair<EGAHTTPApiResponse, Json::Value>(requestResponseEnum, requestJsonDict);
        }

        void GAHTTPApi::sendSdkErrorEvent(EGASdkErrorType type)
        {
            std::string gameKey = state::GAState::getGameKey();
            std::string secretKey = state::GAState::getGameSecret();

            // Validate
            if (!validators::GAValidator::validateSdkErrorEvent(gameKey, secretKey, type))
            {
                return;
            }

            // Generate URL
            std::string url = baseUrl + "/" + gameKey + "/" + eventsUrlPath;
            logging::GALogger::d("Sending 'events' URL: " + url);

            Json::Value json = state::GAState::getSdkErrorEventAnnotations();

            std::string typeString = sdkErrorTypeToString(type);
            json["type"] = typeString;

            std::vector<Json::Value> eventArray;
            eventArray.push_back(json);
            std::string payloadJSONString = utilities::GAUtilities::arrayOfObjectsToJsonString(eventArray);

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
                logging::GALogger::d("init request content : " + body);

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
            std::string key = state::GAState::getGameSecret();

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
