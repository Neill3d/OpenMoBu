include(cmake/MotionBuilder.cmake)

CheckEnvVariable(MOBU_ROOT)
if( NOT MOBU_ROOT )
	message(FATAL_ERROR "MOBU_ROOT must be specified; could be specified as an environment variable or a CMake variable")
endif()

if(NOT buildversion)
	set(buildversion 0)
endif()

# Read Product Version
file(READ ${OPENREALITY_ROOT}/PRODUCT_VERSION.txt productversion)

# If requested path doesn't exist, override BIN_PATH with default release build.
if(NOT EXISTS ${OPENREALITY_ROOT}/lib/${BIN_PATH})
	if (WIN32)
		set(BIN_PATH x64)
	else()
		set(BIN_PATH linux_64)
	endif()	
endif()

add_library(fbsdk SHARED IMPORTED GLOBAL)
set_target_properties(fbsdk PROPERTIES
	IMPORTED_LOCATION ${MOBU_ROOT}/bin/${BIN_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}fbsdk${CMAKE_SHARED_LIBRARY_SUFFIX}
    INTERFACE_INCLUDE_DIRECTORIES ${OPENREALITY_ROOT}/include
    IMPORTED_IMPLIB ${OPENREALITY_ROOT}/lib/${BIN_PATH}/fbsdk${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(boost_python SHARED IMPORTED GLOBAL)
set_target_properties(boost_python PROPERTIES
	IMPORTED_LOCATION ${MOBU_ROOT}/bin/${BIN_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}boost_python${CMAKE_SHARED_LIBRARY_SUFFIX}
    INTERFACE_INCLUDE_DIRECTORIES ${OPENREALITY_ROOT}/include 
	INTERFACE_INCLUDE_DIRECTORIES ${OPENREALITY_ROOT}/include/python/include
    IMPORTED_IMPLIB ${OPENREALITY_ROOT}/lib/${BIN_PATH}/boost_python${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(fbxsdk SHARED IMPORTED GLOBAL)
set_target_properties(fbxsdk PROPERTIES
	IMPORTED_LOCATION ${MOBU_ROOT}/bin/${BIN_PATH}/libfbxsdk-adsk${CMAKE_SHARED_LIBRARY_SUFFIX}
    INTERFACE_INCLUDE_DIRECTORIES ${OPENREALITY_ROOT}/include
    IMPORTED_IMPLIB ${OPENREALITY_ROOT}/lib/${BIN_PATH}/libfbxsdk-adsk${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(pyfbsdk SHARED IMPORTED GLOBAL)
set_target_properties(pyfbsdk PROPERTIES
	IMPORTED_LOCATION ${MOBU_ROOT}/bin/${BIN_PATH}/python/pyfbsdk${CMAKE_SHARED_LIBRARY_SUFFIX}
    INTERFACE_INCLUDE_DIRECTORIES ${OPENREALITY_ROOT}/include
    IMPORTED_IMPLIB ${OPENREALITY_ROOT}/lib/${BIN_PATH}/pyfbsdk${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(HIK2013 INTERFACE IMPORTED GLOBAL)
set_target_properties(HIK2013 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${OPENREALITY_ROOT}/include/HIK2013
    INTERFACE_LINK_LIBRARIES ${OPENREALITY_ROOT}/lib/${BIN_PATH}/HIK2013/${CMAKE_STATIC_LIBRARY_PREFIX}humanik${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(HIK2014 INTERFACE IMPORTED GLOBAL)
set_target_properties(HIK2014 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${OPENREALITY_ROOT}/include/HIK2014
    INTERFACE_LINK_LIBRARIES ${OPENREALITY_ROOT}/lib/${BIN_PATH}/HIK2014/${CMAKE_STATIC_LIBRARY_PREFIX}humanik${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(HIK2016 INTERFACE IMPORTED GLOBAL)
set_target_properties(HIK2016 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${OPENREALITY_ROOT}/include/HIK2016
    INTERFACE_LINK_LIBRARIES ${OPENREALITY_ROOT}/lib/${BIN_PATH}/HIK2016/${CMAKE_STATIC_LIBRARY_PREFIX}humanik${CMAKE_STATIC_LIBRARY_SUFFIX}
)

CheckEnvVariable(QTDIR)
if( QTDIR )
	message("QTDIR = ${QTDIR}")
	message("QtVersion = ${QtVersion}")

	# Setting up for Qt auto detection of tools to execute
	cmake_policy(SET CMP0071 NEW)
	set(CMAKE_PREFIX_PATH ${QTDIR})

    if(QtVersion VERSION_LESS 6)
        set(QtExtras OpenGLExtensions)
        if (NOT WIN32)
            list(APPEND QtExtras X11Extras)
        endif()
    else()
        set(QtExtras OpenGLWidgets)
    endif()
    
    find_package(Qt${QT_VERSION_MAJOR} ${QtVersion} REQUIRED COMPONENTS Core Gui Svg Widgets Multimedia OpenGL UiTools WebEngineWidgets Xml ${QtExtras})
endif()

CheckEnvVariable(PXR_USD_LOCATION)
CheckEnvVariable(CG_INC_PATH)
CheckEnvVariable(CG_LIB64_PATH)
CheckEnvVariable(KINECTSDK10_DIR)

if(NOT CG_INC_PATH)
	add_definitions( -DK_NO_CG )
endif()

link_directories( lib/${BIN_PATH} )
