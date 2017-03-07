CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

PROJECT (GameAnalytics)

# useful Cmake macro for recursive search of files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../jenkins/CMakeIncludes")
include("create_source_groups_macro")

message(STATUS ${CMAKE_MODULE_PATH})

set(GA_SOURCE_DIR "${PROJECT_SOURCE_DIR}/../../../source/gameanalytics")
set(DEPENDENCIES_DIR "${PROJECT_SOURCE_DIR}/../../../source/dependencies")

message(STATUS "********************** platform is ${PLATFORM}")

if("${PLATFORM}" STREQUAL "win32-vc140-static" OR "${PLATFORM}" STREQUAL "win32-vc120-static" OR "${PLATFORM}" STREQUAL "win64-vc140-static" OR "${PLATFORM}" STREQUAL "win64-vc120-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-static")
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" ) # Multiprocessor compilation
    add_definitions("-D_CRT_SECURE_NO_WARNINGS") #  warning C4996
    set(LIB_EXTENSION "lib")
    SET (LIB_TYPE STATIC)
elseif("${PLATFORM}" STREQUAL "win32-vc140-shared" OR "${PLATFORM}" STREQUAL "win32-vc120-shared" OR "${PLATFORM}" STREQUAL "win64-vc140-shared" OR "${PLATFORM}" STREQUAL "win64-vc120-shared" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-shared")
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" ) # Multiprocessor compilation
    include(GenerateExportHeader)
    add_definitions("-D_CRT_SECURE_NO_WARNINGS -DELPP_NO_DEFAULT_LOG_FILE") #  warning C4996
    SET (LIB_TYPE SHARED)
    add_definitions("-DGA_SHARED_LIB")
elseif("${PLATFORM}" STREQUAL "osx-static")
    set(LIB_EXTENSION "a")
    set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64") # for now, we only support 64bit
    set(EXTRA_LIBS z)
    SET (LIB_TYPE STATIC)
elseif("${PLATFORM}" STREQUAL "osx-shared")
    set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64") # for now, we only support 64bit
    add_definitions("-DELPP_NO_DEFAULT_LOG_FILE -DOSX")
    set(EXTRA_LIBS z)
    SET (LIB_TYPE SHARED)
    add_definitions("-DGA_SHARED_LIB")
elseif("${PLATFORM}" STREQUAL "linux-static")
    set(LIB_EXTENSION "a")
    set(CMAKE_CXX_FLAGS "-std=c++11")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release")
    SET (LIB_TYPE STATIC)
elseif("${PLATFORM}" STREQUAL "linux-shared")
    set(LIB_EXTENSION "so")
    set(CMAKE_CXX_FLAGS "-std=c++11")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release")
    SET (LIB_TYPE SHARED)
endif("${PLATFORM}" STREQUAL "win32-vc140-static" OR "${PLATFORM}" STREQUAL "win32-vc120-static" OR "${PLATFORM}" STREQUAL "win64-vc140-static" OR "${PLATFORM}" STREQUAL "win64-vc120-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-static")

if("${PLATFORM}" STREQUAL "uwp-x86-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-shared")
    add_definitions("-DUSE_UWP -ZW")
endif("${PLATFORM}" STREQUAL "uwp-x86-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-shared")

if("${PLATFORM}" STREQUAL "win32-vc140-static" OR "${PLATFORM}" STREQUAL "win32-vc120-static" OR "${PLATFORM}" STREQUAL "win64-vc140-static" OR "${PLATFORM}" STREQUAL "win64-vc120-static" OR "${PLATFORM}" STREQUAL "win32-vc140-shared" OR "${PLATFORM}" STREQUAL "win32-vc120-shared" OR "${PLATFORM}" STREQUAL "win64-vc140-shared" OR "${PLATFORM}" STREQUAL "win64-vc120-shared")
    add_definitions("-DGUID_WINDOWS")
endif("${PLATFORM}" STREQUAL "win32-vc140-static" OR "${PLATFORM}" STREQUAL "win32-vc120-static" OR "${PLATFORM}" STREQUAL "win64-vc140-static" OR "${PLATFORM}" STREQUAL "win64-vc120-static" OR "${PLATFORM}" STREQUAL "win32-vc140-shared" OR "${PLATFORM}" STREQUAL "win32-vc120-shared" OR "${PLATFORM}" STREQUAL "win64-vc140-shared" OR "${PLATFORM}" STREQUAL "win64-vc120-shared")

if("${PLATFORM}" STREQUAL "osx-static" OR "${PLATFORM}" STREQUAL "osx-shared")
    add_definitions("-DGUID_CFUUID")
endif("${PLATFORM}" STREQUAL "osx-static" OR "${PLATFORM}" STREQUAL "osx-shared")

if("${PLATFORM}" STREQUAL "linux-static" OR "${PLATFORM}" STREQUAL "linux-shared")
    add_definitions("-DGUID_LIBUUID")
endif("${PLATFORM}" STREQUAL "linux-static" OR "${PLATFORM}" STREQUAL "linux-shared")

message(STATUS "********************** DEPENDENCIES_DIR is ${DEPENDENCIES_DIR}")
add_definitions("-DUSE_OPENSSL -DCURL_STATICLIB")

if("${PLATFORM}" STREQUAL "uwp-x86-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-shared")
    add_definitions("-DSQLITE_OS_WINRT")

    FILE(GLOB_RECURSE CPP_SOURCES
        # Add GameAnalytics Sources
        "${GA_SOURCE_DIR}/*.cpp"
        "${GA_SOURCE_DIR}/*.h"

        # Add dependencies
        "${DEPENDENCIES_DIR}/crossguid/*.cpp"
        "${DEPENDENCIES_DIR}/jsoncpp/*.cpp"
        "${DEPENDENCIES_DIR}/sqlite/*.c"
        "${DEPENDENCIES_DIR}/sqlite/*.h"
        "${DEPENDENCIES_DIR}/miniz/*.c"
    )

    include_directories(
        "${DEPENDENCIES_DIR}/crossguid"
        "${DEPENDENCIES_DIR}/jsoncpp"
        "${DEPENDENCIES_DIR}/sqlite"
        "${DEPENDENCIES_DIR}/miniz"
    )
elseif("${PLATFORM}" STREQUAL "osx-static" OR "${PLATFORM}" STREQUAL "osx-shared")
    # traverse all the subdirectories of the matched directory
    FILE(GLOB_RECURSE CPP_SOURCES
        # Add GameAnalytics Sources
        "${GA_SOURCE_DIR}/*.cpp"
        "${GA_SOURCE_DIR}/*.mm"
        "${GA_SOURCE_DIR}/*.h"

        # Add dependencies
        "${DEPENDENCIES_DIR}/crossguid/*.cpp"
        "${DEPENDENCIES_DIR}/jsoncpp/*.cpp"
        "${DEPENDENCIES_DIR}/sqlite/*.c"
        "${DEPENDENCIES_DIR}/sqlite/*.h"
        "${DEPENDENCIES_DIR}/crypto/*.c"
        "${DEPENDENCIES_DIR}/crypto/*.cpp"
        "${DEPENDENCIES_DIR}/plog/*.h"
        "${DEPENDENCIES_DIR}/miniz/*.c"
        "${DEPENDENCIES_DIR}/curlcpp/include/*.h"
        "${DEPENDENCIES_DIR}/curlcpp/src/*.cpp"
    )

    add_definitions("-DCRYPTOPP_DISABLE_ASM")

    include_directories(
        "${DEPENDENCIES_DIR}/crossguid"
        "${DEPENDENCIES_DIR}/jsoncpp"
        "${DEPENDENCIES_DIR}/plog"
        "${DEPENDENCIES_DIR}/sqlite"
        "${DEPENDENCIES_DIR}/crypto"
        "${DEPENDENCIES_DIR}/miniz"
        "${DEPENDENCIES_DIR}/curl/include"
        "${DEPENDENCIES_DIR}/curlcpp/include"
    )
else()
    # traverse all the subdirectories of the matched directory
    FILE(GLOB_RECURSE CPP_SOURCES
        # Add GameAnalytics Sources
        "${GA_SOURCE_DIR}/*.cpp"
        "${GA_SOURCE_DIR}/*.h"

        # Add dependencies
        "${DEPENDENCIES_DIR}/crossguid/*.cpp"
        "${DEPENDENCIES_DIR}/jsoncpp/*.cpp"
        "${DEPENDENCIES_DIR}/sqlite/*.c"
        "${DEPENDENCIES_DIR}/sqlite/*.h"
        "${DEPENDENCIES_DIR}/crypto/*.c"
        "${DEPENDENCIES_DIR}/crypto/*.cpp"
        "${DEPENDENCIES_DIR}/plog/*.h"
        "${DEPENDENCIES_DIR}/miniz/*.c"
        "${DEPENDENCIES_DIR}/curlcpp/include/*.h"
        "${DEPENDENCIES_DIR}/curlcpp/src/*.cpp"
    )

    add_definitions("-DCRYPTOPP_DISABLE_ASM")

    include_directories(
        "${DEPENDENCIES_DIR}/crossguid"
        "${DEPENDENCIES_DIR}/jsoncpp"
        "${DEPENDENCIES_DIR}/plog"
        "${DEPENDENCIES_DIR}/sqlite"
        "${DEPENDENCIES_DIR}/crypto"
        "${DEPENDENCIES_DIR}/miniz"
        "${DEPENDENCIES_DIR}/curl/include"
        "${DEPENDENCIES_DIR}/curlcpp/include"
    )
endif("${PLATFORM}" STREQUAL "uwp-x86-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-shared")

create_source_groups( CPP_SOURCES )
ADD_LIBRARY (GameAnalytics ${LIB_TYPE} ${CPP_SOURCES})
#target_compile_features(GameAnalytics PRIVATE cxx_inline_namespaces)

if("${PLATFORM}" STREQUAL "osx-static")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include_osx"
    )
