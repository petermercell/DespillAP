# Container
if(DEFINED ENV{GLOBAL_TOOLCHAIN})
    message("Set toolchain $ENV{GLOBAL_TOOLCHAIN}")
    set(CMAKE_TOOLCHAIN_FILE $ENV{GLOBAL_TOOLCHAIN})
endif()

# CMake
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(DespillAP)

cmake_policy(SET CMP0074 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include_directories(${CMAKE_SOURCE_DIR})

find_package(Nuke REQUIRED)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# cxx standards
if (NUKE_VERSION_MAJOR VERSION_GREATER_EQUAL 15)
    # Nuke 15.x: C++17 + new ABI
    message("Configuring for Nuke 15.x: C++17")
    # https://learn.foundry.com/nuke/developers/150/ndkdevguide/appendixa/linux.html
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=1")
    set(CMAKE_CXX_STANDARD 17)
elseif (NUKE_VERSION_MAJOR VERSION_GREATER_EQUAL 14)
    # Nuke 14.x: C++17 + old ABI
    message("Configuring for Nuke 14.x: C++17") 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
    set(CMAKE_CXX_STANDARD 17)
elseif (NUKE_VERSION_MAJOR VERSION_GREATER_EQUAL 13)
    # Nuke 13.x: C++14
    message("Configuring for Nuke 13.x: C++14")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
    set(CMAKE_CXX_STANDARD 14)
else()
    # Nuke 12.x: C++11
    message("Configuring for Nuke 12.x: C++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
    set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# add sub directory
add_subdirectory(src)