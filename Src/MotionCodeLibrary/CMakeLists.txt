
project(MotionCodeLibrary LANGUAGES CXX)

file(GLOB_RECURSE SRCS *.cxx *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE ${OPENREALITY_ROOT}/include)

# Read Product Version
file(READ ${CMAKE_SOURCE_DIR}/PRODUCT_VERSION.txt productversion)

target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_VERSION=${productversion})

target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX TIXML_USE_STL GLEW_STATIC)

#
# GLEW

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/third_party/glew)
set(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/third_party/glew/lib/Release/x64)
set (GLEW_USE_STATIC_LIBS TRUE)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})

# fbxsdk is intentionally not linked here: only FBXUtils.cpp uses it (headers come from
# OPENREALITY_ROOT/include above), and consumers that need FBX link fbxsdk themselves.
# Linking it here would propagate the import lib to every plugin, since this is a static library.
target_link_libraries(${PROJECT_NAME} PRIVATE Version.lib fbsdk OpenGL::GL OpenGL::GLU GLEW::glew_s)

#
# XXHASH - built from the sources in third_party/xxhash

add_library(xxhash_local STATIC
  "${CMAKE_SOURCE_DIR}/third_party/xxhash/xxhash.c"
)

target_include_directories(xxhash_local PUBLIC
  "${CMAKE_SOURCE_DIR}/third_party/xxhash"
)

target_link_libraries(${PROJECT_NAME} PRIVATE xxhash_local)