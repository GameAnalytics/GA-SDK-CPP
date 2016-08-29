//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <json/json.h>
#include <string>

// TODO(nikolaj) remove
//#define property(type, name) type name = type()

#if defined(__ANDROID_API__) && !defined(_LIBCPP_VERSION)
// gnustl_static does not support std::to_string yet, so we implement it here

namespace std
{
    template <typename T>
    std::string to_string(const T& o)
    {
        std::stringstream ss;
        ss << o;
        return ss.str();
    }
}

#endif /* __ANDROID_API__ */
