cmake_minimum_required(VERSION 3.0)

project(fritzgraph)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions(-std=c++14 -Wall -Wpedantic)
    add_definitions(-fno-exceptions -fno-rtti)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        add_definitions(-O0) # -g is added automatically
    elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        add_definitions(-s) # -O3 is added automatically
    endif()
endif()

find_package(OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Qml Quick Network)

add_subdirectory(src)
add_subdirectory(assets)
