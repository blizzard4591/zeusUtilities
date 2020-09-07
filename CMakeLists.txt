cmake_minimum_required (VERSION 3.13)

set(CMAKE_SYSTEM_VERSION "10.0.19041.0" CACHE STRING INTERNAL FORCE)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Set project name
project (ZeusDebug CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/")
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

SET(ZEUSDEBUG_CMAKE_SEARCH_PATH "C:/Qt/5.15.0/msvc2019_64" CACHE PATH "Additional Qt5 search path" )

SET(CMAKE_PREFIX_PATH "${OPENMITTSU_CMAKE_SEARCH_PATH}/lib/cmake/")
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/")

set(ZEUSDEBUG_CUSTOM_VERSION_STRING "" CACHE STRING "Disables Git version number checking and uses the custom string instead. Should be of the form 0.1.2-34-567890ab, i.e. major.minor.patch-commitsSinceTag-shortHash")

# Find the Qt library
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /MP /Gz")

	add_definitions(/DNOMINMAX)
	add_definitions(/MP)
	add_definitions(/std:c++latest)
	add_definitions(/D_WINSOCK_DEPRECATED_NO_WARNINGS)
	
	add_definitions(-DUNICODE)
	add_definitions(-D_UNICODE)
endif()

include_directories("${PROJECT_SOURCE_DIR}/srcShared")

#
# Make a version file containing the current version from git.
#

# First check if this is a Git checkout or if a custom version tag should be used.
if ((NOT "${ZEUSDEBUG_CUSTOM_VERSION_STRING}" STREQUAL "") AND (NOT "${ZEUSDEBUG_CUSTOM_VERSION_STRING}" MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+\\-[0-9]+\\-[a-z0-9]+(\\-.*)?$"))
	message(FATAL_ERROR "Builtin version information: A custom version string is set, but it is malformed. Should be: 0.1.2-34-567890ab, i.e. major.minor.patch-commitsSinceTag-shortHash")
endif()

set(ZEUSDEBUG_GIT_VERSION_STRING "")
if (NOT "${ZEUSDEBUG_CUSTOM_VERSION_STRING}" STREQUAL "")
	message(STATUS "Builtin version information: custom version string set, using ${ZEUSDEBUG_CUSTOM_VERSION_STRING}")
	set(ZEUSDEBUG_GIT_VERSION_STRING "${ZEUSDEBUG_CUSTOM_VERSION_STRING}")
elseif (NOT EXISTS "${PROJECT_SOURCE_DIR}/.git")
	message(STATUS "Builtin version information: .git folder non-existant, falling back to 0.0.0-0-00000000")
	set(ZEUSDEBUG_GIT_VERSION_STRING "0.0.0-0-00000000")
else()
	message(STATUS "Builtin version information: .git folder exists, using git_describe_checkout")
	include(GetGitRevisionDescription)
	git_describe_checkout(ZEUSDEBUG_GIT_VERSION_STRING)
endif()

if ("${ZEUSDEBUG_GIT_VERSION_STRING}" MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+\\-[0-9]+\\-[a-z0-9]+(\\-.*)?$")
	# Parse the git Tag into variables
	string(REGEX REPLACE "^([0-9]+)\\..*" "\\1" ZEUSDEBUG_CPP_VERSION_MAJOR "${ZEUSDEBUG_GIT_VERSION_STRING}")
	string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1" ZEUSDEBUG_CPP_VERSION_MINOR "${ZEUSDEBUG_GIT_VERSION_STRING}")
	string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" ZEUSDEBUG_CPP_VERSION_PATCH "${ZEUSDEBUG_GIT_VERSION_STRING}")
	string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+\\-([0-9]+)\\-.*" "\\1" ZEUSDEBUG_CPP_VERSION_COMMITS_AHEAD "${ZEUSDEBUG_GIT_VERSION_STRING}")
	string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+\\-[0-9]+\\-([a-z0-9]+).*" "\\1" ZEUSDEBUG_CPP_VERSION_HASH "${ZEUSDEBUG_GIT_VERSION_STRING}")
	string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+\\-[0-9]+\\-[a-z0-9]+\\-(.*)" "\\1" ZEUSDEBUG_CPP_VERSION_APPENDIX "${ZEUSDEBUG_GIT_VERSION_STRING}")
	if ("${ZEUSDEBUG_CPP_VERSION_APPENDIX}" MATCHES "^.*dirty.*$")
		set(ZEUSDEBUG_CPP_VERSION_DIRTY 1)
	else()
		set(ZEUSDEBUG_CPP_VERSION_DIRTY 0)
	endif()
message(STATUS "ZeusDebug - Version information: ${ZEUSDEBUG_CPP_VERSION_MAJOR}.${ZEUSDEBUG_CPP_VERSION_MINOR}.${ZEUSDEBUG_CPP_VERSION_PATCH} (${ZEUSDEBUG_CPP_VERSION_COMMITS_AHEAD} commits ahead of Tag) build from ${ZEUSDEBUG_CPP_VERSION_HASH} (Dirty: ${ZEUSDEBUG_CPP_VERSION_DIRTY})")
else()
	message(STATUS "No GIT support, no tags fetched or other problems. Defaulting to version 0.0.0-0-00000000 (result was ${ZEUSDEBUG_GIT_VERSION_STRING})")
	set(ZEUSDEBUG_CPP_VERSION_MAJOR 0)
	set(ZEUSDEBUG_CPP_VERSION_MINOR 0)
	set(ZEUSDEBUG_CPP_VERSION_PATCH 0)
	set(ZEUSDEBUG_CPP_VERSION_COMMITS_AHEAD 0)
	set(ZEUSDEBUG_CPP_VERSION_HASH "00000000")
	set(ZEUSDEBUG_CPP_VERSION_DIRTY 0)
endif()

# Generate a version string
set(ZEUSDEBUG_VERSION_NUM "${ZEUSDEBUG_CPP_VERSION_MAJOR}.${ZEUSDEBUG_CPP_VERSION_MINOR}.${ZEUSDEBUG_CPP_VERSION_PATCH}")
set(ZEUSDEBUG_VERSION "${ZEUSDEBUG_CPP_VERSION_MAJOR}.${ZEUSDEBUG_CPP_VERSION_MINOR}.${ZEUSDEBUG_CPP_VERSION_PATCH}")
if (ZEUSDEBUG_CPP_VERSION_COMMITS_AHEAD GREATER 0)
	set(ZEUSDEBUG_VERSION "${ZEUSDEBUG_VERSION}+${ZEUSDEBUG_CPP_VERSION_COMMITS_AHEAD}")
endif()
set(ZEUSDEBUG_VERSION "${ZEUSDEBUG_VERSION}-${ZEUSDEBUG_CPP_VERSION_HASH}")

# Configure a file to pass the ZeusDebug version to the source code
configure_file (
	"${PROJECT_SOURCE_DIR}/version.cpp.in"
	"${PROJECT_BINARY_DIR}/include/version.cpp"
)


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

# Plot Sources
file(GLOB_RECURSE ZEUSDEBUG_PLOT_HEADERS ${PROJECT_SOURCE_DIR}/srcPlot/*.h)
file(GLOB_RECURSE ZEUSDEBUG_PLOT_SOURCES_CPP ${PROJECT_SOURCE_DIR}/srcPlot/*.cpp)

file(GLOB_RECURSE ZEUSDEBUG_PLOT_SOURCES_C ${PROJECT_SOURCE_DIR}/srcPlot/*.c)
file(GLOB_RECURSE ZEUSDEBUG_PLOT_FORMS ${PROJECT_SOURCE_DIR}/srcPlot/*.ui)
file(GLOB_RECURSE ZEUSDEBUG_PLOT_RESOURCES ${PROJECT_SOURCE_DIR}/resources/*.qrc)

QT5_WRAP_UI(ZEUSDEBUG_PLOT_FORMS_HEADERS ${ZEUSDEBUG_PLOT_FORMS})
QT5_ADD_RESOURCES(ZEUSDEBUG_PLOT_RESOURCESOURCES ${ZEUSDEBUG_PLOT_RESOURCES})

# Shared Sources
file(GLOB_RECURSE ZEUSDEBUG_SHARED_HEADERS ${PROJECT_SOURCE_DIR}/srcShared/*.h)
file(GLOB_RECURSE ZEUSDEBUG_SHARED_SOURCES_CPP ${PROJECT_SOURCE_DIR}/srcShared/*.cpp)

# Add the executables
add_executable(zeusDebug ${ZEUSDEBUG_HEADERS} ${ZEUSDEBUG_SOURCES_C} ${ZEUSDEBUG_SOURCES_CPP} ${ZEUSDEBUG_RESOURCESOURCES} ${ZEUSDEBUG_FORMS_HEADERS} ${ZEUSDEBUG_SHARED_HEADERS} ${ZEUSDEBUG_SHARED_SOURCES_CPP} "${PROJECT_BINARY_DIR}/include/version.cpp")
add_executable(zeusDebugPlot ${ZEUSDEBUG_PLOT_HEADERS} ${ZEUSDEBUG_PLOT_SOURCES_C} ${ZEUSDEBUG_PLOT_SOURCES_CPP} ${ZEUSDEBUG_PLOT_RESOURCESOURCES} ${ZEUSDEBUG_PLOT_FORMS_HEADERS} ${ZEUSDEBUG_SHARED_HEADERS} ${ZEUSDEBUG_SHARED_SOURCES_CPP} "${PROJECT_BINARY_DIR}/include/version.cpp")

target_include_directories(zeusDebug PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_include_directories(zeusDebugPlot PRIVATE "${PROJECT_SOURCE_DIR}/srcPlot")

if (MSVC)
	set_target_properties(zeusDebug PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
	set_target_properties(zeusDebug PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
	
	set_target_properties(zeusDebugPlot PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
endif(MSVC)

# Use the required modules from Qt 5.
target_link_libraries(zeusDebug Qt5::Core)
target_link_libraries(zeusDebug Qt5::Gui)
target_link_libraries(zeusDebug Qt5::Widgets)
target_link_libraries(zeusDebug Qt5::Network)
target_link_libraries(zeusDebug Qt5::PrintSupport)

target_link_libraries(zeusDebugPlot Qt5::Core)
target_link_libraries(zeusDebugPlot Qt5::Gui)
target_link_libraries(zeusDebugPlot Qt5::Widgets)
target_link_libraries(zeusDebugPlot Qt5::PrintSupport)

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
	target_link_libraries(zeusDebug comctl32)
	target_link_libraries(zeusDebug cfgmgr32)
	target_link_libraries(zeusDebug uxtheme)
	target_link_libraries(zeusDebug winsta)
	target_link_libraries(zeusDebug winhttp)
	target_link_libraries(zeusDebug aclui)
	target_link_libraries(zeusDebug pdh)
	target_link_libraries(zeusDebug wbemUuid)
	target_link_libraries(zeusDebug shlwapi)
	target_link_libraries(zeusDebug windowscodecs)
endif()

