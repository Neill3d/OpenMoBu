
project(shader_SuperDynamicLighting LANGUAGES CXX)

set (SHADERS
    GLSL/scene_bufferid.fsh
    GLSL/scene_bufferid.vsh
    GLSL/scene_shading.fsh
    GLSL/scene_shading.vsh
    GLSL/scene_shadow.fsh
    GLSL/scene_shadow.vsh
)
source_group("Shaders" FILES ${SHADERS})


file(GLOB_RECURSE SRCS *.cxx *.cpp *.h)
add_library(${PROJECT_NAME} SHARED ${SRCS} ${SHADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${OPENREALITY_ROOT}/include ${CMAKE_SOURCE_DIR}/MotionCodeLibrary)

# Read Product Version
file(READ ${CMAKE_SOURCE_DIR}/PRODUCT_VERSION.txt productversion)
target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_VERSION=${productversion} GLEW_STATIC)

#
# GLEW

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/third_party/glew)
set(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/third_party/glew/lib/Release/x64)
set (GLEW_USE_STATIC_LIBS TRUE)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})

#
# GLM

set(glm_DIR ${CMAKE_SOURCE_DIR}/third_party/glm) # if necessary
find_package(glm REQUIRED)

#
# link libraries

target_link_libraries(${PROJECT_NAME} PRIVATE fbsdk OpenGL::GL OpenGL::GLU GLEW::glew_s glm MotionCodeLibrary)

if (COPY_TO_PLUGINS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/bin/${productversion}/plugins/${PROJECT_NAME}.dll
    )
endif()

#
# Copy GLSL shaders to the output binary directory where they can be located

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/GLSL ${CMAKE_BINARY_DIR}/bin/${productversion}/plugins/GLSL
)