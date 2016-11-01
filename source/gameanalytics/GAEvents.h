//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <string>
#include "Foundation/GASingleton.h"
#include "GameAnalytics.h"
#include <json/json.h>

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
            static void addSessionEndEvent(Json::Int64 timeAdjustement = 0);
            static void addBusinessEvent(const STRING_TYPE& currency, int amount, const STRING_TYPE& itemType, const STRING_TYPE& itemId, const STRING_TYPE& cartType);
            static void addResourceEvent(EGAResourceFlowType flowType, const STRING_TYPE& currency, double amount, const STRING_TYPE& itemType, const STRING_TYPE& itemId);
            static void addProgressionEvent(EGAProgressionStatus progressionStatus, const STRING_TYPE& progression01, const STRING_TYPE& progression02, const STRING_TYPE& progression03, double score, bool sendScore);
            static void addDesignEvent(const STRING_TYPE& eventId, double value, bool sendValue);
            static void addErrorEvent(EGAErrorSeverity severity, const STRING_TYPE& message);
            static const STRING_TYPE progressionStatusString(EGAProgressionStatus progressionStatus);
            static const STRING_TYPE errorSeverityString(EGAErrorSeverity errorSeverity);
            static const STRING_TYPE resourceFlowTypeString(EGAResourceFlowType flowType);

         private:
            static void processEventQueue();
            static void processEvents(const STRING_TYPE& category, bool performCleanUp);
            static void cleanupEvents();
            static void fixMissingSessionEndEvents();
            static void addEventToStore(const Json::Value& eventData);
            static void addDimensionsToEvent(Json::Value& eventData);

            static const STRING_TYPE CategorySessionStart;
            static const STRING_TYPE CategorySessionEnd;
            static const STRING_TYPE CategoryDesign;
            static const STRING_TYPE CategoryBusiness;
            static const STRING_TYPE CategoryProgression;
            static const STRING_TYPE CategoryResource;
            static const STRING_TYPE CategoryError;
            static const double ProcessEventsIntervalInSeconds;
            static const int MaxEventCount;

            bool isRunning;
            bool keepRunning;
        };
    }
}
