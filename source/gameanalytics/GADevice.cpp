//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#include "GADevice.h"
#if USE_UWP
#include <Windows.h>
#include <sstream>
#include <locale>
#include <codecvt>
#else
#include <boost/filesystem.hpp>
#endif

namespace gameanalytics
{
    namespace device
    {
        const STRING_TYPE GADevice::_buildPlatform = GADevice::runtimePlatformToString();
        const STRING_TYPE GADevice::_osVersion = GADevice::getOSVersionString();
        STRING_TYPE GADevice::_deviceModel = GADevice::deviceModel();
        STRING_TYPE GADevice::_deviceManufacturer = GADevice::deviceManufacturer();
        STRING_TYPE GADevice::_writablepath = GADevice::getPersistentPath();
        STRING_TYPE GADevice::_sdkGameEngineVersion;
        STRING_TYPE GADevice::_gameEngineVersion;
        STRING_TYPE GADevice::_connectionType = TEXT("");
        const STRING_TYPE GADevice::_sdkWrapperVersion = TEXT("cpp 1.1.1");

        void GADevice::setSdkGameEngineVersion(const STRING_TYPE& sdkGameEngineVersion)
        {
            GADevice::_sdkGameEngineVersion = sdkGameEngineVersion;
        }

        const STRING_TYPE GADevice::getGameEngineVersion()
        {
            return GADevice::_gameEngineVersion;
        }

        void GADevice::setGameEngineVersion(const STRING_TYPE& gameEngineVersion)
        {
            GADevice::_gameEngineVersion = gameEngineVersion;
        }

        void GADevice::setConnectionType(const STRING_TYPE& connectionType)
        {
            GADevice::_connectionType = connectionType;
        }

        const STRING_TYPE GADevice::getConnectionType()
        {
            return GADevice::_connectionType;
        }

        const STRING_TYPE GADevice::getRelevantSdkVersion()
        {
            if(!GADevice::_sdkGameEngineVersion.empty())
            {
                return GADevice::_sdkGameEngineVersion;
            }

            return GADevice::_sdkWrapperVersion;
        }

        const STRING_TYPE GADevice::getBuildPlatform()
        {
            return GADevice::_buildPlatform;
        }

        const STRING_TYPE GADevice::getOSVersion()
        {
            return GADevice::_osVersion;
        }

        void GADevice::setDeviceModel(const STRING_TYPE& deviceModel)
        {
            GADevice::_deviceModel = deviceModel;
        }

        const STRING_TYPE GADevice::getDeviceModel()
        {
            return GADevice::_deviceModel;
        }

        void GADevice::setDeviceManufacturer(const STRING_TYPE& deviceManufacturer)
        {
            GADevice::_deviceManufacturer = deviceManufacturer;
        }

        const STRING_TYPE GADevice::getDeviceManufacturer()
        {
            return GADevice::_deviceManufacturer;
        }

        void GADevice::setWritablePath(const STRING_TYPE& writablepath)
        {
            GADevice::_writablepath = writablepath;
        }

        const STRING_TYPE GADevice::getWritablePath()
        {
            return GADevice::_writablepath;
        }

        void GADevice::UpdateConnectionType()
        {
            GADevice::_connectionType = TEXT("lan");
        }

        const STRING_TYPE GADevice::getOSVersionString()
        {
#if USE_UWP
            auto deviceFamilyVersion = Windows::System::Profile::AnalyticsInfo::VersionInfo->DeviceFamilyVersion;
            std::wistringstream stringStream(deviceFamilyVersion->Data());
            unsigned long long version;

            stringStream >> version;
            unsigned long long major = (version & 0xFFFF000000000000L) >> 48;
            unsigned long long minor = (version & 0x0000FFFF00000000L) >> 32;
            unsigned long long build = (version & 0x00000000FFFF0000L) >> 16;
            std::wostringstream stream;
            stream << getBuildPlatform() << TEXT(" ") << major << TEXT(".") << minor << TEXT(".") << build;
            return stream.str();
#else
            return GADevice::getBuildPlatform() + " 0.0.0";
#endif
        }

        const STRING_TYPE GADevice::deviceManufacturer()
        {
#if USE_UWP
            auto info = ref new Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
            return info->SystemManufacturer->Data();
#else
            return "unknown";
#endif
        }

        const STRING_TYPE GADevice::deviceModel()
        {
#if USE_UWP
            auto info = ref new Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
            return info->SystemProductName->Data();
#else
            return "unknown";
#endif
        }

        const STRING_TYPE GADevice::runtimePlatformToString()
        {
#if USE_UWP
            auto deviceFamily = Windows::System::Profile::AnalyticsInfo::VersionInfo->DeviceFamily;

            if (deviceFamily == "Windows.Mobile")
            {
                return TEXT("uwp_mobile");
            }
            else if (deviceFamily == "Windows.Desktop")
            {
                return TEXT("uwp_desktop");
            }
            else if (deviceFamily == "Windows.Universal")
            {
                return TEXT("uwp_iot");
            }
            else if (deviceFamily == "Windows.Xbox")
            {
                return TEXT("uwp_console");
            }
            else if (deviceFamily == "Windows.Team")
            {
                return TEXT("uwp_surfacehub");
            }
            else
            {
                return deviceFamily->Data();
            }
#else
#if defined(__MACH__) || defined(__APPLE__)
            return "mac_osx";
#elif defined(_WIN32)
            return "windows";
#elif defined(__linux__) || defined(__unix__) || defined(__unix) || defined(unix)
            return "linux";
#else
            return "unknown";
#endif
#endif
        }

        const STRING_TYPE GADevice::getPersistentPath()
        {
#if USE_UWP
            return Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data();
#else
            return boost::filesystem::detail::temp_directory_path().string();
#endif
        }
    }
}
