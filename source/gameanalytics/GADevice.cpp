//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#if defined(__linux__) || defined(__unix__) || defined(__unix) || defined(unix)
#define IS_LINUX 1
#else
#define IS_LINUX 0
#endif

#if defined(__MACH__) || defined(__APPLE__)
#define IS_MAC 1
#else
#define IS_MAC 0
#endif

#if _WIN32
#define _WIN32_DCOM
#endif

#include "GADevice.h"
#include "GAUtilities.h"
#include <string.h>
#include <stdio.h>
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
#include <VersionHelpers.h>
#if !GA_SHARED_LIB
#include <comdef.h>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#endif
#else
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#if IS_MAC
#include "GADeviceOSX.h"
#include <sys/sysctl.h>
#elif IS_LINUX
#include <sys/utsname.h>
#include <cctype>
#endif

namespace gameanalytics
{
    namespace device
    {
        char GADevice::_writablepath[257] = "";
        char GADevice::_buildPlatform[32] = "";
        char GADevice::_osVersion[65] = "";
        char GADevice::_deviceModel[129] = "";
#if USE_UWP
        const std::string GADevice::_advertisingId = utilities::GAUtilities::ws2s(Windows::System::UserProfile::AdvertisingManager::AdvertisingId->Data());
        const std::string GADevice::_deviceId = GADevice::deviceId();
#elif USE_TIZEN
        const std::string GADevice::_deviceId = GADevice::deviceId();
#endif
        char GADevice::_deviceManufacturer[129] = "";
        char GADevice::_sdkGameEngineVersion[33] = "";
        char GADevice::_gameEngineVersion[33] = "";
        char GADevice::_connectionType[33] = "";
#if USE_UWP
        const char* GADevice::_sdkWrapperVersion = "uwp_cpp 2.1.2";
#elif USE_TIZEN
        const char* GADevice::_sdkWrapperVersion = "tizen 2.1.2";
#else
        const char* GADevice::_sdkWrapperVersion = "cpp 2.1.2";
#endif

        void GADevice::setSdkGameEngineVersion(const char* sdkGameEngineVersion)
        {
            snprintf(GADevice::_sdkGameEngineVersion, sizeof(GADevice::_sdkGameEngineVersion), "%s", sdkGameEngineVersion);
        }

        const char* GADevice::getGameEngineVersion()
        {
            return GADevice::_gameEngineVersion;
        }

        void GADevice::setGameEngineVersion(const char* gameEngineVersion)
        {
            snprintf(GADevice::_gameEngineVersion, sizeof(GADevice::_gameEngineVersion), "%s", gameEngineVersion);
        }

        void GADevice::setConnectionType(const char* connectionType)
        {
            snprintf(GADevice::_connectionType, sizeof(GADevice::_connectionType), "%s", connectionType);
        }

        const char* GADevice::getConnectionType()
        {
            return GADevice::_connectionType;
        }

        const char* GADevice::getRelevantSdkVersion()
        {
            if(strlen(GADevice::_sdkGameEngineVersion) > 0)
            {
                return GADevice::_sdkGameEngineVersion;
            }

            return GADevice::_sdkWrapperVersion;
        }

        const char* GADevice::getBuildPlatform()
        {
            if(strlen(GADevice::_buildPlatform) == 0)
            {
                initRuntimePlatform();
            }
            return GADevice::_buildPlatform;
        }

        const char* GADevice::getOSVersion()
        {
            if(strlen(GADevice::_osVersion) == 0)
            {
                initOSVersion();
            }
            return GADevice::_osVersion;
        }

        void GADevice::setDeviceModel(const char* deviceModel)
        {
            if(strlen(GADevice::_deviceModel) == 0)
            {
                snprintf(GADevice::_deviceModel, sizeof(GADevice::_deviceModel), "%s", "unknown");
            }
            else
            {
                snprintf(GADevice::_deviceModel, sizeof(GADevice::_deviceModel), "%s", deviceModel);
            }
        }

        const char* GADevice::getDeviceModel()
        {
            if(strlen(GADevice::_deviceModel) == 0)
            {
                initDeviceModel();
            }
            return GADevice::_deviceModel;
        }

        void GADevice::setDeviceManufacturer(const char* deviceManufacturer)
        {
            if(strlen(GADevice::_deviceModel) == 0)
            {
                snprintf(GADevice::_deviceManufacturer, sizeof(GADevice::_deviceManufacturer), "%s", "unknown");
            }
            else
            {
                snprintf(GADevice::_deviceManufacturer, sizeof(GADevice::_deviceManufacturer), "%s", deviceManufacturer);
            }
        }

