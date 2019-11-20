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
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

namespace gameanalytics
{
    namespace http
    {
        // base url settings
        char GAHTTPApi::protocol[6] = "https";
        char GAHTTPApi::hostName[22] = "api.gameanalytics.com";

        char GAHTTPApi::version[3] = "v2";
        char GAHTTPApi::remoteConfigsVersion[3] = "v1";

        // create base url
        char GAHTTPApi::baseUrl[257] = "";
        char GAHTTPApi::remoteConfigsBaseUrl[257] = "";

        // route paths
        char GAHTTPApi::initializeUrlPath[5] = "init";
        char GAHTTPApi::eventsUrlPath[7] = "events";
        // Constructor - setup the basic information for HTTP
        GAHTTPApi::GAHTTPApi()
        {
            // use gzip compression on JSON body
#if defined(_DEBUG)
            useGzip = false;
#else
            useGzip = true;
#endif
            snprintf(GAHTTPApi::baseUrl, sizeof(GAHTTPApi::baseUrl), "%s://%s/%s", protocol, hostName, version);
            snprintf(GAHTTPApi::remoteConfigsBaseUrl, sizeof(GAHTTPApi::remoteConfigsBaseUrl), "%s://%s/remote_configs/%s", protocol, hostName, remoteConfigsVersion);
            httpClient = ref new Windows::Web::Http::HttpClient();
            Windows::Networking::Connectivity::NetworkInformation::NetworkStatusChanged += ref new Windows::Networking::Connectivity::NetworkStatusChangedEventHandler(&GANetworkStatus::NetworkInformationOnNetworkStatusChanged);
            GANetworkStatus::CheckInternetAccess();
        }

        GAHTTPApi::~GAHTTPApi()
        {
        }

        bool GAHTTPApi::_destroyed = false;
        GAHTTPApi* GAHTTPApi::_instance = 0;
        std::once_flag GAHTTPApi::_initInstanceFlag;

        void GAHTTPApi::cleanUp()
        {
            delete _instance;
            _instance = 0;
            _destroyed = true;
        }

        GAHTTPApi* GAHTTPApi::getInstance()
        {
            std::call_once(_initInstanceFlag, &GAHTTPApi::initInstance);
            return _instance;
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

        concurrency::task<std::pair<EGAHTTPApiResponse, std::string>> GAHTTPApi::requestInitReturningDict(const char* configsHash)
        {
            std::string gameKey = state::GAState::getGameKey();

            // Generate URL
            std::string url = std::string(remoteConfigsBaseUrl) + "/" + std::string(gameKey) + "/" + std::string(initializeUrlPath);

            rapidjson::Document initAnnotations;
            initAnnotations.SetObject();
            state::GAState::getInitAnnotations(initAnnotations);

            // make JSON string from data
            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                initAnnotations.Accept(writer);
            }
            std::string JSONstring = buffer.GetString();

            if (JSONstring.empty())
            {
                return concurrency::create_task([]()
                {
                    return std::pair<EGAHTTPApiResponse, std::string>(JsonEncodeFailed, "");
                });
            }

            std::vector<char> payloadData = createPayloadData(JSONstring.c_str(), false);
            auto message = ref new Windows::Web::Http::HttpRequestMessage();

            std::vector<char> authorization = createRequest(message, url, payloadData, useGzip);

            if (!GANetworkStatus::hasInternetAccess)
            {
                return concurrency::create_task([]()
                {
                    return std::pair<EGAHTTPApiResponse, std::string>(NoResponse, "");
                });
            }

            return concurrency::create_task(httpClient->SendRequestAsync(message)).then([=](Windows::Web::Http::HttpResponseMessage^ response)
            {
                EGAHTTPApiResponse requestResponseEnum = processRequestResponse(response, "Init");

                // if not 200 result
                if (requestResponseEnum != Ok && requestResponseEnum != Created && requestResponseEnum != BadRequest)
                {
                    logging::GALogger::d("Failed Init Call. URL: %s, JSONString: %s, Authorization: %s", url, JSONstring, authorization);
                    return std::pair<EGAHTTPApiResponse, std::string>(requestResponseEnum, "");
                }

                // print reason if bad request
                if (requestResponseEnum == BadRequest)
                {
                    rapidjson::Document requestJsonDict;
                    requestJsonDict.Parse(utilities::GAUtilities::ws2s(response->Content->ToString()->Data()).c_str());

                    rapidjson::StringBuffer buffer;
                    {
                        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                        requestJsonDict.Accept(writer);
                    }
                    logging::GALogger::d("Failed Init Call. Bad request. Response: %s", buffer.GetString());
                    // return bad request result
                    return std::pair<EGAHTTPApiResponse, std::string>(requestResponseEnum, "");
                }

                concurrency::task<Platform::String^> readTask(response->Content->ReadAsStringAsync());
                Platform::String^ responseBodyAsText = readTask.get();

                // Return response.
                std::string body = utilities::GAUtilities::ws2s(responseBodyAsText->Data());

                logging::GALogger::d("init request content : %s", body.c_str());

                rapidjson::Document requestJsonDict;
                requestJsonDict.Parse(body.c_str());

                if (requestJsonDict.IsNull())
                {
                    logging::GALogger::d("Failed Init Call. Json decoding failed");
                    return std::pair<EGAHTTPApiResponse, std::string>(JsonDecodeFailed, "");
                }

                rapidjson::Document validatedInitValues;
                // validate Init call values
                validators::GAValidator::validateAndCleanInitRequestResponse(requestJsonDict, validatedInitValues, requestResponseEnum == Created);

                if (validatedInitValues.IsNull())
                {
                    return std::pair<EGAHTTPApiResponse, std::string>(BadResponse, "");
                }

                rapidjson::StringBuffer buffer;
                {
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    validatedInitValues.Accept(writer);
                }
                // all ok
                return std::pair<EGAHTTPApiResponse, std::string>(requestResponseEnum, buffer.GetString());
            });
        }