endif("${PLATFORM}" STREQUAL "osx-static")

if("${PLATFORM}" STREQUAL "linux-static" OR "${PLATFORM}" STREQUAL "linux-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/ooid"
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include_osx"
    )
endif("${PLATFORM}" STREQUAL "linux-static" OR "${PLATFORM}" STREQUAL "linux-shared")

if("${PLATFORM}" STREQUAL "osx-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include_osx"
    )

    target_link_libraries(GameAnalytics
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/osx/libcrypto.a
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/osx/libssl.a
        ${DEPENDENCIES_DIR}/curl/lib/osx/libcurl.a
        "-framework CoreFoundation"
        "-framework Foundation"
        "-framework CoreServices"
    )
endif("${PLATFORM}" STREQUAL "osx-shared")

if("${PLATFORM}" STREQUAL "linux-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include_osx"
    )

    target_link_libraries(GameAnalytics
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/linux/libcrypto.a
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/linux/libssl.a
        ${DEPENDENCIES_DIR}/curl/lib/linux/libcurl.a
    )
endif("${PLATFORM}" STREQUAL "linux-shared")

if("${PLATFORM}" STREQUAL "win32-vc140-static" OR "${PLATFORM}" STREQUAL "win32-vc120-static")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )
endif("${PLATFORM}" STREQUAL "win32-vc140-static" OR "${PLATFORM}" STREQUAL "win32-vc120-static")

