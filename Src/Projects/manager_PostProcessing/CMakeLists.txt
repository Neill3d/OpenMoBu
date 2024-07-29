
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

#add_subdirectory(GLSL)
#add_subdirectory(ImGui)

set (IMGUI_CORE ${CMAKE_SOURCE_DIR}/third_party/imgui/*.*)
set (IMGUI_NODES ${CMAKE_SOURCE_DIR}/third_party/imnodes/*.*)

source_group("ImGui" FILES ${IMGUI_CORE})
source_group("ImGui" FILES ${IMGUI_NODES})

file(GLOB_RECURSE SRCS *.cxx *.cpp *.c *.h)
file(GLOB_RECURSE SRCS_SHADERS *.fsh *.vsh *.glslv *.glslf)

file(GLOB_RECURSE IMGUI_SRCS ${CMAKE_SOURCE_DIR}/third_party/imgui/*.*)
file(GLOB_RECURSE IMGUI_NODES_SRCS ${CMAKE_SOURCE_DIR}/third_party/imnodes/*.*)

#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/GLSL FILES "${SRCS_SHADERS}")
source_group(${CMAKE_CURRENT_SOURCE_DIR}/GLSL FILES "${SRCS_SHADERS}")
#set (SRCS ${SRCS} ${SRCS_SHADERS} ${IMGUI_SRCS} ${IMGUI_NODES_SRCS})

add_library(${PROJECT_NAME} SHARED ${SRCS} ${SRCS_SHADERS} ${IMGUI_SRCS} ${IMGUI_NODES_SRCS})

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