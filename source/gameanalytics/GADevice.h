//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <string>

namespace gameanalytics
{
    namespace device
    {
        class GADevice
        {
        public:
        	static void setSdkGameEngineVersion(const std::string& sdkGameEngineVersion);
        	static const std::string getGameEngineVersion();
        	static void setGameEngineVersion(const std::string& gameEngineVersion);
            static void setConnectionType(const std::string& connectionType);
            static const std::string getConnectionType();
        	static const std::string getRelevantSdkVersion();
        	static const std::string getBuildPlatform();
        	static const std::string getOSVersion();
            static void setDeviceModel(const std::string& deviceModel);
        	static const std::string getDeviceModel();
            static void setDeviceManufacturer(const std::string& deviceManufacturer);
        	static const std::string getDeviceManufacturer();
            static void setWritablePath(const std::string& writablePath);
        	static const std::string getWritablePath();
            static void UpdateConnectionType();	

        private:
            static const std::string getOSVersionString();
            static const std::string deviceManufacturer();
            static const std::string deviceModel();
            static const std::string runtimePlatformToString();
            static const std::string getPersistentPath();

            static const std::string _buildPlatform;
            static const std::string _osVersion;
            static std::string _deviceModel;
            static std::string _deviceManufacturer;
            static std::string _writablepath;
            static std::string _sdkGameEngineVersion;
            static std::string _gameEngineVersion;
            static std::string _connectionType;
            static const std::string _sdkWrapperVersion;
        };
    }
} 