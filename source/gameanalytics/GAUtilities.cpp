//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

//#include <climits>
#include "GAUtilities.h"
#include "GALogger.h"
#include <algorithm>
#include <regex>
#if USE_UWP
#include <Objbase.h>
#else
#include <hmac_sha2.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#endif

// From crypto
#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"

namespace gameanalytics
{
    namespace utilities
    {
        // Compress a STL string using zlib with given compression level and return the binary data.
        // Note: the zlib header is supressed
        static std::string deflate_string(const std::string& str,
            int compressionlevel = Z_BEST_COMPRESSION)
        {
            // z_stream is zlib's control structure
            z_stream zs;
            memset(&zs, 0, sizeof(zs));

            /* windowsize is negative to suppress Zlib header */
            if (Z_OK != deflateInit2(&zs, compressionlevel, MZ_DEFLATED, -MZ_DEFAULT_WINDOW_BITS, 9, MZ_DEFAULT_STRATEGY))
            {
                throw(std::runtime_error("deflateInit failed while compressing."));
            }

            zs.next_in = (Bytef*)str.data();
            //zs.next_in = reinterpret_cast<Bytef*>(str.data());

            // set the z_stream's input
            zs.avail_in = str.size();
            int ret;
            static char outbuffer[32768];
            std::string outstring;

            // retrieve the compressed bytes blockwise
            do
            {
                zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
                zs.avail_out = sizeof(outbuffer);

                ret = deflate(&zs, Z_FINISH);

                if (outstring.size() < zs.total_out)
                {
                    // append the block to the output string
                    outstring.append(outbuffer,
                        zs.total_out - outstring.size());
                }
            } while (ret == Z_OK);

            deflateEnd(&zs);

            if (ret != Z_STREAM_END) {
                // an error occurred that was not EOF
                std::ostringstream oss;
                oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
                throw(std::runtime_error(oss.str()));
            }
            return outstring;
        }

        // TODO(nikolaj): explain template?
        template <typename T>
        T swap_endian(T u)
        {
            static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

            union
            {
                T u;
                unsigned char u8[sizeof(T)];
            } source, dest;

            source.u = u;

            for (size_t k = 0; k < sizeof(T); k++)
                dest.u8[k] = source.u8[sizeof(T) - k - 1];

            return dest.u;
        }

        uint32 htonl2(uint32 v)
        {
            uint32 result = 0;
            result |= (v & 0xFF000000) >> 24;
            result |= (v & 0x00FF0000) >> 8;
            result |= (v & 0x0000FF00) << 8;
            result |= (v & 0x000000FF) << 24;

            return result;
        }

        uint32 to_little_endian(uint32 v)
        {
            // convert to big endian
            v = htonl2(v);

            // and to little endian, because gzip wants it so.
            v = swap_endian(v);

            return v;
        }

#if !USE_UWP
        static char nb_base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

        int GAUtilities::base64_needed_encoded_length(int length_of_data)
        {
            int nb_base64_chars = (length_of_data + 2) / 3 * 4;

            return nb_base64_chars +               /* base64 char incl padding */
            (nb_base64_chars - 1) / 76 +    /* newlines */
            1;                              /* NUL termination of string */
        }

        /**
         * buf_ is allocated by malloc(3).The size is grater than nb_base64_needed_encoded_length(src_len).
         */
        void GAUtilities::base64_encode(const unsigned char * src, int src_len, unsigned char *buf_)
        {
            unsigned char *buf = buf_;
            int i = 0;
            int j = 0;
            unsigned char char_array_3[3] = {0};
            unsigned char char_array_4[4] = {0};

            while (src_len--)
            {
                char_array_3[i++] = *(src++);
                if (i == 3)
                {
                    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                    char_array_4[1] =
                    ((char_array_3[0] & 0x03) << 4) +
                    ((char_array_3[1] & 0xf0) >> 4);
                    char_array_4[2] =
                    ((char_array_3[1] & 0x0f) << 2) +
                    ((char_array_3[2] & 0xc0) >> 6);
                    char_array_4[3] = char_array_3[2] & 0x3f;
                    for (i = 0; (i < 4); i++) {
                        *buf++ = nb_base64_chars[char_array_4[i]];
                    }
                    i = 0;
                }
            }

            if (i)
            {
                for (j = i; j < 3; j++)
                {
                    char_array_3[j] = '\0';
                }

                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] =
                ((char_array_3[0] & 0x03) << 4) +
                ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] =
                ((char_array_3[1] & 0x0f) << 2) +
                ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (j = 0; (j < i + 1); j++)
                {
                    *buf++ = nb_base64_chars[char_array_4[j]];
                }

                while ((i++ < 3))
                {
                    *buf++ = '=';
                }
            }
            *buf++ = '\0';
        }
