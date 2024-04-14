
project(manip_lockCamera LANGUAGES CXX)

file(GLOB_RECURSE SRCS *.cxx *.cpp *.h)
add_library(${PROJECT_NAME} SHARED ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE ${OPENREALITY_ROOT}/include ${CMAKE_SOURCE_DIR}/MotionCodeLibrary)

# Read Product Version
file(READ ${OPENREALITY_ROOT}/PRODUCT_VERSION.txt productversion)
target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_VERSION=${productversion} GLEW_STATIC)

#
# GLEW

set(CMAKE_PREFIX_PATH "C:/Users/Sergei/Documents/GitHub/MoPlugs_External/glew")
set(CMAKE_LIBRARY_PATH "C:/Users/Sergei/Documents/GitHub/MoPlugs_External/glew/lib/Release/x64")
set (GLEW_USE_STATIC_LIBS TRUE)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})

#
# link libraries

target_link_libraries(${PROJECT_NAME} PRIVATE fbsdk OpenGL::GL OpenGL::GLU GLEW::glew_s MotionCodeLibrary)

if (COPY_TO_PLUGINS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${MOBU_ROOT}/bin/x64/plugins/${PROJECT_NAME}.dll
    )
endif()