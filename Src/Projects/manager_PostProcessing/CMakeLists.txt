
add_library(freetype2 SHARED IMPORTED GLOBAL)
set_target_properties(freetype2 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/third_party/freetype2/include
    IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/third_party/freetype.lib
)

add_library(freetype-gl SHARED IMPORTED GLOBAL)
set_target_properties(freetype-gl PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/third_party/freetype-gl
    IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/third_party/freetype-gl.lib
)

project(manager_PostProcessing LANGUAGES CXX)

file(GLOB_RECURSE SRCS *.cxx *.cpp *.c *.h)
file(GLOB_RECURSE SRCS_SHADERS *.fsh *.vsh)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/GLSL FILES "${SRCS_SHADERS}")

add_library(${PROJECT_NAME} SHARED ${SRCS} ${SRCS_SHADERS})

# Read Product Version
file(READ ${CMAKE_SOURCE_DIR}/PRODUCT_VERSION.txt productversion)
target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_VERSION=${productversion} GLEW_STATIC HUD_FONT)

#
# GLEW

set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/third_party/glew)
set(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/third_party/glew/lib/Release/x64)
set (GLEW_USE_STATIC_LIBS TRUE)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})

#
# link libraries

target_link_libraries(${PROJECT_NAME} PRIVATE fbsdk OpenGL::GL OpenGL::GLU GLEW::glew_s freetype2 freetype-gl MotionCodeLibrary)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENREALITY_ROOT}/include ${CMAKE_SOURCE_DIR}/MotionCodeLibrary)

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