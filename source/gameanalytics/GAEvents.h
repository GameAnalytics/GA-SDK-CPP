//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <string>
#include "Foundation/GASingleton.h"
#include "GameAnalytics.h"
#include "rapidjson/document.h"

namespace gameanalytics
{
    namespace events
    {
        class GAEvents : public GASingleton<GAEvents>
        {
         public:
            GAEvents();

            static void stopEventQueue();
            static void ensureEventQueueIsRunning();
            static void addSessionStartEvent();
            static void addSessionEndEvent();
            static void addBusinessEvent(const char* currency, int amount, const char* itemType, const char* itemId, const char* cartType, const rapidjson::Value& fields);
            static void addResourceEvent(EGAResourceFlowType flowType, const char* currency, double amount, const char* itemType, const char* itemId, const rapidjson::Value& fields);
            static void addProgressionEvent(EGAProgressionStatus progressionStatus, const char* progression01, const char* progression02, const char* progression03, double score, bool sendScore, const rapidjson::Value& fields);
            static void addDesignEvent(const char* eventId, double value, bool sendValue, const rapidjson::Value& fields);
            static void addErrorEvent(EGAErrorSeverity severity, const std::string& message, const rapidjson::Value& fields);
            static const std::string progressionStatusString(EGAProgressionStatus progressionStatus);
            static const std::string errorSeverityString(EGAErrorSeverity errorSeverity);
            static const std::string resourceFlowTypeString(EGAResourceFlowType flowType);
            static void processEvents(const char* category, bool performCleanUp);

         private:
            static void processEventQueue();
            static void cleanupEvents();
            static void fixMissingSessionEndEvents();
            static void addEventToStore(const rapidjson::Value& eventData);
            static void addDimensionsToEvent(rapidjson::Document& eventData);
            static void addFieldsToEvent(rapidjson::Document& eventData, rapidjson::Document& fields);
            static void updateSessionTime();

            static const char* CategorySessionStart;
            static const char* CategorySessionEnd;
            static const char* CategoryDesign;
            static const char* CategoryBusiness;
            static const char* CategoryProgression;
            static const char* CategoryResource;
            static const char* CategoryError;
            static const double ProcessEventsIntervalInSeconds;
            static const int MaxEventCount;

            bool isRunning;
            bool keepRunning;
        };
    }
}
