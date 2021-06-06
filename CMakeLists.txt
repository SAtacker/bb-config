option(BUILD_ARM "Build for ARM " OFF)
message("Setting for BUILD_ARM ${BUILD_ARM}")

if(BUILD_ARM)
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)
    
    set(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabi-g++-8)
    set(CMAKE_C_COMPILER /usr/bin/arm-linux-gnueabi-gcc-8)
    
    set(CMAKE_FIND_ROOT_PATH  /usr/arm-linux-gnueabihf)

    # search for programs in the build host directories
    SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

    # for libraries and headers in the target directories
    SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif(BUILD_ARM)
unset(BUILD_ARM)

cmake_minimum_required(VERSION 3.0.0)
project(beagle-config VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

add_subdirectory(./board)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
