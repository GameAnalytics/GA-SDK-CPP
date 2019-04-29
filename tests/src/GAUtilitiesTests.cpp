//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>

#include <GAUtilities.h>
#include <random>

// test helpers
#include "helpers/GATestHelpers.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

TEST(GAUtilities, testHmacWithKey)
{
    {
        char mac[257] = "";
        gameanalytics::utilities::GAUtilities::hmacWithKey("test1", {'t','e','s','t','2'}, mac);
        ASSERT_STREQ(mac, "E+sBF4BA9mLvVlfwHx53G2poUPwEUZ1f37oVrgHhOFQ=");
    }


    rapidjson::Document d;
    d.SetArray();
    rapidjson::Document::AllocatorType& a = d.GetAllocator();

    {
        rapidjson::Value v(rapidjson::kObjectType);
        v.AddMember("v", 2, a);
        v.AddMember("user_id", "test", a);
        d.PushBack(v, a);
    }
    {
        rapidjson::Value v(rapidjson::kObjectType);
        v.AddMember("v", 3, a);
        v.AddMember("user_id", "test2", a);
        d.PushBack(v, a);
    }

    rapidjson::StringBuffer buffer;
    {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);
    }

    const char* JSONstring = buffer.GetString();

    std::vector<char> payloadData;
    size_t size = strlen(JSONstring);

    for(size_t i = 0; i < size; ++i)
    {
        payloadData.push_back(JSONstring[i]);
    }
    payloadData.push_back('\0');

    {
        char mac[257] = "";
        gameanalytics::utilities::GAUtilities::hmacWithKey("test1", payloadData, mac);
        ASSERT_STREQ(mac, "DaiXv2Qe+20P0FoYYghca4glPJQTahtDS5VGQzIvjTQ=");
    }
}

TEST(GAUtilities, testGenerateUUID)
{
    char guid[65] = "";
    gameanalytics::utilities::GAUtilities::generateUUID(guid);
    ASSERT_EQ(strlen(guid), 36);
}

TEST(GAUtilities, testJsonToString)
{
    rapidjson::Document jsonObject;
    jsonObject.SetObject();
    rapidjson::Document::AllocatorType& a = jsonObject.GetAllocator();
    jsonObject.AddMember("user_id", "test", a);
    jsonObject.AddMember("v", 2, a);

    rapidjson::StringBuffer buffer;
    {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        jsonObject.Accept(writer);
    }

    ASSERT_STREQ(buffer.GetString(), "{\"user_id\":\"test\",\"v\":2}");
}

TEST(GAUtilities, testArrayOfJsonToString)
{
    rapidjson::Document d;
    d.SetArray();
    rapidjson::Document::AllocatorType& a = d.GetAllocator();

    {
        rapidjson::Value v(rapidjson::kObjectType);
        v.AddMember("v", 2, a);
        v.AddMember("user_id", "test", a);
        d.PushBack(v, a);
    }
    {
        rapidjson::Value v(rapidjson::kObjectType);
        v.AddMember("v", 3, a);
        v.AddMember("user_id", "test2", a);
        d.PushBack(v, a);
    }

    rapidjson::StringBuffer buffer;
    {
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        d.Accept(writer);
    }

    ASSERT_STREQ(buffer.GetString(), "[{\"v\":2,\"user_id\":\"test\"},{\"v\":3,\"user_id\":\"test2\"}]");
}

TEST(GAUtilities, testStringMatch)
{
    const char* regex1 = "^[A-z0-9]{40}$";
    const char* regex2 = "^[A-Za-z0-9\\s\\-_\\.\\(\\)\\!\\?]{1,64}(:[A-Za-z0-9\\s\\-_\\.\\(\\)\\!\\?]{1,64}){0,4}$";

    ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("Assdadskfdstdfgdfrtmddrhbgfdtytytt4kjkk3", regex1));

    ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("kfdstdfgdfrtmddrhbgfdtytytt4kk34lk3k43", regex1));
    ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("Æssdadskfdstdfgdfrtmddrhbgfdtytytt4kk34l", regex1));
    ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("___", regex1));
    ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("ØØøøstdfgdfrtmddrhbgfdtytytt4kk34lk3k43k", regex1));
    ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("_ssdadskasældkælskd678kalsk3434k3l4kk34l", regex1));

    ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234:1234:1234:1234", regex2));
    ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234:1234:1234", regex2));
    ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234:1234", regex2));
    ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234", regex2));
    ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("1234", regex2));
    ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("___", regex2));
    ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("-_-():!?", regex2));

    ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234:1234:12345678901234567890123456789012345678901234567890123456789012345678901234567890", regex2));
    ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("12345678901234567890123456789012345678901234567890123456789012345678901234567890:1234:1234:1234", regex2));
    ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("123åæø", regex2));
    ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234:1234:1234:1234:1234", regex2));
}
