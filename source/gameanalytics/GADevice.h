//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <string>
#include "defines.h"

namespace gameanalytics
{
    namespace device
    {
        class GADevice
        {
        public:
        	static void setSdkGameEngineVersion(const STRING_TYPE& sdkGameEngineVersion);
        	static const STRING_TYPE getGameEngineVersion();
        	static void setGameEngineVersion(const STRING_TYPE& gameEngineVersion);
            static void setConnectionType(const STRING_TYPE& connectionType);
            static const STRING_TYPE getConnectionType();
        	static const STRING_TYPE getRelevantSdkVersion();
        	static const STRING_TYPE getBuildPlatform();
        	static const STRING_TYPE getOSVersion();
            static void setDeviceModel(const STRING_TYPE& deviceModel);
        	static const STRING_TYPE getDeviceModel();
            static void setDeviceManufacturer(const STRING_TYPE& deviceManufacturer);
        	static const STRING_TYPE getDeviceManufacturer();
            static void setWritablePath(const STRING_TYPE& writablePath);
        	static const STRING_TYPE getWritablePath();
            static void UpdateConnectionType();	

        private:
            static const STRING_TYPE getOSVersionString();
            static const STRING_TYPE deviceManufacturer();
            static const STRING_TYPE deviceModel();
            static const STRING_TYPE runtimePlatformToString();
            static const STRING_TYPE getPersistentPath();
#if USE_UWP
            static const STRING_TYPE getDeviceId();
#endif

            static const STRING_TYPE _buildPlatform;
            static const STRING_TYPE _osVersion;
            static STRING_TYPE _deviceModel;
            static STRING_TYPE _deviceManufacturer;
            static STRING_TYPE _writablepath;
            static STRING_TYPE _sdkGameEngineVersion;
            static STRING_TYPE _gameEngineVersion;
            static STRING_TYPE _connectionType;
            static const STRING_TYPE _sdkWrapperVersion;
        };
    }
} 
