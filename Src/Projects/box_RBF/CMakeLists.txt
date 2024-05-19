
project(box_RBF LANGUAGES CXX)

file(GLOB_RECURSE SRCS *.cxx *.cpp *.c *.h)
add_library(${PROJECT_NAME} SHARED ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE ${OPENREALITY_ROOT}/include ${CMAKE_SOURCE_DIR}/MotionCodeLibrary)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/third_party)

# Read Product Version
file(READ ${CMAKE_SOURCE_DIR}/PRODUCT_VERSION.txt productversion)
target_compile_definitions(${PROJECT_NAME} PRIVATE PRODUCT_VERSION=${productversion})

#
# link libraries

target_link_libraries(${PROJECT_NAME} PRIVATE fbsdk MotionCodeLibrary)

if (COPY_TO_PLUGINS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/bin/${productversion}/plugins/${PROJECT_NAME}.dll
    )
endif()