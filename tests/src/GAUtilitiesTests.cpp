//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>

#include <GAUtilities.h>
#include <random>

// test helpers
#include "helpers/GATestHelpers.h"

// TEST(GAUtilities, testHmacWithKey)
// {
//     ASSERT_STREQ(gameanalytics::utilities::GAUtilities::hmacWithKey("test1", "test2").c_str(), "E+sBF4BA9mLvVlfwHx53G2poUPwEUZ1f37oVrgHhOFQ=");
//
//     Json::Value jsonObject1;
//     jsonObject1["v"] = 2;
//     jsonObject1["user_id"] = "test";
//
//     Json::Value jsonObject2;
//     jsonObject2["v"] = 3;
//     jsonObject2["user_id"] = "test2";
//
//     std::vector<Json::Value> eventArray;
//     eventArray.push_back(jsonObject1);
//     eventArray.push_back(jsonObject2);
//
//     std::string JSONstring = gameanalytics::utilities::GAUtilities::arrayOfObjectsToJsonString(eventArray);
//
//     ASSERT_STREQ(gameanalytics::utilities::GAUtilities::hmacWithKey("test1", JSONstring).c_str(), "DIsYMFmB6RUdQVCYPNWn/oSKIbtFMr2aMfQbwgVjE9I=");
// }
//
// TEST(GAUtilities, testGenerateUUID)
// {
//     ASSERT_EQ(gameanalytics::utilities::GAUtilities::generateUUID().size(), 36);
// }
//
// TEST(GAUtilities, testJsonToString)
// {
//     Json::Value jsonObject;
//     jsonObject["user_id"] = "test";
//     jsonObject["v"] = 2;
//
//     ASSERT_STREQ(gameanalytics::utilities::GAUtilities::jsonToString(jsonObject).c_str(), "{\"user_id\":\"test\",\"v\":2}\n");
// }
//
// TEST(GAUtilities, testArrayOfJsonToString)
// {
//     Json::Value jsonObject1;
//     jsonObject1["v"] = 2;
//     jsonObject1["user_id"] = "test";
//
//     Json::Value jsonObject2;
//     jsonObject2["v"] = 3;
//     jsonObject2["user_id"] = "test2";
//
//     std::vector<Json::Value> payloadArray;
//     payloadArray.push_back(jsonObject1);
//     payloadArray.push_back(jsonObject2);
//
//     ASSERT_STREQ(gameanalytics::utilities::GAUtilities::arrayOfObjectsToJsonString(payloadArray).c_str(), "[{\"user_id\":\"test\",\"v\":2},{\"user_id\":\"test2\",\"v\":3}]\n");
// }
//
// TEST(GAUtilities, testStringMatch)
// {
//     std::string regex1 = "^[A-z0-9]{40}$";
//     std::string regex2 = "^[A-Za-z0-9\\s\\-_\\.\\(\\)\\!\\?]{1,64}(:[A-Za-z0-9\\s\\-_\\.\\(\\)\\!\\?]{1,64}){0,4}$";
//
//     ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("Assdadskfdstdfgdfrtmddrhbgfdtytytt4kjkk3", regex1));
//
//     ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("kfdstdfgdfrtmddrhbgfdtytytt4kk34lk3k43", regex1));
//     ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("Æssdadskfdstdfgdfrtmddrhbgfdtytytt4kk34l", regex1));
//     ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("___", regex1));
//     ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("ØØøøstdfgdfrtmddrhbgfdtytytt4kk34lk3k43k", regex1));
//     ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("_ssdadskasældkælskd678kalsk3434k3l4kk34l", regex1));
//
//     ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234:1234:1234:1234", regex2));
//     ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234:1234:1234", regex2));
//     ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234:1234", regex2));
//     ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234", regex2));
//     ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("1234", regex2));
//     ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("___", regex2));
//     ASSERT_TRUE(gameanalytics::utilities::GAUtilities::stringMatch("-_-():!?", regex2));
//
//     ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234:1234:12345678901234567890123456789012345678901234567890123456789012345678901234567890", regex2));
//     ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("12345678901234567890123456789012345678901234567890123456789012345678901234567890:1234:1234:1234", regex2));
//     ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("123åæø", regex2));
//     ASSERT_FALSE(gameanalytics::utilities::GAUtilities::stringMatch("1234:1234:1234:1234:1234:1234", regex2));
// }
