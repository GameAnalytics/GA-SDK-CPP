//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <GAState.h>
#include <json/json.h>

#include "helpers/GATestHelpers.h"

TEST(GAStateTest, testValidateAndCleanCustomFields)
{
    Json::Value map;

    {
        map = Json::Value();
        for(int i = 0; i < 100; ++i)
        {
            map[GATestHelpers::getRandomString(4)] = GATestHelpers::getRandomString(4);
        }
    }
    ASSERT_TRUE(gameanalytics::state::GAState::validateAndCleanCustomFields(map).size() == 50);

    {
        map = Json::Value();
        for(int i = 0; i < 50; ++i)
        {
            map[GATestHelpers::getRandomString(4)] = GATestHelpers::getRandomString(4);
        }
    }
    ASSERT_TRUE(gameanalytics::state::GAState::validateAndCleanCustomFields(map).size() == 50);

    {
        map = Json::Value();
        for(int i = 0; i < 50; ++i)
        {
            map[GATestHelpers::getRandomString(4)] = "";
        }
    }
    ASSERT_TRUE(gameanalytics::state::GAState::validateAndCleanCustomFields(map).size() == 0);

    {
        map = Json::Value();
        for(int i = 0; i < 50; ++i)
        {
            map[GATestHelpers::getRandomString(4)] = GATestHelpers::getRandomString(257);
        }
    }
    ASSERT_TRUE(gameanalytics::state::GAState::validateAndCleanCustomFields(map).size() == 0);

    {
        map = Json::Value();
        map[""] = GATestHelpers::getRandomString(4);
    }
    ASSERT_TRUE(gameanalytics::state::GAState::validateAndCleanCustomFields(map).size() == 0);

    {
        map = Json::Value();
        map["___"] = GATestHelpers::getRandomString(4);
    }
    ASSERT_TRUE(gameanalytics::state::GAState::validateAndCleanCustomFields(map).size() == 1);

    {
        map = Json::Value();
        map["_&_"] = GATestHelpers::getRandomString(4);
    }
    ASSERT_TRUE(gameanalytics::state::GAState::validateAndCleanCustomFields(map).size() == 0);

    {
        map = Json::Value();
        map[GATestHelpers::getRandomString(65)] = GATestHelpers::getRandomString(4);
    }
    ASSERT_TRUE(gameanalytics::state::GAState::validateAndCleanCustomFields(map).size() == 0);

    {
        map = Json::Value();
        map[GATestHelpers::getRandomString(4)] = 100;
    }
    ASSERT_TRUE(gameanalytics::state::GAState::validateAndCleanCustomFields(map).size() == 1);

    {
        map = Json::Value();
        map[GATestHelpers::getRandomString(4)] = true;
    }
    ASSERT_TRUE(gameanalytics::state::GAState::validateAndCleanCustomFields(map).size() == 0);
}
