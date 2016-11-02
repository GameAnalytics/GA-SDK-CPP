#if USE_UWP
#include "GAHTTPApi.h"
#include "GAState.h"
#include "GALogger.h"
#include "GAUtilities.h"

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
            auto message = ref new Windows::Web::Http::HttpRequestMessage();

            std::string authorization = createRequest(message, url, payloadData, useGzip);

            try
            {
                curl.perform();
            }
            catch (curl::curl_easy_exception error)
            {
                error.print_traceback();
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
    }
}

#endif