
project(device_faceCap LANGUAGES CXX)

file(GLOB_RECURSE SRCS *.cxx *.cpp *.c *.h)
add_library(${PROJECT_NAME} SHARED ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE ${OPENREALITY_ROOT}/include)

# Read Product Version
file(READ ${CMAKE_SOURCE_DIR}/PRODUCT_VERSION.txt productversion)
target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_VERSION=${productversion})

#
#

target_link_libraries(${PROJECT_NAME} PRIVATE fbsdk ws2_32)

if (COPY_TO_PLUGINS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${MOBU_ROOT}/bin/x64/plugins/${PROJECT_NAME}.dll
    )
endif()