#if USE_UWP
#include "GAHTTPApi.h"
#include "GAState.h"
#include "GALogger.h"
#include "GAUtilities.h"
#include "GAValidator.h"
#include <map>
#include <robuffer.h>
#include <assert.h>
#if USE_UWP
#include "GADevice.h"
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
            httpClient = ref new Windows::Web::Http::HttpClient();
            Windows::Networking::Connectivity::NetworkInformation::NetworkStatusChanged += ref new Windows::Networking::Connectivity::NetworkStatusChangedEventHandler(&GANetworkStatus::NetworkInformationOnNetworkStatusChanged);
            GANetworkStatus::CheckInternetAccess();
        }

        bool GANetworkStatus::hasInternetAccess = false;

        void GANetworkStatus::NetworkInformationOnNetworkStatusChanged(Platform::Object^ sender)
        {
            CheckInternetAccess();
        }

        void GANetworkStatus::CheckInternetAccess()
        {
            auto connectionProfile = Windows::Networking::Connectivity::NetworkInformation::GetInternetConnectionProfile();
            hasInternetAccess = (connectionProfile != nullptr && connectionProfile->GetNetworkConnectivityLevel() == Windows::Networking::Connectivity::NetworkConnectivityLevel::InternetAccess);

            if (hasInternetAccess)
            {
                if (connectionProfile->IsWlanConnectionProfile)
                {
                    device::GADevice::setConnectionType("wifi");
                }
                else if (connectionProfile->IsWwanConnectionProfile)
                {
                    device::GADevice::setConnectionType("wwan");
                }
                else
                {
                    device::GADevice::setConnectionType("lan");
                }
            }
            else
            {
                device::GADevice::setConnectionType("offline");
            }
        }

        concurrency::task<std::pair<EGAHTTPApiResponse, Json::Value>> GAHTTPApi::requestInitReturningDict()
        {
            std::string gameKey = state::GAState::getGameKey();

            // Generate URL
            std::string url = baseUrl + "/" + gameKey + "/" + initializeUrlPath;
            url = "https://rubick.gameanalytics.com/v2/command_center?game_key=" + gameKey + "&interval_seconds=1000000";
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

            if (!GANetworkStatus::hasInternetAccess)
            {
                return concurrency::create_task([]()
                {
                    return std::pair<EGAHTTPApiResponse, Json::Value>(NoResponse, Json::Value());
                });
            }

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

                concurrency::task<Platform::String^> readTask(response->Content->ReadAsStringAsync());
                Platform::String^ responseBodyAsText = readTask.get();

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

            if (!GANetworkStatus::hasInternetAccess)
            {
                return concurrency::create_task([]()
                {
                    return std::pair<EGAHTTPApiResponse, Json::Value>(NoResponse, Json::Value());
                });
            }

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

                concurrency::task<Platform::String^> readTask(response->Content->ReadAsStringAsync());
                Platform::String^ responseBodyAsText = readTask.get();

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

            if (!GANetworkStatus::hasInternetAccess)
            {
                return;
            }

            concurrency::create_task(httpClient->SendRequestAsync(message)).then([=](Windows::Web::Http::HttpResponseMessage^ response)
            {
                Windows::Web::Http::HttpStatusCode statusCode = response->StatusCode;

                // if not 200 result
                if (statusCode != Windows::Web::Http::HttpStatusCode::Ok)
                {
                    logging::GALogger::d("sdk error failed. response code not 200. status code: " + utilities::GAUtilities::ws2s(statusCode.ToString()->Data()));
                    return;
                }

                concurrency::task<Platform::String^> readTask(response->Content->ReadAsStringAsync());
                Platform::String^ responseBodyAsText = readTask.get();

                // Return response.
                std::string body = utilities::GAUtilities::ws2s(responseBodyAsText->Data());

                logging::GALogger::d("init request content : " + body);

                countMap[type] = countMap[type] + 1;
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

            // create authorization hash
            std::string key = state::GAState::getGameSecret();
            auto authorization = ref new Platform::String(utilities::GAUtilities::s2ws(utilities::GAUtilities::hmacWithKey(key, payloadData)).c_str());

            message->Headers->TryAppendWithoutValidation(L"Authorization", authorization);

            if (gzip)
            {
                Windows::Storage::Streams::InMemoryRandomAccessStream^ stream = createStream(payloadData).get();
                message->Content = ref new Windows::Web::Http::HttpStreamContent(stream);
                message->Content->Headers->ContentEncoding->Append(ref new Windows::Web::Http::Headers::HttpContentCodingHeaderValue(ref new Platform::String(L"gzip")));
            }
            else
            {
                message->Content = ref new Windows::Web::Http::HttpStringContent(ref new Platform::String(utilities::GAUtilities::s2ws(payloadData).c_str()), Windows::Storage::Streams::UnicodeEncoding::Utf8, ref new Platform::String(L"application/json"));
            }

            return utilities::GAUtilities::ws2s(authorization->Data());
        }

        concurrency::task<Windows::Storage::Streams::InMemoryRandomAccessStream^> GAHTTPApi::createStream(std::string data)
        {
            using namespace Windows::Storage::Streams;
            using namespace Microsoft::WRL;
            using namespace Platform;
            using namespace concurrency;

            return create_task([=]()
            {
                unsigned int size = data.size();
                Buffer^ buffer = ref new Buffer(size);
                buffer->Length = size;

                ComPtr<IBufferByteAccess> bufferByteAccess;
                HRESULT hr = reinterpret_cast<IUnknown*>(buffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));
                if (FAILED(hr))
                {
                    throw ref new Exception(hr);
                }

                byte* rawBuffer;
                hr = bufferByteAccess->Buffer(&rawBuffer);
                if (FAILED(hr))
                {
                    throw ref new Exception(hr);
                }

                for (unsigned int i = 0; i < size; ++i)
                {
                    rawBuffer[i] = data[i];
                }

                InMemoryRandomAccessStream^ stream = ref new InMemoryRandomAccessStream();
                task<unsigned int> writeTask(stream->WriteAsync(buffer));
                writeTask.wait();
                assert(writeTask.get() == data.size());

                // Rewind stream
                stream->Seek(0);

                return stream;
            });
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
