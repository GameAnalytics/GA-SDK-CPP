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
            static void addBusinessEvent(const std::string& currency, int amount, const std::string& itemType, const std::string& itemId, const std::string& cartType, const rapidjson::Value& fields);
            static void addResourceEvent(EGAResourceFlowType flowType, const std::string& currency, double amount, const std::string& itemType, const std::string& itemId, const rapidjson::Value& fields);
            static void addProgressionEvent(EGAProgressionStatus progressionStatus, const std::string& progression01, const std::string& progression02, const std::string& progression03, double score, bool sendScore, const rapidjson::Value& fields);
            static void addDesignEvent(const std::string& eventId, double value, bool sendValue, const rapidjson::Value& fields);
            static void addErrorEvent(EGAErrorSeverity severity, const std::string& message, const rapidjson::Value& fields);
            static const std::string progressionStatusString(EGAProgressionStatus progressionStatus);
            static const std::string errorSeverityString(EGAErrorSeverity errorSeverity);
            static const std::string resourceFlowTypeString(EGAResourceFlowType flowType);
            static void processEvents(const std::string& category, bool performCleanUp);

         private:
            static void processEventQueue();
            static void cleanupEvents();
            static void fixMissingSessionEndEvents();
            static void addEventToStore(const rapidjson::Value& eventData);
            static void addDimensionsToEvent(rapidjson::Value& eventData);
            static void addFieldsToEvent(rapidjson::Value& eventData, const rapidjson::Value& fields);
            static void updateSessionTime();

            static const std::string CategorySessionStart;
            static const std::string CategorySessionEnd;
            static const std::string CategoryDesign;
            static const std::string CategoryBusiness;
            static const std::string CategoryProgression;
            static const std::string CategoryResource;
            static const std::string CategoryError;
            static const double ProcessEventsIntervalInSeconds;
            static const int MaxEventCount;

            bool isRunning;
            bool keepRunning;
        };
    }
}
