//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#if USE_UWP || USE_TIZEN
#include <string>
#endif

namespace gameanalytics
{
    namespace device
    {
        class GADevice
        {
        public:
        	static void setSdkGameEngineVersion(const char* sdkGameEngineVersion);
        	static const char* getGameEngineVersion();
        	static void setGameEngineVersion(const char* gameEngineVersion);
            static void setConnectionType(const char* connectionType);
            static const char* getConnectionType();
        	static const char* getRelevantSdkVersion();
        	static const char* getBuildPlatform();
        	static const char* getOSVersion();
            static void setDeviceModel(const char* deviceModel);
        	static const char* getDeviceModel();
            static void setDeviceManufacturer(const char* deviceManufacturer);
        	static const char* getDeviceManufacturer();
            static void setWritablePath(const char* writablePath);
        	static const char* getWritablePath();
#if USE_UWP
            static const char* getDeviceId();
            static const char* getAdvertisingId();
#elif USE_TIZEN
            static const char* getDeviceId();
#endif
            static void UpdateConnectionType();

        private:
            static void initOSVersion();
            static void initDeviceManufacturer();
            static void initDeviceModel();
            static void initRuntimePlatform();
            static void initPersistentPath();
#if USE_UWP
            static const std::string deviceId();

            static const std::string _advertisingId;
            static const std::string _deviceId;
#elif USE_TIZEN
            static void initDeviceId();

            static char _deviceId[];
#endif

            static char _buildPlatform[];
            static char _osVersion[];
            static char _deviceModel[];
            static char _deviceManufacturer[];
            static char _writablepath[];
            static char _sdkGameEngineVersion[];
            static char _gameEngineVersion[];
            static char _connectionType[];
            static const char* _sdkWrapperVersion;
        };
    }
}
