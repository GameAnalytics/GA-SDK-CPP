INCLUDE(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR ARM)

CMAKE_FORCE_C_COMPILER(arm-linux-gnueabi-gcc GNU 4.9+)
CMAKE_FORCE_CXX_COMPILER(arm-linux-gnueabi-g++ GNU 4.9+)

set(TIZEN 1)
set(STATIC 1)
