//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#include "GADevice.h"
#include "GAUtilities.h"
#if USE_UWP
#include <Windows.h>
#include <sstream>
#elif USE_TIZEN
#include <system_info.h>
#include <app_common.h>
#include <algorithm>
#elif _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace gameanalytics
{
    namespace device
    {
        const std::string GADevice::_buildPlatform = GADevice::runtimePlatformToString();
        const std::string GADevice::_osVersion = GADevice::getOSVersionString();
        std::string GADevice::_deviceModel = GADevice::deviceModel();
#if USE_UWP
        const std::string GADevice::_advertisingId = utilities::GAUtilities::ws2s(Windows::System::UserProfile::AdvertisingManager::AdvertisingId->Data());
        const std::string GADevice::_deviceId = GADevice::deviceId();
#elif USE_TIZEN
        const std::string GADevice::_deviceId = GADevice::deviceId();
#endif
        std::string GADevice::_deviceManufacturer = GADevice::deviceManufacturer();
        std::string GADevice::_writablepath = GADevice::getPersistentPath();
        std::string GADevice::_sdkGameEngineVersion;
        std::string GADevice::_gameEngineVersion;
        std::string GADevice::_connectionType = "";
#if USE_UWP
        const std::string GADevice::_sdkWrapperVersion = "uwp_cpp 1.3.1";
#elif USE_TIZEN
        const std::string GADevice::_sdkWrapperVersion = "tizen 1.3.1";
#else
        const std::string GADevice::_sdkWrapperVersion = "cpp 1.3.1";
#endif

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
#if USE_UWP
            auto deviceFamilyVersion = Windows::System::Profile::AnalyticsInfo::VersionInfo->DeviceFamilyVersion;
            std::istringstream stringStream(utilities::GAUtilities::ws2s(deviceFamilyVersion->Data()));
            unsigned long long version;

            stringStream >> version;
            unsigned long long major = (version & 0xFFFF000000000000L) >> 48;
            unsigned long long minor = (version & 0x0000FFFF00000000L) >> 32;
            unsigned long long build = (version & 0x00000000FFFF0000L) >> 16;
            std::ostringstream stream;
            stream << getBuildPlatform() << " " << major << "." << minor << "." << build;
            return stream.str();
#elif USE_TIZEN
            std::string version = "0.0.0";
            char *value;
            int ret;
            ret = system_info_get_platform_string("http://tizen.org/feature/platform.version", &value);
            if (ret == SYSTEM_INFO_ERROR_NONE)
            {
                version = value;
            }

            return GADevice::getBuildPlatform() + " " + version;
#else
#ifdef _WIN32
            OSVERSIONINFOEX info;
            ZeroMemory(&info, sizeof(OSVERSIONINFOEX));
            info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
            GetVersionEx(&info);

            return GADevice::getBuildPlatform() + " " + std::to_string(info.dwMajorVersion) + "." + std::to_string(info.dwMinorVersion) + "." + std::to_string(info.dwBuildNumber);
#else
            return GADevice::getBuildPlatform() + " 0.0.0";
#endif
#endif
        }

        const std::string GADevice::deviceManufacturer()
        {
#if USE_UWP
            auto info = ref new Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
            return utilities::GAUtilities::ws2s(info->SystemManufacturer->Data());
#elif USE_TIZEN
            std::string result = "unknown";
            char *value;
            int ret;
            ret = system_info_get_platform_string("http://tizen.org/system/manufacturer", &value);
            if (ret == SYSTEM_INFO_ERROR_NONE)
            {
                result = value;
            }

            return result;
#else
            return "unknown";
#endif
        }

        const std::string GADevice::deviceModel()
        {
#if USE_UWP
            auto info = ref new Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
            return utilities::GAUtilities::ws2s(info->SystemProductName->Data());
#elif USE_TIZEN
            std::string result = "unknown";
            char *value;
            int ret;
            ret = system_info_get_platform_string("http://tizen.org/system/model_name", &value);
            if (ret == SYSTEM_INFO_ERROR_NONE)
            {
                result = value;
            }

            return result;
#else
            return "unknown";
#endif
        }

#if USE_UWP
        const std::string GADevice::getDeviceId()
        {
            return GADevice::_deviceId;
        }

        const std::string GADevice::getAdvertisingId()
        {
            return GADevice::_advertisingId;
        }

        const std::string GADevice::deviceId()
        {
            std::string result = "";

            if (Windows::Foundation::Metadata::ApiInformation::IsTypePresent("Windows.System.Profile.HardwareIdentification"))
            {
                auto token = Windows::System::Profile::HardwareIdentification::GetPackageSpecificToken(nullptr);
                auto hardwareId = token->Id;
                auto hardwareIdString = Windows::Security::Cryptography::CryptographicBuffer::EncodeToHexString(hardwareId);
                result = utilities::GAUtilities::ws2s(hardwareIdString->Data());
            }

            return result;
        }
#elif USE_TIZEN
        const std::string GADevice::getDeviceId()
        {
            return GADevice::_deviceId;
        }

        const std::string GADevice::deviceId()
        {
            std::string result = "";
            char *value;
            int ret;
            ret = system_info_get_platform_string("http://tizen.org/system/tizenid", &value);
            if (ret == SYSTEM_INFO_ERROR_NONE)
            {
                result = value;
            }

            return result;
        }
#endif

        const std::string GADevice::runtimePlatformToString()
        {
#if USE_UWP
            auto deviceFamily = Windows::System::Profile::AnalyticsInfo::VersionInfo->DeviceFamily;

            if (deviceFamily == "Windows.Mobile")
            {
                return "uwp_mobile";
            }
            else if (deviceFamily == "Windows.Desktop")
            {
                return "uwp_desktop";
            }
            else if (deviceFamily == "Windows.Universal")
            {
                return "uwp_iot";
            }
            else if (deviceFamily == "Windows.Xbox")
            {
                return "uwp_console";
            }
            else if (deviceFamily == "Windows.Team")
            {
                return "uwp_surfacehub";
            }
            else
            {
                return utilities::GAUtilities::ws2s(deviceFamily->Data());
            }
#elif USE_TIZEN
            std::string result = "tizen";
            char *value;
            int ret;
            ret = system_info_get_platform_string("http://tizen.org/system/platform.name", &value);
            if (ret == SYSTEM_INFO_ERROR_NONE)
            {
                result = value;
            }

            std::transform(result.begin(), result.end(), result.begin(), ::tolower);

            return result;
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

        const std::string GADevice::getPersistentPath()
        {
#if USE_UWP
            return utilities::GAUtilities::ws2s(Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data()) + "\\GameAnalytics";
#elif USE_TIZEN
            return app_get_data_path();
#else
#ifdef _WIN32
            std::string result = std::getenv("LOCALAPPDATA") + utilities::GAUtilities::getPathSeparator() + "GameAnalytics";
            _mkdir(result.c_str());
            return result;
#else
            std::string result = std::getenv("HOME") + utilities::GAUtilities::getPathSeparator() + "GameAnalytics";
            mode_t nMode = 0733;
            mkdir(result.c_str(),nMode);
            return result;
#endif
#endif
        }
    }
}