if("${PLATFORM}" STREQUAL "win32-vc140-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )

    target_link_libraries(GameAnalytics
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win32/vc140_x86_release/libeay32.lib
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win32/vc140_x86_release/ssleay32.lib
        ${DEPENDENCIES_DIR}/curl/lib/win32/vc140_x86_release/libcurl.lib
    )
endif("${PLATFORM}" STREQUAL "win32-vc140-shared")

if("${PLATFORM}" STREQUAL "win32-vc120-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )

    target_link_libraries(GameAnalytics
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win32/vc120_x86_release/libeay32.lib
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win32/vc120_x86_release/ssleay32.lib
        ${DEPENDENCIES_DIR}/curl/lib/win32/vc120_x86_release/libcurl.lib
    )
endif("${PLATFORM}" STREQUAL "win32-vc120-shared")

if("${PLATFORM}" STREQUAL "win64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )
endif("${PLATFORM}" STREQUAL "win64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-shared")

if("${PLATFORM}" STREQUAL "uwp-x86-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-shared")
    set_target_properties(GameAnalytics PROPERTIES VS_DESKTOP_EXTENSIONS_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
endif("${PLATFORM}" STREQUAL "uwp-x86-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-static" OR "${PLATFORM}" STREQUAL "uwp-x86-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-x64-vc140-shared" OR "${PLATFORM}" STREQUAL "uwp-arm-vc140-shared")

if("${PLATFORM}" STREQUAL "win64-vc140-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )

    target_link_libraries(GameAnalytics
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win64/vc140_x64_release/libeay32.lib
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win64/vc140_x64_release/ssleay32.lib
        ${DEPENDENCIES_DIR}/curl/lib/win64/vc140_x64_release/libcurl.lib
    )
endif("${PLATFORM}" STREQUAL "win64-vc140-shared")

if("${PLATFORM}" STREQUAL "win64-vc120-static")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )
endif("${PLATFORM}" STREQUAL "win64-vc120-static")

if("${PLATFORM}" STREQUAL "win64-vc120-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )

    target_link_libraries(GameAnalytics
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win64/vc120_x64_release/libeay32.lib
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win64/vc120_x64_release/ssleay32.lib
        ${DEPENDENCIES_DIR}/curl/lib/win64/vc120_x64_release/libcurl.lib
    )
endif("${PLATFORM}" STREQUAL "win64-vc120-shared")

target_include_directories(
    GameAnalytics
    PUBLIC
    "${GA_SOURCE_DIR}"
)

# enable c++11 via -std=c++11 when compiler is clang or gcc
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_features(GameAnalytics PRIVATE cxx_nonstatic_member_init)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_features(GameAnalytics PRIVATE cxx_nonstatic_member_init)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_features(GameAnalytics PRIVATE cxx_nonstatic_member_init)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()