#endif

        // gzip compresses a string
        static std::string compress_string_gzip(const std::string& str,
            int compressionlevel = Z_BEST_COMPRESSION)
        {
            // https://tools.ietf.org/html/rfc1952
            std::stringstream ss;
            std::string deflated = deflate_string(str, compressionlevel);

            static const char gzip_header[10] =
            { '\037', '\213', Z_DEFLATED, 0,
                0, 0, 0, 0, /* mtime */
                0, 0x03 /* Unix OS_CODE */
            };

            // Note: apparently, the crc is never validated on ther server side. So I'm not sure, if I have to convert it to little endian.
            uint32_t crc = to_little_endian(crc32(0, (unsigned char*)str.data(), str.size()));
            uint32 size  = to_little_endian(str.size());

            ss.write(gzip_header, sizeof(gzip_header));
            ss.write(deflated.data(), deflated.size());
            ss.write((const char*)&crc, 4);
            ss.write((const char*)&size, 4);

            return ss.str();
        }

        const char GAUtilities::getPathSeparatorChar()
        {
#ifdef _WIN32
            return '\\';
#else
            return '/';
#endif
        }

        // TODO(nikolaj): explain function
        std::string GAUtilities::generateUUID()
        {
#if USE_UWP
            GUID result;
            HRESULT hr = CoCreateGuid(&result);

            if (SUCCEEDED(hr))
            {
                // Generate new GUID.
                Platform::Guid guid(result);
                auto guidString = std::wstring(guid.ToString()->Data());

                // Remove curly brackets.
                auto sessionId = guidString.substr(1, guidString.length() - 2);
                return ws2s(sessionId);
            }

            throw Platform::Exception::CreateException(hr);
#else
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            return boost::lexical_cast<std::string>(uuid);
#endif
        }

        // TODO(nikolaj): explain function
        std::string GAUtilities::hmacWithKey(const std::string& key, const std::string& data)
        {
#if USE_UWP
            using namespace Platform;
            using namespace Windows::Security::Cryptography::Core;
            using namespace Windows::Security::Cryptography;

            auto keyString = ref new String(utilities::GAUtilities::s2ws(key).c_str());
            auto alg = MacAlgorithmProvider::OpenAlgorithm(MacAlgorithmNames::HmacSha256);
            Platform::Array<unsigned char>^ byteArray = ref new Platform::Array<unsigned char>(data.size());
            for (unsigned int i = 0; i < data.size(); ++i)
            {
                byteArray[i] = data[i];
            }
            auto dataBuffer = CryptographicBuffer::CreateFromByteArray(byteArray);
            auto secretKeyBuffer = CryptographicBuffer::ConvertStringToBinary(keyString, BinaryStringEncoding::Utf8);
            auto hmacKey = alg->CreateKey(secretKeyBuffer);

            auto hashedJsonBuffer = CryptographicEngine::Sign(hmacKey, dataBuffer);
            auto hashedJsonBase64 = CryptographicBuffer::EncodeToBase64String(hashedJsonBuffer);
            return utilities::GAUtilities::ws2s(hashedJsonBase64->Data());
#else
            unsigned char mac[SHA256_DIGEST_SIZE];
            hmac_sha256(
                (unsigned char*)key.data(),
                key.size(),
                (unsigned char*)data.data(),
                data.size(),
                mac,
                SHA256_DIGEST_SIZE
            );
            int output_size = base64_needed_encoded_length(SHA256_DIGEST_SIZE);
            std::vector<unsigned char> ret(output_size);
            GAUtilities::base64_encode(mac, SHA256_DIGEST_SIZE, ret.data());

            return {
                (char*)ret.data(),
                (char*)ret.data()+ret.size()
            };
#endif
        }

        // TODO(nikolaj): explain function
        bool GAUtilities::stringMatch(const std::string& string, const std::string& pattern)
        {
            try
            {
                std::regex expression(pattern);
                return std::regex_match(string, expression);
            }
            catch (const std::regex_error& e)
            {
                logging::GALogger::e("failed to parse regular expression '" + pattern + "', code: " + std::to_string(e.code()) + ", what: " + e.what());
                logging::GALogger::e("Please note, that the gnustl might not have regex support yet: https://gcc.gnu.org/onlinedocs/libstdc++/manual/status.html");
                #if _DEBUG
                throw;
                #else
                return true;
                #endif
            }
        }

        std::string GAUtilities::gzipCompress(const std::string& data)
        {
            return compress_string_gzip(data);
        }

        // TODO(nikolaj): explain function
        std::string GAUtilities::jsonToString(const Json::Value& obj)
        {
            Json::FastWriter writer;
            std::string output = writer.write(obj);
            return output;
        }

        // TODO(nikolaj): explain function
        std::string GAUtilities::arrayOfObjectsToJsonString(const std::vector<Json::Value>& arr)
        {
            Json::Value json_array(Json::arrayValue);
            for (const auto& x : arr)
            {
                json_array.append(x);
            }
            return jsonToString(json_array);
        }

        // TODO(nikolaj): explain function
        Json::Value GAUtilities::jsonFromString(const std::string& string)
        {
            Json::Reader reader;
            Json::Value parsingResult;
            bool parsed = reader.parse(string, parsingResult);
            if (!parsed)
            {
                return {};
            }
            return parsingResult;
        }

        // TODO(nikolaj): explain function
        bool GAUtilities::stringVectorContainsString(std::vector<std::string> vector, std::string search)
        {
            if (vector.size() == 0) {
                return false;
            }

            std::vector<std::string>::iterator it = std::find(vector.begin(), vector.end(), search);
            return it != vector.end();
        }

        // using std::chrono to get time
        Json::Int64 GAUtilities::timeIntervalSince1970()
        {
            return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }

        // TODO(nikolaj): explain function
        std::string GAUtilities::uppercaseString(std::string s)
        {
            std::transform(s.begin(), s.end(), s.begin(), toupper);
            return s;
        }


        // TODO(nikolaj): explain function
        std::string GAUtilities::lowercaseString(std::string s)
        {
            std::transform(s.begin(), s.end(), s.begin(), tolower);
            return s;
        }

        // TODO(nikolaj): explain function
        std::string GAUtilities::joinStringArray(const std::vector<std::string>& v, const std::string& delimiter)
        {
            std::stringstream s;
            copy(v.begin(), v.end(), std::ostream_iterator<std::string>(s, delimiter.c_str()));
            return s.str();
        }
    }
}
