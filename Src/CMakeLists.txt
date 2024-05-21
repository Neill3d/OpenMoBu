cmake_minimum_required(VERSION 3.19)

# This is the main CMake project for the ORSDK samples, but it is also
# an example on how to create your own CMake plugin project.

# Project name
project(OpenMoBu)

# Read Product Version
file(READ ${CMAKE_SOURCE_DIR}/PRODUCT_VERSION.txt productversion)

# Mandatory path to MotionBuilder Root folder.
set(MOBU_ROOT "C:/Program Files/Autodesk/MotionBuilder ${productversion}")
set(OPENREALITY_ROOT "C:/Program Files/Autodesk/MotionBuilder ${productversion}/OpenRealitySDK" CACHE PATH "Provide a path to OpenReality SDK")

set(COPY_TO_PLUGINS ON CACHE BOOL "Copy binaries into mobu plugins folder?")

# Include OpenReality CMake code to compile plugins
include(cmake/OpenReality.cmake)

# Static library with a shared code - MotionCodeLibrary
add_subdirectory( MotionCodeLibrary )

set_property(GLOBAL PROPERTY USE_FOLDERS On)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "v")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) #Optional

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

# Include of source folders
add_subdirectory( Projects )