        const char* GADevice::getDeviceManufacturer()
        {
            if(strlen(GADevice::_deviceModel) == 0)
            {
                initDeviceManufacturer();
            }
            return GADevice::_deviceManufacturer;
        }

        void GADevice::setWritablePath(const char* writablepath)
        {
            snprintf(GADevice::_writablepath, sizeof(GADevice::_writablepath), "%s", writablepath);
        }

        const char* GADevice::getWritablePath()
        {
            if(strlen(GADevice::_writablepath) == 0)
            {
                initPersistentPath();
            }
            return GADevice::_writablepath;
        }

        void GADevice::UpdateConnectionType()
        {
            snprintf(GADevice::_connectionType, sizeof(GADevice::_connectionType), "%s", "lan");
        }

        void GADevice::initOSVersion()
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
#if (_MSC_VER == 1900)
            if (IsWindows10OrGreater())
            {
                return GADevice::getBuildPlatform() + " 10.0";
            }
            else
#endif
                if (IsWindows8Point1OrGreater())
            {
                return GADevice::getBuildPlatform() + " 6.3";
            }
            else if (IsWindows8OrGreater())
            {
                return GADevice::getBuildPlatform() + " 6.2";
            }
            else if (IsWindows7OrGreater())
            {
                return GADevice::getBuildPlatform() + " 6.1";
            }
            else if (IsWindowsVistaOrGreater())
            {
                return GADevice::getBuildPlatform() + " 6.0";
            }
            else if (IsWindowsXPOrGreater())
            {
                return GADevice::getBuildPlatform() + " 5.1";
            }
            else
            {
                return GADevice::getBuildPlatform() + " 0.0.0";
            }
#elif IS_MAC
            snprintf(GADevice::_osVersion, sizeof(GADevice::_osVersion), "%s %s", GADevice::getBuildPlatform(), getOSXVersion());
#elif IS_LINUX
            struct utsname info;
            uname(&info);
            std::string v(info.release);

            std::size_t i;
            for(i = 0; i < v.size(); ++i)
            {
                if(!isdigit(v[i]) && v[i] != '.')
                {
                    v = v.substr(0, i);
                    break;
                }
            }
            return GADevice::getBuildPlatform() + " " + v;
#else
            return GADevice::getBuildPlatform() + " 0.0.0";
#endif
#endif
        }

        void GADevice::initDeviceManufacturer()
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
#if defined(_WIN32) && !GA_SHARED_LIB
            IWbemLocator *locator = nullptr;
            IWbemServices *services = nullptr;
            auto hResult = CoInitializeEx(0, COINIT_MULTITHREADED);
            if (FAILED(hResult))
            {
                return "unknown";
            }
            hResult = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&locator);

            auto hasFailed = [&hResult]() {
                if (FAILED(hResult))
                {
                    return true;
                }
                return false;
            };

            auto getValue = [&hResult, &hasFailed](IWbemClassObject *classObject, LPCWSTR property) {
                BSTR propertyValueText = L"unknown";
                VARIANT propertyValue;
                hResult = classObject->Get(property, 0, &propertyValue, 0, 0);
                if (!hasFailed()) {
                    if ((propertyValue.vt == VT_NULL) || (propertyValue.vt == VT_EMPTY)) {
                    }
                    else if (propertyValue.vt & VT_ARRAY) {
                        propertyValueText = L"unknown"; //Array types not supported
                    }
                    else {
                        propertyValueText = propertyValue.bstrVal;
                    }
                }
                VariantClear(&propertyValue);
                return propertyValueText;
            };

            BSTR manufacturer = L"unknown";
            if (!hasFailed()) {
                // Connect to the root\cimv2 namespace with the current user and obtain pointer pSvc to make IWbemServices calls.
                hResult = locator->ConnectServer(L"ROOT\\CIMV2", nullptr, nullptr, 0, NULL, 0, 0, &services);

                if (!hasFailed()) {
                    // Set the IWbemServices proxy so that impersonation of the user (client) occurs.
                    hResult = CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL,
                        RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

                    if (!hasFailed()) {
                        IEnumWbemClassObject* classObjectEnumerator = nullptr;
                        hResult = services->ExecQuery(L"WQL", L"SELECT * FROM Win32_ComputerSystem", WBEM_FLAG_FORWARD_ONLY |
                            WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &classObjectEnumerator);
                        if (!hasFailed()) {
                            IWbemClassObject *classObject;
                            ULONG uReturn = 0;
                            hResult = classObjectEnumerator->Next(WBEM_INFINITE, 1, &classObject, &uReturn);
                            if (uReturn != 0) {
                                manufacturer = getValue(classObject, (LPCWSTR)L"Manufacturer");
                            }
                            classObject->Release();
                        }
                        classObjectEnumerator->Release();
                    }
                }
            }

            if (locator) {
                locator->Release();
            }
            if (services) {
                services->Release();
            }
            CoUninitialize();

            return std::string(_com_util::ConvertBSTRToString(manufacturer));
