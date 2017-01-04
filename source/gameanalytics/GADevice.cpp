//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#include "GADevice.h"
#if USE_UWP
#include <Windows.h>
#include <sstream>
#include "GAUtilities.h"
#else
#include <boost/filesystem.hpp>
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
#endif
        std::string GADevice::_deviceManufacturer = GADevice::deviceManufacturer();
        std::string GADevice::_writablepath = GADevice::getPersistentPath();
        std::string GADevice::_sdkGameEngineVersion;
        std::string GADevice::_gameEngineVersion;
        std::string GADevice::_connectionType = "";
#if USE_UWP
        const std::string GADevice::_sdkWrapperVersion = "uwp_cpp 1.2.4";
#else
        const std::string GADevice::_sdkWrapperVersion = "cpp 1.2.4";
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
#else
            return GADevice::getBuildPlatform() + " 0.0.0";
#endif
        }

        const std::string GADevice::deviceManufacturer()
        {
#if USE_UWP
            auto info = ref new Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
            return utilities::GAUtilities::ws2s(info->SystemManufacturer->Data());
#else
            return "unknown";
#endif
        }

        const std::string GADevice::deviceModel()
        {
#if USE_UWP
            auto info = ref new Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
            return utilities::GAUtilities::ws2s(info->SystemProductName->Data());
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
#else
            return boost::filesystem::detail::temp_directory_path().string();
#endif
        }
    }
}