        concurrency::task<std::pair<EGAHTTPApiResponse, std::string>> GAHTTPApi::sendEventsInArray(const rapidjson::Value& eventArray)
        {
            if (eventArray.Empty())
            {
                logging::GALogger::d("sendEventsInArray called with missing eventArray");
            }

            auto gameKey = state::GAState::getGameKey();

            // Generate URL
            char url[513];
            snprintf(url, sizeof(url), "%s/%s/%s", baseUrl, gameKey, eventsUrlPath);
            logging::GALogger::d("Sending 'events' URL: %s", url);

            // make JSON string from data
            rapidjson::StringBuffer buffer;
            {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                eventArray.Accept(writer);
            }

            std::string JSONstring = buffer.GetString();

            if (JSONstring.empty())
            {
                logging::GALogger::d("sendEventsInArray JSON encoding failed of eventArray");
                return concurrency::create_task([]()
                {
                    return std::pair<EGAHTTPApiResponse, std::string>(JsonDecodeFailed, "");
                });
            }

            std::vector<char> payloadData = createPayloadData(JSONstring.c_str(), useGzip);
            auto message = ref new Windows::Web::Http::HttpRequestMessage();

            std::string authorization = createRequest(message, url, payloadData, useGzip).data();

            if (!GANetworkStatus::hasInternetAccess)
            {
                return concurrency::create_task([]()
                {
                    return std::pair<EGAHTTPApiResponse, std::string>(NoResponse, "");
                });
            }

            return concurrency::create_task(httpClient->SendRequestAsync(message)).then([=](Windows::Web::Http::HttpResponseMessage^ response)
            {
                EGAHTTPApiResponse requestResponseEnum = processRequestResponse(response, "Events");

                // if not 200 result
                if (requestResponseEnum != Ok && requestResponseEnum != Created && requestResponseEnum != BadRequest)
                {
                    logging::GALogger::d("Failed Events Call. URL: %s, JSONString: %s, Authorization: %s", url, JSONstring, authorization.c_str());
                    return std::pair<EGAHTTPApiResponse, std::string>(requestResponseEnum,"");
                }

                // print reason if bad request
                if (requestResponseEnum == BadRequest)
                {
                    rapidjson::Document requestJsonDict;
                    requestJsonDict.Parse(utilities::GAUtilities::ws2s(response->Content->ToString()->Data()).c_str());

                    rapidjson::StringBuffer buffer;
                    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
                    requestJsonDict.Accept(writer);

                    logging::GALogger::d("Failed Events Call. Bad request. Response: %s", buffer.GetString());
                    // return bad request result
                    return std::pair<EGAHTTPApiResponse, std::string>(requestResponseEnum,"");
                }

                concurrency::task<Platform::String^> readTask(response->Content->ReadAsStringAsync());
                Platform::String^ responseBodyAsText = readTask.get();

                // Return response.
                std::string body = utilities::GAUtilities::ws2s(responseBodyAsText->Data());

                logging::GALogger::d("body: %s", body.c_str());

                rapidjson::Document requestJsonDict;
                requestJsonDict.Parse(body.c_str());

                if (requestJsonDict.IsNull())
                {
                    return std::pair<EGAHTTPApiResponse, std::string>(JsonDecodeFailed,"");
                }

                // all ok
                rapidjson::StringBuffer buffer;
                {
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    requestJsonDict.Accept(writer);
                }
                return std::pair<EGAHTTPApiResponse, std::string>(requestResponseEnum, buffer.GetString());
            });
        }

