cmake_minimum_required (VERSION 2.8.12)

set(CMAKE_SYSTEM_VERSION "10.0.19041.0" CACHE STRING INTERNAL FORCE)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Set project name
project (ZeusDebug CXX C)

include(CheckIncludeFiles)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckLibraryExists)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CMakeDependentOption)
include(CheckCXXSymbolExists)
include(CheckTypeSize)
include(FindThreads)
include(GNUInstallDirs)
include(CheckTypeSize)
include(CMakePackageConfigHelpers)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

SET(CMAKE_PREFIX_PATH "C:/Qt/5.14.2/msvc2017_64/lib/cmake/")
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/")

# Find the Qt library
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

include(ExternalProject)
ExternalProject_Add(processHackerProject 
	SOURCE_DIR "${PROJECT_SOURCE_DIR}/thirdParty/processhacker" 
	INSTALL_DIR "${PROJECT_BINARY_DIR}/installProcessHacker"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
link_directories("${PROJECT_BINARY_DIR}/installProcessHacker/lib")

# Add base folder for better inclusion paths
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/processhacker")

include_directories("${PROJECT_SOURCE_DIR}/thirdParty/processhacker/sdk/include")
include_directories("${PROJECT_SOURCE_DIR}/thirdParty/processhacker/ProcessHacker/include")
include_directories("${PROJECT_SOURCE_DIR}/thirdParty/processhacker/plugins/ExtendedTools")

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /MP /Gz")

	add_definitions(/DNOMINMAX)
	add_definitions(/MP)
	add_definitions(/std:c++latest)
	add_definitions(/Gz)
	
	add_definitions(-DUNICODE)
	add_definitions(-D_UNICODE)
	
	add_definitions(/D_PHLIB_)
	add_definitions(/D_PHAPP_)
	add_definitions(/DPHLIB_NO_DEFAULT_LIB)
endif()

if (WIN32)
	# Cannot use check_library_exists because the symbol is always declared as char(*)(void)
	set(CMAKE_REQUIRED_LIBRARIES "ws2_32.lib")
	check_cxx_symbol_exists(WSAStartup "winsock2.h" HAVE_WS2_32)

	set(CMAKE_REQUIRED_LIBRARIES "iphlpapi.lib")
	check_cxx_symbol_exists(GetAdaptersAddresses "winsock2.h;iphlpapi.h" HAVE_IPHLAPI)

	set(CMAKE_REQUIRED_LIBRARIES "")
	check_library_exists(ws2 fopen "" HAVE_WS2)
endif(WIN32)

if(WIN32 AND NOT CYGWIN)
	if(NOT HAVE_WS2_32 AND NOT HAVE_WS2)
		message(FATAL_ERROR "Cannot link to ws2_32 or ws2")
	endif()
	
	if(NOT HAVE_IPHLAPI)
		message(FATAL_ERROR "Cannot link to iphlapi")
	endif()
endif()

# Main Sources
file(GLOB_RECURSE ZEUSDEBUG_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE ZEUSDEBUG_SOURCES_CPP ${PROJECT_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE ZEUSDEBUG_SOURCES_C ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE ZEUSDEBUG_FORMS ${PROJECT_SOURCE_DIR}/src/*.ui)
file(GLOB_RECURSE ZEUSDEBUG_RESOURCES ${PROJECT_SOURCE_DIR}/resources/*.qrc)

QT5_WRAP_UI(ZEUSDEBUG_FORMS_HEADERS ${ZEUSDEBUG_FORMS})
QT5_ADD_RESOURCES(ZEUSDEBUG_RESOURCESOURCES ${ZEUSDEBUG_RESOURCES})

# Add the executables
add_executable(zeusDebug ${ZEUSDEBUG_HEADERS} ${ZEUSDEBUG_SOURCES_C} ${ZEUSDEBUG_SOURCES_CPP} ${ZEUSDEBUG_RESOURCESOURCES} ${ZEUSDEBUG_FORMS_HEADERS})
# "${PROJECT_SOURCE_DIR}/thirdParty/processhacker/ProcessHacker/ProcessHacker.def"
set_property(TARGET zeusDebug PROPERTY ENABLE_EXPORTS 1)


if (MSVC)
	set_target_properties(zeusDebug PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
	set_target_properties(zeusDebug PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif(MSVC)

# Use the required modules from Qt 5.
target_link_libraries(zeusDebug Qt5::Core)
target_link_libraries(zeusDebug Qt5::Gui)
target_link_libraries(zeusDebug Qt5::Widgets)
target_link_libraries(zeusDebug Qt5::Network)
target_link_libraries(zeusDebug Qt5::PrintSupport)

if(HAVE_WS2_32)
	target_link_libraries(zeusDebug ws2_32)
elseif(HAVE_WS2)
	target_link_libraries(zeusDebug ws2)
endif()

if(HAVE_IPHLAPI)
	target_link_libraries(zeusDebug iphlpapi)
endif()

if (WIN32)
	target_link_libraries(zeusDebug ntdll)
	target_link_libraries(zeusDebug Comctl32)
	target_link_libraries(zeusDebug Cfgmgr32)
	target_link_libraries(zeusDebug Uxtheme)
	target_link_libraries(zeusDebug winsta)
	target_link_libraries(zeusDebug Winhttp)
	target_link_libraries(zeusDebug Aclui)
	target_link_libraries(zeusDebug Pdh)
	target_link_libraries(zeusDebug WbemUuid)
	target_link_libraries(zeusDebug Shlwapi)
	target_link_libraries(zeusDebug windowscodecs)
endif()

target_link_libraries(zeusDebug phlib)
target_link_libraries(zeusDebug processHacker)
target_link_libraries(zeusDebug extendedTools)
