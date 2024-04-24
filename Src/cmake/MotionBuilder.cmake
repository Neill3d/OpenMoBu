# set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 14)
set(CMAKE_CUDA_STANDARD_REQUIRED TRUE)

set(CMAKE_INSTALL_MESSAGE LAZY)

# Debug vs Release differences
get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(isMultiConfig)
	if (CMAKE_VERSION VERSION_LESS "3.20")
		message(FATAL_ERROR "Multi config generator not supported")
	else()
		message("Multi config generator")
		set(Configuration $<IF:$<CONFIG:Debug>,debug,release>)
		if (WIN32)
			get_filename_component(install_prefix ${CMAKE_INSTALL_PREFIX} NAME)
			set(BIN_PATH $<IF:$<CONFIG:RelWithDebInfo>,x64,${install_prefix}>)
		else()
			set(BIN_PATH $<IF:$<CONFIG:RelWithDebInfo>,linux_64,${CMAKE_BUILD_TYPE}>)
		endif()
	endif()
else()
	if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(Configuration debug)
	else()
		set(Configuration release)
	endif()
	if (WIN32)
		if ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
			set(BIN_PATH x64)
		else()
			set(BIN_PATH x64-${CMAKE_BUILD_TYPE})
		endif()
	else()
		if ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
			set(BIN_PATH linux_64)
		else()
			set(BIN_PATH linux_64-${CMAKE_BUILD_TYPE})
		endif()
	endif()	
endif()
add_compile_definitions($<$<CONFIG:Debug>:_DEBUG> $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

# Build with debug version of Python in Debug builds
if (${CMAKE_BUILD_TYPE} MATCHES "^Debug$")
    add_compile_definitions(BOOST_DEBUG_PYTHON)
endif()

if (WIN32)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099") # PDB 'filename' was not found with 'object/library' or at 'path'; linking object as if no debug info
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -WX") # Warnings as errors

	if (NOT ${CMAKE_BUILD_TYPE} MATCHES "^Debug$")
		# Those are default Visual Studio release flags which are not defaults for CMake.
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:inline /fp:precise /GS /Gm- /GR /Gd /Zc:wchar_t /Zc:forScope") 
	endif()
endif()

cmake_policy(SET CMP0072 NEW) # FindOpenGL prefers GLVND by default when available.
find_package(OpenGL REQUIRED)

set(QtVersion 6.5.3)

if(QtVersion VERSION_LESS 6)
	set(QT_VERSION_MAJOR 5)
	add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)
else()
	set(QT_VERSION_MAJOR 6)
	add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060F00)
endif()

# Function to set CMake variable if to the value of environment variable if it exist.
# otherwise setting it to ARGV1 if specified.
# var:		environment variable name
# [ARGV1]:	Artifactory path
function(CheckEnvVariable var)
	if (DEFINED ENV{${var}})
		file(TO_CMAKE_PATH $ENV{${var}} path)
		set(${var} ${path} PARENT_SCOPE)
	else()
		if(DEFINED MB_USE_ARTIFACTORY AND DEFINED ARGV1)
			set(${var} ${ARGV1} PARENT_SCOPE)
		endif()
	endif()
endfunction(CheckEnvVariable)

# Function creates a symbolic link instead of copying all the files.
# srcdir:	source folder
# dstdir:	destination folder
function(mobu_link_artifact srcdir dstdir)
	if (IS_DIRECTORY ${dstdir} AND NOT IS_SYMLINK ${dstdir})
		# Remove the previous copied folder
		file(REMOVE_RECURSE ${dstdir})
	endif()

	if (IS_ABSOLUTE ${dstdir})
		get_filename_component(dstParent ${dstdir} DIRECTORY)
		file(MAKE_DIRECTORY ${dstParent})
	endif()

	file(CREATE_LINK ${srcdir} ${dstdir} SYMBOLIC RESULT result)
	if (NOT result EQUAL 0)
		message(FATAL_ERROR "Failed to create symbolic link ${dstdir}->${srcdir}: ${result}...")
	endif()
endfunction(mobu_link_artifact)


function(mobu_target_finalize target extension)
	# Detect PythonPlugins
	if(extension MATCHES "python")
		set(IsPythonPlugin TRUE)
		set_target_properties(${target} PROPERTIES PREFIX "")
		set(extension python)
	endif()

	target_compile_definitions(${target} PRIVATE BUILD_VERSION=${buildversion})
	target_compile_definitions(${target} PRIVATE PRODUCT_VERSION=${productversion})
	target_compile_definitions(${target} PRIVATE K_APPLICATION_COPYRIGHT_YEAR="${copyrightYear}")

	if(WIN32)
		if(EXISTS ${MOBU_ROOT}/src/MotionBuilder/kupires/version.rc)
			target_sources(${target} PRIVATE ${MOBU_ROOT}/src/MotionBuilder/kupires/version.rc)
		endif()
		if(IsPythonPlugin)
			if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
				set_target_properties(${target} PROPERTIES SUFFIX "_d.pyd")
			else()
				set_target_properties(${target} PROPERTIES SUFFIX ".pyd")
			endif()
		endif()
	endif()

	if (Qt6_FOUND)
		# Stop QT from defining UNICODE
		qt_disable_unicode_defines(${target})
	endif()

	set_target_properties( ${target}
	  PROPERTIES
	  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${BIN_PATH}/${extension}"
	  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${BIN_PATH}/${extension}"
	)
endfunction(mobu_target_finalize)