        void GAHTTPApi::sendSdkErrorEvent(EGASdkErrorType type)
        {
            std::string gameKey = state::GAState::getGameKey();
            std::string secretKey = state::GAState::getGameSecret();

            // Validate
            if (!validators::GAValidator::validateSdkErrorEvent(gameKey.c_str(), secretKey.c_str(), type))
            {
                return;
            }

            // Generate URL
            std::string url = std::string(baseUrl) + "/" + std::string(gameKey) + "/" + std::string(eventsUrlPath);
            logging::GALogger::d("Sending 'events' URL: %s", url.c_str());

            rapidjson::Document json;
            json.SetObject();
            state::GAState::getSdkErrorEventAnnotations(json);

            char typeString[65];
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
            std::string payloadJSONString =  buffer.GetString();

            if (payloadJSONString.empty())
            {
                logging::GALogger::w("sendSdkErrorEvent: JSON encoding failed.");
                return;
            }

            logging::GALogger::d("sendSdkErrorEvent json: %s", payloadJSONString.c_str());

            if (countMap[type] >= MaxCount)
            {
                return;
            }

            std::vector<char> payloadData = createPayloadData(payloadJSONString.c_str(), useGzip);
            auto message = ref new Windows::Web::Http::HttpRequestMessage();

            std::vector<char> authorization = createRequest(message, url, payloadData, useGzip);

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
                    logging::GALogger::d("sdk error failed. response code not 200. status code: %s", utilities::GAUtilities::ws2s(statusCode.ToString()->Data()));
                    return;
                }

                concurrency::task<Platform::String^> readTask(response->Content->ReadAsStringAsync());
                Platform::String^ responseBodyAsText = readTask.get();

                // Return response.
                std::string body = utilities::GAUtilities::ws2s(responseBodyAsText->Data());

                logging::GALogger::d("init request content : %s", body);

                countMap[type] = countMap[type] + 1;
            });
        }

        const int GAHTTPApi::MaxCount = 10;
        std::map<EGASdkErrorType, int> GAHTTPApi::countMap = std::map<EGASdkErrorType, int>();

        std::vector<char> GAHTTPApi::createPayloadData(const char* payload, bool gzip)
        {
            std::vector<char> payloadData;

            if (gzip)
            {
                payloadData = utilities::GAUtilities::gzipCompress(payload);
                logging::GALogger::d("Gzip stats. Size: %d, Compressed: %d", strlen(payload), payloadData.size());
            }
            else
            {
                size_t s = strlen(payload);

                for(size_t i = 0; i < s; ++i)
                {
                    payloadData.push_back(payload[i]);
                }
            }

            return payloadData;
        }

        std::vector<char> GAHTTPApi::createRequest(Windows::Web::Http::HttpRequestMessage^ message, const std::string& url, const std::vector<char>& payloadData, bool gzip)
        {
            auto urlString = ref new Platform::String(utilities::GAUtilities::s2ws(url).c_str());
            message->RequestUri = ref new Windows::Foundation::Uri(urlString);
            message->Method = Windows::Web::Http::HttpMethod::Post;

            // create authorization hash
            std::string key = state::GAState::getGameSecret();
            char a[257];
            utilities::GAUtilities::hmacWithKey(key.c_str(), payloadData, a);
            auto authorization = ref new Platform::String(utilities::GAUtilities::s2ws(a).c_str());

            message->Headers->TryAppendWithoutValidation(L"Authorization", authorization);

            if (gzip)
            {
                Windows::Storage::Streams::InMemoryRandomAccessStream^ stream = createStream(std::string(payloadData.data())).get();
                message->Content = ref new Windows::Web::Http::HttpStreamContent(stream);
                message->Content->Headers->ContentEncoding->Append(ref new Windows::Web::Http::Headers::HttpContentCodingHeaderValue(ref new Platform::String(L"gzip")));
            }
            else
            {
                message->Content = ref new Windows::Web::Http::HttpStringContent(ref new Platform::String(utilities::GAUtilities::s2ws(payloadData.data()).c_str()), Windows::Storage::Streams::UnicodeEncoding::Utf8, ref new Platform::String(L"application/json"));
            }

            std::string r = utilities::GAUtilities::ws2s(authorization->Data());
            std::vector<char> result;
            for(size_t i = 0; i < r.size(); ++i)
            {
                result.push_back(r[i]);
            }
            result.push_back('\0');
            return result;
        }

        concurrency::task<Windows::Storage::Streams::InMemoryRandomAccessStream^> GAHTTPApi::createStream(std::string data)
        {
            using namespace Windows::Storage::Streams;
            using namespace Microsoft::WRL;
            using namespace Platform;
            using namespace concurrency;

            return create_task([=]()
            {
                size_t size = data.size();
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
                logging::GALogger::d("%s request. failed. Might be no connection. Status code: %s", requestId, utilities::GAUtilities::ws2s(statusCode.ToString()->Data()));
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
                logging::GALogger::d("%s request. 401 - Unauthorized.", requestId);
                return Unauthorized;
            }

            if (statusCode == Windows::Web::Http::HttpStatusCode::BadRequest)
            {
                logging::GALogger::d("%s request. 400 - Bad Request.", requestId);
                return BadRequest;
            }

            if (statusCode == Windows::Web::Http::HttpStatusCode::InternalServerError)
            {
                logging::GALogger::d("%s request. 500 - Internal Server Error.", requestId);
                return InternalServerError;
            }
            return UnknownResponseCode;
        }
    }
}

#endif
