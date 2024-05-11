
project(device_projectTango LANGUAGES CXX)

file(GLOB SRCS *.cxx *.cpp *.c *.h)
file(GLOB SRCS_COMMON ${CMAKE_SOURCE_DIR}/projects/Common_Tango/*.*)
add_library(${PROJECT_NAME} SHARED ${SRCS} ${SRCS_COMMON})

target_include_directories(${PROJECT_NAME} PRIVATE ${OPENREALITY_ROOT}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/projects/Common_Tango)

# Read Product Version
file(READ ${CMAKE_SOURCE_DIR}/PRODUCT_VERSION.txt productversion)
target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_VERSION=${productversion} GLEW_STATIC)

#
# third party zlib

set(ZLIB_ROOT ${CMAKE_SOURCE_DIR}/third_party/zlib-1.2.11)
set(ZLIB_LIBRARY ${CMAKE_SOURCE_DIR}/third_party/zlibstatic.lib)
find_package(zlib REQUIRED)
set(ZLIB_USE_STATIC_LIBS "ON")

#
# GLEW

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/third_party/glew)
set(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/third_party/glew/lib/Release/x64)
set (GLEW_USE_STATIC_LIBS TRUE)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})

#
#

target_link_libraries(${PROJECT_NAME} PRIVATE fbsdk OpenGL::GL OpenGL::GLU GLEW::glew_s ZLIB::ZLIB)

if (COPY_TO_PLUGINS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/bin/${productversion}/plugins/${PROJECT_NAME}.dll
    )
endif()