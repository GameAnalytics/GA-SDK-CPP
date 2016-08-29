//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

//#pragma once

#include <functional>

class GATimer
{
 public:
    void stop();
    bool isRunning();
    GATimer(double interval, std::function<void()> callback);
};
