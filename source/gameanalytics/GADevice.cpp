//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#include "GADevice.h"
#include <boost/filesystem/operations.hpp>

namespace gameanalytics
{
    namespace device
    {
        const std::string GADevice::_buildPlatform = GADevice::runtimePlatformToString();
        const std::string GADevice::_osVersion = GADevice::getOSVersionString();
        std::string GADevice::_deviceModel = GADevice::deviceModel();
        std::string GADevice::_deviceManufacturer = GADevice::deviceManufacturer();
        std::string GADevice::_writablepath = GADevice::getPersistentPath();
        std::string GADevice::_sdkGameEngineVersion;
        std::string GADevice::_gameEngineVersion;
        std::string GADevice::_connectionType = "offline";
        const std::string GADevice::_sdkWrapperVersion = "cpp 1.0.1";

        void GADevice::setSdkGameEngineVersion(const std::string& sdkGameEngineVersion)
        {
            GADevice::_sdkGameEngineVersion = sdkGameEngineVersion;
        }

        const std::string GADevice::getGameEngineVersion()
        {
            return GADevice::_gameEngineVersion;
        }

        void GADevice::setGameEngineVersion(const std::string& gameEngineVersion)
        {
            GADevice::_gameEngineVersion = gameEngineVersion;
        }

        void GADevice::setConnectionType(const std::string& connectionType)
        {
            GADevice::_connectionType = connectionType;
        }

        const std::string GADevice::getConnectionType()
        {
            return GADevice::_connectionType;
        }

        const std::string GADevice::getRelevantSdkVersion()
        {
            if(!GADevice::_sdkGameEngineVersion.empty())
            {
                return GADevice::_sdkGameEngineVersion;
            }

            return GADevice::_sdkWrapperVersion;
        }

        const std::string GADevice::getBuildPlatform()
        {
            return GADevice::_buildPlatform;
        }

        const std::string GADevice::getOSVersion()
        {
            return GADevice::_osVersion;
        }

        void GADevice::setDeviceModel(const std::string& deviceModel)
        {
            GADevice::_deviceModel = deviceModel;
        }

        const std::string GADevice::getDeviceModel()
        {
            return GADevice::_deviceModel;
        }

        void GADevice::setDeviceManufacturer(const std::string& deviceManufacturer)
        {
            GADevice::_deviceManufacturer = deviceManufacturer;
        }

        const std::string GADevice::getDeviceManufacturer()
        {
            return GADevice::_deviceManufacturer;
        }

        void GADevice::setWritablePath(const std::string& writablepath)
        {
            GADevice::_writablepath = writablepath;
        }

        const std::string GADevice::getWritablePath()
        {
            return GADevice::_writablepath;
        }

        void GADevice::UpdateConnectionType()
        {
            GADevice::_connectionType = "lan";
        }

        const std::string GADevice::getOSVersionString()
        {
            return GADevice::getBuildPlatform() + " 0.0.0";
        }

        const std::string GADevice::deviceManufacturer()
        {
            return "unknown";
        }

        const std::string GADevice::deviceModel()
        {
            return "unknown";
        }

        const std::string GADevice::runtimePlatformToString()
        {
#if defined(__MACH__) || defined(__APPLE__)
            return "mac_osx";
#elif defined(_WIN32)
            return "windows";
#elif defined(__linux__) || defined(__unix__) || defined(__unix) || defined(unix)
            return "linux";
#else
            return "unknown";
#endif
        }

        const std::string GADevice::getPersistentPath()
        {
            return boost::filesystem::detail::temp_directory_path().string();
        }
    }
}