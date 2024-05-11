
project(solver_calculateNormals LANGUAGES CXX)

file(GLOB_RECURSE SRCS *.cxx *.cpp *.h *.cs)
add_library(${PROJECT_NAME} SHARED ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE ${OPENREALITY_ROOT}/include ${CMAKE_SOURCE_DIR}/MotionCodeLibrary)

#
# GLSL shaders in the project
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENREALITY_ROOT}/include ${CMAKE_SOURCE_DIR}/MotionCodeLibrary)

set_source_files_properties(GLSL_CS/recomputeNormals.cs PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(GLSL_CS/recomputeNormalsZero.cs PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(GLSL_CS/recomputeNormalsNorm.cs PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(GLSL_CS/recomputeNormalsDup.cs PROPERTIES HEADER_FILE_ONLY TRUE)

# Read Product Version
file(READ ${CMAKE_SOURCE_DIR}/PRODUCT_VERSION.txt productversion)
target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_VERSION=${productversion} GLEW_STATIC NOMINMAX)

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

target_link_libraries(${PROJECT_NAME} PRIVATE fbsdk OpenGL::GL OpenGL::GLU GLEW::glew_s Version MotionCodeLibrary)

if (COPY_TO_PLUGINS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/bin/${productversion}/plugins/${PROJECT_NAME}.dll
    )
endif()

#
# Copy GLSL shaders to the output binary directory where they can be located

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/GLSL_CS ${CMAKE_BINARY_DIR}/bin/${productversion}/plugins/GLSL_CS
)