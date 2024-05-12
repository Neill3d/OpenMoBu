
project(shadingGraph_exporter LANGUAGES CXX)

file(GLOB_RECURSE SRCS *.cxx *.cpp *.c *.h)
file(GLOB_RECURSE COMMON_FBX_SRC "${CMAKE_SOURCE_DIR}/Projects/Common_FBXSDK/*.cxx")
file(GLOB_RECURSE COMMON_FBX_INC "${CMAKE_SOURCE_DIR}/Projects/Common_FBXSDK/*.h")

add_executable(${PROJECT_NAME} ${SRCS} ${COMMON_FBX_SRC} ${COMMON_FBX_INC})

target_include_directories(${PROJECT_NAME} PRIVATE ${OPENREALITY_ROOT}/include ${CMAKE_SOURCE_DIR}/MotionCodeLibrary)

# Read Product Version
file(READ ${CMAKE_SOURCE_DIR}/PRODUCT_VERSION.txt productversion)
target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_VERSION=${productversion} TIXML_USE_STL)

#
# link libraries

target_link_libraries(${PROJECT_NAME} PRIVATE fbxsdk MotionCodeLibrary)

if (COPY_TO_PLUGINS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}.exe
    )
endif()