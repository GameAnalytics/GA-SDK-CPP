//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#include "GAHTTPApi.h"
#include "GAState.h"
#include "GALogger.h"
#include "GAUtilities.h"
#include "GAValidator.h"
#include <cstring>

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
            useGzip = true;
            
            curl_global_init(CURL_GLOBAL_ALL);
        }
        
        GAHTTPApi::~GAHTTPApi()
        {
            curl_global_cleanup();
        }

        EGAHTTPApiResponse GAHTTPApi::requestInitReturningDict(Json::Value& dict)
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
                dict.clear();
                return JsonEncodeFailed;
            }

            std::string payloadData = createPayloadData(JSONstring, useGzip);
            CURL* curl = createRequest(url, payloadData, useGzip);
            CURLcode res;
            
            char errbuf[CURL_ERROR_SIZE];
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
            
            res = curl_easy_perform(curl);
            
            std::string body;
            
            if(res != CURLE_OK)
            {
                std::string error(errbuf);
                
                if(error.length() > 0)
                {
                    body = error;
                }
                else
                {
                    logging::GALogger::d("Init request. failed. Might be no connection. Description: failed to parse response body");
                    return NoResponse;
                }
            }
            else
            {
                
            }
            
            std::string body = os.str();
            // process the response
            logging::GALogger::d("init request content : " + body);

            Json::Value requestJsonDict = utilities::GAUtilities::jsonFromString(body);
            EGAHTTPApiResponse requestResponseEnum = processRequestResponse(response, body, "Init");

            // if not 200 result
            if (requestResponseEnum != Ok && requestResponseEnum != BadRequest) 
            {
                std::string authorization;
                for (auto header : response.headers()) 
                {
                    if(header.first.compare("Authorization") == 0)
                    {
                        authorization = header.second;
                        break;
                    }
                }
                
                logging::GALogger::d("Failed Init Call. URL: " + url + ", Authorization: " + authorization + ", JSONString: " + JSONstring);
                dict = Json::Value();
                return requestResponseEnum;
            }

            if (requestJsonDict.isNull()) 
            {
                logging::GALogger::d("Failed Init Call. Json decoding failed");
                dict = Json::Value();
                return JsonDecodeFailed;
            }

            // print reason if bad request
            if (requestResponseEnum == BadRequest) 
            {
                logging::GALogger::d("Failed Init Call. Bad request. Response: " + requestJsonDict.toStyledString());
                // return bad request result
                dict = Json::Value();
                return requestResponseEnum;
            }

            // validate Init call values
            Json::Value validatedInitValues = validators::GAValidator::validateAndCleanInitRequestResponse(requestJsonDict);

            if (!validatedInitValues) 
            {
                dict = Json::Value();
                return BadResponse;
            }

            // all ok
            dict = validatedInitValues;

            return Ok;
        }

        EGAHTTPApiResponse GAHTTPApi::sendEventsInArray(const std::vector<Json::Value>& eventArray, Json::Value& dict)
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
                dict = {};
                return JsonEncodeFailed;
            }

            std::string payloadData = createPayloadData(JSONstring, useGzip);
            client::request request = createRequest(url, payloadData, useGzip);
            client client;
            client::response response = client.post(request, payloadData);
            std::ostringstream os;
            
            try
            {
                os << body(response);
            }
            catch (std::exception& ex)
            {
                logging::GALogger::d("Events request. failed. Might be no connection. Description: failed to parse response body");
                return NoResponse;
            }
            
            std::string body = os.str();
            logging::GALogger::d("body: " + body);

            EGAHTTPApiResponse requestResponseEnum = processRequestResponse(response, body, "Events");

            // if not 200 result
            if (requestResponseEnum != Ok && requestResponseEnum != BadRequest) 
            {
                std::string authorization;
                for (auto header : response.headers()) 
                {
                    
                    if(header.first.compare("Authorization") == 0)
                    {
                        authorization = header.second;
                        break;
                    }
                }
                
                logging::GALogger::d("Failed Events Call. URL: " + url + ", Authorization: " + authorization + ", JSONString: " + JSONstring);
                dict = {};
                return requestResponseEnum;
            }

            // decode JSON
            Json::Value requestJsonDict = utilities::GAUtilities::jsonFromString(body);

            if (requestJsonDict.isNull()) 
            {
                dict = {};
                return JsonDecodeFailed;
            }

            // print reason if bad request
            if (requestResponseEnum == BadRequest) 
            {
                logging::GALogger::d("Failed Events Call. Bad request. Response: " + requestJsonDict.toStyledString());
            }

            // return response
            dict = requestJsonDict;
            return requestResponseEnum;
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
        }

        const std::string GAHTTPApi::createPayloadData(const std::string& payload, bool gzip)
        {
            std::string payloadData;
            
            if (gzip)
            {
                payloadData = utilities::GAUtilities::gzipEnflate(payload);
                logging::GALogger::d("Gzip stats. Size: " + std::to_string(payload.size()) + ", Compressed: " + std::to_string(payloadData.size()));
            }
            else
            {
                payloadData = payload;
            }
            
            return payloadData;
        }

        const std::string GAHTTPApi::sdkErrorTypeToString(EGASdkErrorType value)
        {
            switch (value) {
            case Rejected:
                return "rejected";
            default:
                break;
            }
            return{};
        }

        CURL* GAHTTPApi::createRequest(const std::string& url, const std::string& payloadData, bool gzip)
        {
            CURL* curl = curl_easy_init();
            
            curl_slist *headers = NULL;
            
            if(curl)
            {
                curl_easy_setopt(curl, CURLOPT_URL, url);
                
                if (gzip)
                {
                    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING , "gzip");
                }
            }

            // create authorization hash
            std::string key = state::GAState::getGameSecret();

            headers = curl_slist_append(headers, std::string("Authorization: " + utilities::GAUtilities::hmacWithKey(key, payloadData)).c_str());

            // always JSON
            headers = curl_slist_append(headers, "Content-Type: application/json");
            
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadData.c_str());

            return curl;
        }
        
        size_t GAHTTPApi::curlCallback(void *contents, size_t size, size_t nmemb, void *userp)
        {
            size_t realsize = size * nmemb;                             /* calculate buffer size */
            CurlFetchStruct *p = (CurlFetchStruct *) userp;   /* cast pointer to fetch struct */

            /* expand buffer */
            p->payload = (char *) new char[p->size + realsize + 1];

            /* check buffer */
            if (p->payload == NULL) 
            {
                /* this isn't good */
                logging::GALogger::d("ERROR: Failed to expand buffer in curlCallback");
                /* free buffer */
                delete[] p->payload;
                /* return */
                return -1;
            }

            /* copy contents to buffer */
            std::memcpy(&(p->payload[p->size]), contents, realsize);

            /* set new buffer size */
            p->size += realsize;

            /* ensure null termination */
            p->payload[p->size] = 0;

            /* return size */
            return realsize;
        }

        EGAHTTPApiResponse GAHTTPApi::processRequestResponse(client::response& response, const std::string& body, const std::string& requestId)
        {
            // if no result - often no connection
            if (body.empty())
            {
                logging::GALogger::d(requestId + " request. failed. Might be no connection. Description: " + response.status_message() + ", Status code: " + std::to_string(response.status()));
                return NoResponse;
            }

            // ok
            if (response.status() == 200)
            {
                return Ok;
            }

            // 401 can return 0 status
            if (response.status() == 0 || response.status() == 401)
            {
                logging::GALogger::d(requestId + " request. 401 - Unauthorized.");
                return Unauthorized;
            }

            if (response.status() == 400)
            {
                logging::GALogger::d(requestId + " request. 400 - Bad Request.");
                return BadRequest;
            }

            if (response.status() == 500)
            {
                logging::GALogger::d(requestId + " request. 500 - Internal Server Error.");
                return InternalServerError;
            }
            return UnknownResponseCode;
        }
    }
}

