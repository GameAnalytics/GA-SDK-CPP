#if USE_UWP
#include "GAHTTPApi.h"
#include "GAState.h"
#include "GALogger.h"
#include "GAUtilities.h"
#include "GAValidator.h"
#include <map>

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
            httpClient = ref new Windows::Web::Http::HttpClient();
        }

        concurrency::task<std::pair<EGAHTTPApiResponse, Json::Value>> GAHTTPApi::requestInitReturningDict()
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
                return concurrency::create_task([]()
                {
                    return std::pair<EGAHTTPApiResponse, Json::Value>(JsonEncodeFailed, Json::Value());
                });
            }

            std::string payloadData = createPayloadData(JSONstring, useGzip);
            auto message = ref new Windows::Web::Http::HttpRequestMessage();

            std::string authorization = createRequest(message, url, payloadData, useGzip);

            return concurrency::create_task(httpClient->SendRequestAsync(message)).then([=](Windows::Web::Http::HttpResponseMessage^ response)
            {
                EGAHTTPApiResponse requestResponseEnum = processRequestResponse(response, "Init");

                // if not 200 result
                if (requestResponseEnum != Ok && requestResponseEnum != BadRequest)
                {
                    logging::GALogger::d("Failed Init Call. URL: " + url + ", JSONString: " + JSONstring + ", Authorization: " + authorization);
                    return std::pair<EGAHTTPApiResponse, Json::Value>(requestResponseEnum, Json::Value());
                }

                // print reason if bad request
                if (requestResponseEnum == BadRequest)
                {
                    Json::Value requestJsonDict = utilities::GAUtilities::jsonFromString(utilities::GAUtilities::ws2s(response->Content->ToString()->Data()));
                    logging::GALogger::d("Failed Init Call. Bad request. Response: " + requestJsonDict.toStyledString());
                    // return bad request result
                    return std::pair<EGAHTTPApiResponse, Json::Value>(requestResponseEnum, Json::Value());
                }

                concurrency::create_task(response->Content->ReadAsStringAsync()).then([=](Platform::String^ responseBodyAsText)
                {
                    // Return response.
                    std::string body = utilities::GAUtilities::ws2s(responseBodyAsText->Data());

                    logging::GALogger::d("init request content : " + body);

                    Json::Value requestJsonDict = utilities::GAUtilities::jsonFromString(body);

                    if (requestJsonDict.isNull())
                    {
                        logging::GALogger::d("Failed Init Call. Json decoding failed");
                        return std::pair<EGAHTTPApiResponse, Json::Value>(JsonDecodeFailed, Json::Value());
                    }

                    // validate Init call values
                    Json::Value validatedInitValues = validators::GAValidator::validateAndCleanInitRequestResponse(requestJsonDict);

                    if (!validatedInitValues)
                    {
                        return std::pair<EGAHTTPApiResponse, Json::Value>(BadResponse, Json::Value());
                    }

                    // all ok
                    return std::pair<EGAHTTPApiResponse, Json::Value>(Ok, validatedInitValues);
                });

                return std::pair<EGAHTTPApiResponse, Json::Value>(NoResponse, Json::Value());
            });
        }

        concurrency::task<std::pair<EGAHTTPApiResponse, Json::Value>> GAHTTPApi::sendEventsInArray(const std::vector<Json::Value>& eventArray)
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
                return concurrency::create_task([]()
                {
                    return std::pair<EGAHTTPApiResponse, Json::Value>(JsonEncodeFailed, Json::Value());
                });
            }

            std::string payloadData = createPayloadData(JSONstring, useGzip);
            auto message = ref new Windows::Web::Http::HttpRequestMessage();

            std::string authorization = createRequest(message, url, payloadData, useGzip);

            return concurrency::create_task(httpClient->SendRequestAsync(message)).then([=](Windows::Web::Http::HttpResponseMessage^ response)
            {
                EGAHTTPApiResponse requestResponseEnum = processRequestResponse(response, "Events");

                // if not 200 result
                if (requestResponseEnum != Ok && requestResponseEnum != BadRequest)
                {
                    logging::GALogger::d("Failed Events Call. URL: " + url + ", JSONString: " + JSONstring + ", Authorization: " + authorization);
                    return std::pair<EGAHTTPApiResponse, Json::Value>(requestResponseEnum, Json::Value());
                }

                // print reason if bad request
                if (requestResponseEnum == BadRequest)
                {
                    Json::Value requestJsonDict = utilities::GAUtilities::jsonFromString(utilities::GAUtilities::ws2s(response->Content->ToString()->Data()));
                    logging::GALogger::d("Failed Events Call. Bad request. Response: " + requestJsonDict.toStyledString());
                    // return bad request result
                    return std::pair<EGAHTTPApiResponse, Json::Value>(requestResponseEnum, Json::Value());
                }

                concurrency::create_task(response->Content->ReadAsStringAsync()).then([requestResponseEnum](Platform::String^ responseBodyAsText)
                {
                    // Return response.
                    std::string body = utilities::GAUtilities::ws2s(responseBodyAsText->Data());

                    logging::GALogger::d("body: " + body);

                    Json::Value requestJsonDict = utilities::GAUtilities::jsonFromString(body);

                    if (requestJsonDict.isNull())
                    {
                        return std::pair<EGAHTTPApiResponse, Json::Value>(JsonDecodeFailed, Json::Value());
                    }

                    // all ok
                    return std::pair<EGAHTTPApiResponse, Json::Value>(requestResponseEnum, requestJsonDict);
                });

                return std::pair<EGAHTTPApiResponse, Json::Value>(NoResponse, Json::Value());
            });
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

            std::string payloadData = createPayloadData(payloadJSONString, useGzip);
            auto message = ref new Windows::Web::Http::HttpRequestMessage();

            std::string authorization = createRequest(message, url, payloadData, useGzip);

            concurrency::create_task(httpClient->SendRequestAsync(message)).then([=](Windows::Web::Http::HttpResponseMessage^ response)
            {
                EGAHTTPApiResponse requestResponseEnum = processRequestResponse(response, "Init");

                // if not 200 result
                if (requestResponseEnum != Ok && requestResponseEnum != BadRequest)
                {
                    logging::GALogger::d("Failed Init Call. URL: " + url + ", JSONString: " + payloadJSONString + ", Authorization: " + authorization);
                    return;
                }

                // print reason if bad request
                if (requestResponseEnum == BadRequest)
                {
                    Json::Value requestJsonDict = utilities::GAUtilities::jsonFromString(utilities::GAUtilities::ws2s(response->Content->ToString()->Data()));
                    logging::GALogger::d("Failed Init Call. Bad request. Response: " + requestJsonDict.toStyledString());
                    // return bad request result
                    return;
                }

                concurrency::create_task(response->Content->ReadAsStringAsync()).then([type](Platform::String^ responseBodyAsText)
                {
                    // Return response.
                    std::string body = utilities::GAUtilities::ws2s(responseBodyAsText->Data());

                    logging::GALogger::d("init request content : " + body);

                    countMap[type] = countMap[type] + 1;
                });
            });
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

        const std::string GAHTTPApi::createRequest(Windows::Web::Http::HttpRequestMessage^ message, const std::string& url, const std::string& payloadData, bool gzip)
        {
            auto urlString = ref new Platform::String(utilities::GAUtilities::s2ws(url).c_str());
            message->RequestUri = ref new Windows::Foundation::Uri(urlString);
            message->Method = Windows::Web::Http::HttpMethod::Post;

            if (gzip)
            {
                message->Headers->Append("Content-Encoding", "gzip");
            }

            // create authorization hash
            std::string key = state::GAState::getGameSecret();
            auto authorization = ref new Platform::String(utilities::GAUtilities::s2ws(utilities::GAUtilities::hmacWithKey(key, payloadData)).c_str());

            message->Headers->TryAppendWithoutValidation(L"Authorization", authorization);

            message->Content = ref new Windows::Web::Http::HttpStringContent(ref new Platform::String(utilities::GAUtilities::s2ws(payloadData).c_str()), Windows::Storage::Streams::UnicodeEncoding::Utf8, ref new Platform::String(L"application/json"));
            message->Headers->Append("Content-Length", payloadData.size().ToString());

            return utilities::GAUtilities::ws2s(authorization->Data());
        }

        EGAHTTPApiResponse GAHTTPApi::processRequestResponse(Windows::Web::Http::HttpResponseMessage^ response, const std::string& requestId)
        {
            Windows::Web::Http::HttpStatusCode statusCode = response->StatusCode;

            // if no result - often no connection
            if (!response->IsSuccessStatusCode && std::wstring(response->Content->ToString()->Data()).empty())
            {
                logging::GALogger::d(requestId + " request. failed. Might be no connection. Status code: " + utilities::GAUtilities::ws2s(statusCode.ToString()->Data()));
                return NoResponse;
            }

            // ok
            if (statusCode == Windows::Web::Http::HttpStatusCode::Ok)
            {
                return Ok;
            }

            // 401 can return 0 status
            if (statusCode == (Windows::Web::Http::HttpStatusCode)0 || statusCode == Windows::Web::Http::HttpStatusCode::Unauthorized)
            {
                logging::GALogger::d(requestId + " request. 401 - Unauthorized.");
                return Unauthorized;
            }

            if (statusCode == Windows::Web::Http::HttpStatusCode::BadRequest)
            {
                logging::GALogger::d(requestId + " request. 400 - Bad Request.");
                return BadRequest;
            }

            if (statusCode == Windows::Web::Http::HttpStatusCode::InternalServerError)
            {
                logging::GALogger::d(requestId + " request. 500 - Internal Server Error.");
                return InternalServerError;
            }
            return UnknownResponseCode;
        }
    }
}

#endif