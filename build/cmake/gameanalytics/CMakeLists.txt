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

if("${PLATFORM}" STREQUAL "win32-vc140-static" OR "${PLATFORM}" STREQUAL "win32-vc120-static")
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" ) # Multiprocessor compilation
    add_definitions("-D_CRT_SECURE_NO_WARNINGS") #  warning C4996
    set(LIB_EXTENSION "lib")
    SET (LIB_TYPE STATIC)
elseif("${PLATFORM}" STREQUAL "win64-vc140-static" OR "${PLATFORM}" STREQUAL "win64-vc120-static")
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" ) # Multiprocessor compilation
    add_definitions("-D_CRT_SECURE_NO_WARNINGS") #  warning C4996
    set(LIB_EXTENSION "lib")
    SET (LIB_TYPE STATIC)
elseif("${PLATFORM}" STREQUAL "win32-vc140-shared" OR "${PLATFORM}" STREQUAL "win32-vc120-shared")
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" ) # Multiprocessor compilation
    include(GenerateExportHeader)
    add_definitions("-D_CRT_SECURE_NO_WARNINGS -DELPP_NO_DEFAULT_LOG_FILE") #  warning C4996
    SET (LIB_TYPE SHARED)
    add_definitions("-DGA_SHARED_LIB")
elseif("${PLATFORM}" STREQUAL "win64-vc140-shared" OR "${PLATFORM}" STREQUAL "win64-vc120-shared")
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" ) # Multiprocessor compilation
    include(GenerateExportHeader)
    add_definitions("-D_CRT_SECURE_NO_WARNINGS -DELPP_NO_DEFAULT_LOG_FILE") #  warning C4996
    SET (LIB_TYPE SHARED)
    add_definitions("-DGA_SHARED_LIB")
elseif("${PLATFORM}" STREQUAL "osx-static")
    set(LIB_EXTENSION "a")
    set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
    set(CMAKE_OSX_ARCHITECTURES "x86_64") # for now, we only support 64bit
    set(EXTRA_LIBS z)
    SET (LIB_TYPE STATIC)
elseif("${PLATFORM}" STREQUAL "osx-shared")
    set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
    set(CMAKE_OSX_ARCHITECTURES "x86_64") # for now, we only support 64bit
    add_definitions("-DELPP_NO_DEFAULT_LOG_FILE -DOSX")
    set(EXTRA_LIBS z)
    SET (LIB_TYPE SHARED)
endif("${PLATFORM}" STREQUAL "win32-vc140-static" OR "${PLATFORM}" STREQUAL "win32-vc120-static")

message(STATUS "********************** DEPENDENCIES_DIR is ${DEPENDENCIES_DIR}")
add_definitions("-DUSE_OPENSSL -DCURL_STATICLIB")

# traverse all the subdirectories of the matched directory
FILE(GLOB_RECURSE CPP_SOURCES
    # Add GameAnalytics Sources
    "${GA_SOURCE_DIR}/*.cpp"
    "${GA_SOURCE_DIR}/*.h"

    # Add dependencies
    "${DEPENDENCIES_DIR}/jsoncpp/*.cpp"
    "${DEPENDENCIES_DIR}/sqlite/*.c"
    "${DEPENDENCIES_DIR}/sqlite/*.h"
    "${DEPENDENCIES_DIR}/crypto/*.c"
    "${DEPENDENCIES_DIR}/crypto/*.cpp"
	"${DEPENDENCIES_DIR}/plog/*.h"
	"${DEPENDENCIES_DIR}/curlcpp/include/*.h"
	"${DEPENDENCIES_DIR}/curlcpp/src/*.cpp"
)

add_definitions("-DCRYPTOPP_DISABLE_ASM")

include_directories(
    "${DEPENDENCIES_DIR}/jsoncpp"
	"${DEPENDENCIES_DIR}/plog"
    "${DEPENDENCIES_DIR}/sqlite"
    "${DEPENDENCIES_DIR}/crypto"
    "${DEPENDENCIES_DIR}/curl/include"
	"${DEPENDENCIES_DIR}/curlcpp/include"
	"${DEPENDENCIES_DIR}/boost/boost_1_60_0"
)

create_source_groups( CPP_SOURCES )
ADD_LIBRARY (GameAnalytics ${LIB_TYPE} ${CPP_SOURCES})
#target_compile_features(GameAnalytics PRIVATE cxx_inline_namespaces)

if("${PLATFORM}" STREQUAL "osx-static")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include_osx"
    )
endif("${PLATFORM}" STREQUAL "osx-static")

if("${PLATFORM}" STREQUAL "osx-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include_osx"
    )

    target_link_libraries(GameAnalytics
		${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/osx/libcrypto.a
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/osx/libssl.a
        ${DEPENDENCIES_DIR}/boost/boost_1_60_0/stage/osx/libboost_filesystem-mt.a
        ${DEPENDENCIES_DIR}/boost/boost_1_60_0/stage/osx/libboost_system-mt.a
        ${DEPENDENCIES_DIR}/curl/lib/osx/libcurl.a
    )
endif("${PLATFORM}" STREQUAL "osx-shared")

if("${PLATFORM}" STREQUAL "win32-vc140-static")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )
endif("${PLATFORM}" STREQUAL "win32-vc140-static")

if("${PLATFORM}" STREQUAL "win32-vc140-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )

    target_link_libraries(GameAnalytics
		${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win32/vc140_x86_release/libeay32.lib
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win32/vc140_x86_release/ssleay32.lib
        ${DEPENDENCIES_DIR}/boost/boost_1_60_0/stage/win32/lib/libboost_filesystem-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/boost_1_60_0/stage/win32/lib/libboost_system-vc140-mt-1_60.lib
		${DEPENDENCIES_DIR}/curl/lib/win32/vc140_x86_release/libcurl.lib
    )
endif("${PLATFORM}" STREQUAL "win32-vc140-shared")

if("${PLATFORM}" STREQUAL "win32-vc120-static")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )
endif("${PLATFORM}" STREQUAL "win32-vc120-static")

if("${PLATFORM}" STREQUAL "win32-vc120-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )
    
    target_link_libraries(GameAnalytics
		${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win32/vc120_x86_release/libeay32.lib
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win32/vc120_x86_release/ssleay32.lib
        ${DEPENDENCIES_DIR}/boost/boost_1_60_0/stage/win32/lib/libboost_filesystem-vc120-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/boost_1_60_0/stage/win32/lib/libboost_system-vc120-mt-1_60.lib
		${DEPENDENCIES_DIR}/curl/lib/win32/vc120_x86_release/libcurl.lib
    )
endif("${PLATFORM}" STREQUAL "win32-vc120-shared")

if("${PLATFORM}" STREQUAL "win64-vc140-static")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )
endif("${PLATFORM}" STREQUAL "win64-vc140-static")

if("${PLATFORM}" STREQUAL "win64-vc140-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )

    target_link_libraries(GameAnalytics
		${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win64/vc140_x64_release/libeay32.lib
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win64/vc140_x64_release/ssleay32.lib
        ${DEPENDENCIES_DIR}/boost/boost_1_60_0/stage/win64/lib/libboost_filesystem-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/boost_1_60_0/stage/win64/lib/libboost_system-vc140-mt-1_60.lib
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
        ${DEPENDENCIES_DIR}/boost/boost_1_60_0/stage/win64/lib/libboost_filesystem-vc120-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/boost_1_60_0/stage/win64/lib/libboost_system-vc120-mt-1_60.lib
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
