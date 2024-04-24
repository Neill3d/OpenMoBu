
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

target_link_libraries(${PROJECT_NAME} PRIVATE fbsdk OpenGL::GL GLEW::glew_s)