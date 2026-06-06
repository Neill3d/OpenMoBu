
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

add_library(xxhash SHARED IMPORTED GLOBAL)
set_target_properties(xxhash PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/third_party/xxhash
    IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/third_party/xxhash.lib
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
list(FILTER SRCS EXCLUDE REGEX ".*/tests/.*")
# Core value/storage files are compiled into shader_property_core static library
# and must not be compiled again into the plugin DLL to avoid duplicate symbols.
list(FILTER SRCS EXCLUDE REGEX "shaderproperty_value\.(h|cpp)$")
list(FILTER SRCS EXCLUDE REGEX "shaderproperty_storage\.(h|cxx)$")

set (SHADERS
    GLSL/blur.fsh
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
    GLSL/imageBlur.glslf
    GLSL/insert_header.glslf
    GLSL/insert_image_crop.glslf
    GLSL/insert_masking.glslf
    GLSL/lensFlare.fsh
    GLSL/lensFlareAnamorphic.fsh
    GLSL/lensFlareBubble.fsh
    GLSL/mix.fsh
    GLSL/motionblur.fsh
    GLSL/scene_masked.glslf
    GLSL/scene_masked.glslv
    GLSL/simple.fsh
    GLSL/simple.vsh
    GLSL/simple130.glslv
    GLSL/ssao.fsh
    GLSL/ssao_linearize.fsh
    GLSL/text.frag
    GLSL/text.vert
    GLSL/vignetting.fsh
)
source_group("Shaders" FILES ${SHADERS})

set (POSTEFFECTSCOMMON
    posteffect_chain.h
    posteffect_chain.cxx
    posteffect_context.h
    posteffect_contextmobu.cpp
    posteffect_contextmobu.h
    posteffect_buffers.h
    posteffect_buffers.cxx
    posteffect_shader.h
    posteffect_shader.cxx
    posteffect_rendercontext.h
    posteffect_rendercontext.cxx
    shaderproperty.h
    shaderproperty.cxx
    shaderproperty_scheme.h
    shaderproperty_scheme.cxx
    shaderproperty_writer.h
    # shaderproperty_value and shaderproperty_storage live in shader_property_core
)
source_group("PostEffectsCommon" FILES ${POSTEFFECTSCOMMON})

set (POSTEFFECTS
    posteffectshader_color.h
    posteffectshader_color.cxx
    posteffectshader_displacement.h
    posteffectshader_displacement.cpp
    posteffectshader_dof.h
    posteffectshader_dof.cxx
    posteffectshader_filmgrain.h
    posteffectshader_filmgrain.cxx
    posteffectshader_fisheye.h
    posteffectshader_fisheye.cxx
    posteffectshader_lensflare.h
    posteffectshader_lensflare.cpp
    posteffectshader_motionblur.h
    posteffectshader_motionblur.cpp
    posteffectshader_ssao.h
    posteffectshader_ssao.cpp
    posteffectshader_vignetting.h
    posteffectshader_vignetting.cxx
)
source_group("PostEffects" FILES ${POSTEFFECTS})

set (POSTEFFECTSSHADERS
    posteffectshader_bilateral_blur.h
    posteffectshader_bilateral_blur.cxx
    posteffectshader_blur_lineardepth.h
    posteffectshader_blur_lineardepth.cxx
    posteffectshader_downscale.h
    posteffectshader_downscale.cxx
    posteffectshader_lineardepth.h
    posteffectshader_lineardepth.cxx
    posteffectshader_mix.h
    posteffectshader_mix.cxx
)
source_group("PostEffectsShaders" FILES ${POSTEFFECTSSHADERS})

# Build the shared library WITHOUT test sources
add_library(${PROJECT_NAME} SHARED ${SRCS} ${SHADERS} ${POSTEFFECTSCOMMON} ${POSTEFFECTS} ${POSTEFFECTSSHADERS} ${IMGUI_CORE} ${IMGUI_NODES})

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
# shader_property_core — fbsdk-free static library
#
# Contains ShaderPropertyValue and ShaderPropertyStorage: the value holder and
# the lock-free double-buffered map.  No MoBu SDK types are included — FBTexture
# and EffectShaderUserObject are only forward-declared.  This lets the test
# executable link against shader_property_core alone, with no fbsdk dependency.

set(SHADER_PROPERTY_CORE_SRCS
    shaderproperty_value.h
    shaderproperty_value.cpp
    shaderproperty_storage.h
    shaderproperty_storage.cxx
)
source_group("ShaderPropertyCore" FILES ${SHADER_PROPERTY_CORE_SRCS})

add_library(shader_property_core STATIC ${SHADER_PROPERTY_CORE_SRCS})

# PUBLIC: consumers (DLL + tests) inherit these include paths automatically
# when they link against shader_property_core.
target_include_directories(shader_property_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}          # shaderproperty_value.h, shaderproperty_storage.h
    ${CMAKE_SOURCE_DIR}/MotionCodeLibrary # Logger.h (LOGE/LOGI decls), mobu_logging.h (VERIFY macro)
    ${GLEW_INCLUDE_DIRS}                 # GL/glew.h -> GLint used in ShaderPropertyValue
)
target_compile_definitions(shader_property_core PUBLIC GLEW_STATIC NOMINMAX)
target_compile_features(shader_property_core PUBLIC cxx_std_17)

#
# link libraries

target_link_libraries(${PROJECT_NAME} PRIVATE fbsdk OpenGL::GL OpenGL::GLU GLEW::glew_s freetype2 freetype-gl MotionCodeLibrary xxhash shader_property_core)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENREALITY_ROOT}/include ${CMAKE_SOURCE_DIR}/MotionCodeLibrary)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party/imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party/imnodes)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party/json/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party/magic_enum/include)

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


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set (POSTEFFECTSTESTS
    tests/posteffectbase_shaderproperty_tests.cpp
    tests/test_stubs.cpp
)
source_group("Tests" FILES ${POSTEFFECTSTESTS})

# Test executable links against shader_property_core only — no fbsdk, no DLL.
# Include directories (MotionCodeLibrary, GLEW headers, the source dir itself)
# propagate automatically via shader_property_core's PUBLIC interface.
add_executable(manager_PostProcessing_tests ${POSTEFFECTSTESTS})
target_link_libraries(manager_PostProcessing_tests PRIVATE shader_property_core gtest_main)
target_compile_features(manager_PostProcessing_tests PRIVATE cxx_std_17)

# Enable CTest
enable_testing()
include(GoogleTest)

# DISCOVERY_MODE PRE_TEST: test enumeration runs just before CTest executes
# the tests (i.e. when VS Test Explorer clicks Run), NOT during the build step.
#
# The default POST_BUILD mode runs --gtest_list_tests as an MSBuild post-build
# command. That fails here because the test exe cannot start at build time —
# MoBu DLLs (fbsdk etc.) are not in PATH during the build, so the discovery
# script exits with code 1 and MSBuild reports MSB3073.
#
# With PRE_TEST, the build always succeeds. Discovery happens later when the
# test runner (CTest / VS Test Explorer) invokes the binary, at which point
# the system PATH contains the MoBu installation directories.
gtest_discover_tests(manager_PostProcessing_tests
    DISCOVERY_MODE PRE_TEST
)