#elif IS_MAC
            snprintf(GADevice::_deviceManufacturer, sizeof(GADevice::_deviceManufacturer), "%s", "Apple");
#elif IS_LINUX
            return "unknown";
#else
            return "unknown";
#endif
#endif
        }

        void GADevice::initDeviceModel()
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
#if defined(_WIN32) && !GA_SHARED_LIB
            IWbemLocator *locator = nullptr;
            IWbemServices *services = nullptr;
            auto hResult = CoInitializeEx(0, COINIT_MULTITHREADED);
            if (FAILED(hResult))
            {
                return "unknown";
            }
            hResult = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&locator);

            auto hasFailed = [&hResult]() {
                if (FAILED(hResult))
                {
                    return true;
                }
                return false;
            };

            auto getValue = [&hResult, &hasFailed](IWbemClassObject *classObject, LPCWSTR property) {
                BSTR propertyValueText = L"unknown";
                VARIANT propertyValue;
                hResult = classObject->Get(property, 0, &propertyValue, 0, 0);
                if (!hasFailed()) {
                    if ((propertyValue.vt == VT_NULL) || (propertyValue.vt == VT_EMPTY)) {
                    }
                    else if (propertyValue.vt & VT_ARRAY) {
                        propertyValueText = L"unknown"; //Array types not supported
                    }
                    else {
                        propertyValueText = propertyValue.bstrVal;
                    }
                }
                VariantClear(&propertyValue);
                return propertyValueText;
            };

            BSTR model = L"unknown";
            if (!hasFailed()) {
                // Connect to the root\cimv2 namespace with the current user and obtain pointer pSvc to make IWbemServices calls.
                hResult = locator->ConnectServer(L"ROOT\\CIMV2", nullptr, nullptr, 0, NULL, 0, 0, &services);

                if (!hasFailed()) {
                    // Set the IWbemServices proxy so that impersonation of the user (client) occurs.
                    hResult = CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL,
                        RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

                    if (!hasFailed()) {
                        IEnumWbemClassObject* classObjectEnumerator = nullptr;
                        hResult = services->ExecQuery(L"WQL", L"SELECT * FROM Win32_ComputerSystem", WBEM_FLAG_FORWARD_ONLY |
                            WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &classObjectEnumerator);
                        if (!hasFailed()) {
                            IWbemClassObject *classObject;
                            ULONG uReturn = 0;
                            hResult = classObjectEnumerator->Next(WBEM_INFINITE, 1, &classObject, &uReturn);
                            if (uReturn != 0) {
                                model = getValue(classObject, (LPCWSTR)L"Model");
                            }
                            classObject->Release();
                        }
                        classObjectEnumerator->Release();
                    }
                }
            }

            if (locator) {
                locator->Release();
            }
            if (services) {
                services->Release();
            }
            CoUninitialize();

            return std::string(_com_util::ConvertBSTRToString(model));
#elif IS_MAC
            size_t len = 0;
            sysctlbyname("hw.model", NULL, &len, NULL, 0);

            char* model = (char*)malloc(len + 1);
            memset(model, 0, len + 1);

            sysctlbyname("hw.model", model, &len, NULL, 0);

            snprintf(GADevice::_deviceModel, sizeof(GADevice::_deviceModel), "%s", model);
            free(model);
#elif IS_LINUX
            return "unknown";
#else
            return "unknown";
#endif
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

        void GADevice::initRuntimePlatform()
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
#if IS_MAC
            snprintf(GADevice::_buildPlatform, sizeof(GADevice::_buildPlatform), "%s", "mac_osx");
#elif defined(_WIN32)
            return "windows";
#elif IS_LINUX
            return "linux";
#else
            return "unknown";
#endif
#endif
        }

        void GADevice::initPersistentPath()
        {
#if USE_UWP
            std::string result = utilities::GAUtilities::ws2s(Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data()) + "\\GameAnalytics";
            return s.c_str();
#elif USE_TIZEN
            return app_get_data_path();
#else
#ifdef _WIN32

            std::string result = std::getenv("LOCALAPPDATA") + utilities::GAUtilities::getPathSeparator() + "GameAnalytics";
            _mkdir(result.c_str());
            return result;
#else
            snprintf(GADevice::_writablepath, sizeof(GADevice::_writablepath), "%s%sGameAnalytics", std::getenv("HOME"), utilities::GAUtilities::getPathSeparator().c_str());
            mode_t nMode = 0733;
            mkdir(GADevice::_writablepath, nMode);
#endif
#endif
        }
    }
}
