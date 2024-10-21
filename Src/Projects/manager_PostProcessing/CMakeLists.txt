
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

set (IMGUI_CORE 
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imconfig.h
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui.h
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_internal.h
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_impl_opengl3.h
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_impl_win32.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_impl_win32.h
    ${CMAKE_SOURCE_DIR}/third_party/imgui/LICENSE.txt
)
set (IMGUI_NODES 
    ${CMAKE_SOURCE_DIR}/third_party/imnodes/imnodes.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imnodes/imnodes.h
    ${CMAKE_SOURCE_DIR}/third_party/imnodes/imnodes_internal.h
    ${CMAKE_SOURCE_DIR}/third_party/imnodes/LICENSE.md
    ${CMAKE_SOURCE_DIR}/third_party/imnodes/README.md
)

source_group("ImGui" FILES ${IMGUI_CORE})
source_group("ImGuiNodes" FILES ${IMGUI_NODES})

file(GLOB_RECURSE SRCS *.cxx *.cpp *.c *.h)

set (SHADERS
    GLSL/color.fsh
    GLSL/depthLinearize.fsh
    GLSL/displacement.fsh
    GLSL/displacement.vsh
    GLSL/dof.fsh
    GLSL/downscale.fsh
    GLSL/downscale.vsh
    GLSL/filmGrain.fsh
    GLSL/fishEye.fsh
    GLSL/fishEye.vsh
    GLSL/imageBlur.fsh
    GLSL/lensFlare.fsh
    GLSL/lensFlareAnamorphic.fsh
    GLSL/lensFlareBubble.fsh
    GLSL/mix.fsh
    GLSL/motionblur.fsh
    GLSL/scene_masked.glslf
    GLSL/scene_masked.glslv
    GLSL/simple.fsh
    GLSL/simple.vsh
    GLSL/ssao.fsh
    GLSL/ssao_linearize.fsh
    GLSL/text.frag
    GLSL/text.vert
    GLSL/vignetting.fsh
)
source_group("Shaders" FILES ${SHADERS})

add_library(${PROJECT_NAME} SHARED ${SRCS} ${SHADERS} ${IMGUI_CORE} ${IMGUI_NODES})

# Read Product Version
file(READ ${CMAKE_SOURCE_DIR}/PRODUCT_VERSION.txt productversion)
target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_VERSION=${productversion} GLEW_STATIC HUD_FONT NOMINMAX)

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
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party/imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party/imnodes)

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


set_property(GLOBAL PROPERTY USE_FOLDERS ON)