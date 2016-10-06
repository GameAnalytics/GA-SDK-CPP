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
elseif("${PLATFORM}" STREQUAL "win32-shared")
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" ) # Multiprocessor compilation
    include(GenerateExportHeader)
    add_definitions("-D_CRT_SECURE_NO_WARNINGS -DELPP_NO_DEFAULT_LOG_FILE") #  warning C4996
    SET (LIB_TYPE SHARED)
elseif("${PLATFORM}" STREQUAL "win64-shared")
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /MP" ) # Multiprocessor compilation
    include(GenerateExportHeader)
    add_definitions("-D_CRT_SECURE_NO_WARNINGS -DELPP_NO_DEFAULT_LOG_FILE") #  warning C4996
    SET (LIB_TYPE SHARED)
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
)

add_definitions("-DCRYPTOPP_DISABLE_ASM")

include_directories(
    "${DEPENDENCIES_DIR}/boost/boost_1_60_0"
	"${DEPENDENCIES_DIR}/cppnetlib/0.11.2/include"
    "${DEPENDENCIES_DIR}/jsoncpp"
    "${DEPENDENCIES_DIR}/sqlite"
    "${DEPENDENCIES_DIR}/crypto"
)

create_source_groups( CPP_SOURCES )
ADD_LIBRARY (GameAnalytics ${LIB_TYPE} ${CPP_SOURCES})
#target_compile_features(GameAnalytics PRIVATE cxx_inline_namespaces)

if("${PLATFORM}" STREQUAL "osx-static" OR "${PLATFORM}" STREQUAL "osx-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include_osx"
    )

    target_link_libraries(GameAnalytics 
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/osx/libboost_thread-mt.a
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/osx/libboost_filesystem-mt.a
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/osx/libboost_log-mt.a
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/osx/libboost_log_setup-mt.a
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/osx/libboost_regex-mt.a
        ${DEPENDENCIES_DIR}/cppnetlib/0.11.2/libs/osx/libcppnetlib-uri.a
        ${DEPENDENCIES_DIR}/cppnetlib/0.11.2/libs/osx/libcppnetlib-client-connections.a
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/osx/libboost_chrono-mt.a
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/osx/libboost_system-mt.a
		${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/osx/libcrypto.a
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/osx/libssl.a
    )
endif("${PLATFORM}" STREQUAL "osx-static" OR "${PLATFORM}" STREQUAL "osx-shared")

if("${PLATFORM}" STREQUAL "win32-vc140-static" OR "${PLATFORM}" STREQUAL "win32-vc120-static" OR "${PLATFORM}" STREQUAL "win32-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )

    target_link_libraries(GameAnalytics 
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win32/libboost_atomic-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win32/libboost_thread-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win32/libboost_filesystem-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win32/libboost_log-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win32/libboost_log_setup-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win32/libboost_regex-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/cppnetlib/0.11.2/libs/win32/cppnetlib-uri.lib
        ${DEPENDENCIES_DIR}/cppnetlib/0.11.2/libs/win32/cppnetlib-client-connections.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win32/libboost_chrono-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win32/libboost_system-vc140-mt-1_60.lib
		${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win32/vc140_x86_release/libeay32.lib
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win32/vc140_x86_release/ssleay32.lib
    )
endif("${PLATFORM}" STREQUAL "win32-vc140-static" OR "${PLATFORM}" STREQUAL "win32-vc120-static" OR "${PLATFORM}" STREQUAL "win32-shared")

if("${PLATFORM}" STREQUAL "win64-vc140-static" OR "${PLATFORM}" STREQUAL "win64-vc120-static" OR "${PLATFORM}" STREQUAL "win64-shared")
    include_directories(
        "${DEPENDENCIES_DIR}/openssl/1.0.2h/include"
    )

    target_link_libraries(GameAnalytics 
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win64/libboost_atomic-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win64/libboost_thread-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win64/libboost_filesystem-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win64/libboost_log-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win64/libboost_log_setup-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win64/libboost_regex-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/cppnetlib/0.11.2/libs/win64/cppnetlib-uri.lib
        ${DEPENDENCIES_DIR}/cppnetlib/0.11.2/libs/win64/cppnetlib-client-connections.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win64/libboost_chrono-vc140-mt-1_60.lib
        ${DEPENDENCIES_DIR}/boost/1.60.0/libs/win64/libboost_system-vc140-mt-1_60.lib
		${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win64/vc140_x64_release/libeay32.lib
        ${DEPENDENCIES_DIR}/openssl/1.0.2h/libs/win64/vc140_x64_release/ssleay32.lib
    )
endif("${PLATFORM}" STREQUAL "win64-vc140-static" OR "${PLATFORM}" STREQUAL "win64-vc120-static" OR "${PLATFORM}" STREQUAL "win64-shared")

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




