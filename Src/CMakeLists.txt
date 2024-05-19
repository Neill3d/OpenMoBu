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

# Include of source folders
add_subdirectory( Projects )
