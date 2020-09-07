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

option(ZEUSDEBUG_USE_NSIS "Use NSIS generator to produce a Windows installer." OFF)

SET(ZEUSDEBUG_CMAKE_SEARCH_PATH "C:/Qt/5.15.0/msvc2019_64" CACHE PATH "Additional Qt5 search path" )

SET(CMAKE_PREFIX_PATH "${ZEUSDEBUG_CMAKE_SEARCH_PATH}/lib/cmake/")
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/")

set(ZEUSDEBUG_CUSTOM_VERSION_STRING "" CACHE STRING "Disables Git version number checking and uses the custom string instead. Should be of the form 0.1.2-34-567890ab, i.e. major.minor.patch-commitsSinceTag-shortHash")
SET(ZEUSDEBUG_ADDITIONAL_INSTALL_FILES "" CACHE STRING "A semicolon separated list of files that will be copied to the bin directory on install.")

if(MINGW OR WIN32)
	set(CLI_INSTALL_DIR    ".")
	set(BIN_INSTALL_DIR    ".")
	set(PLUGIN_INSTALL_DIR ".")
	set(DATA_INSTALL_DIR   "share")
endif()

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

# For version.h
include_directories("${PROJECT_SOURCE_DIR}")

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

# Check for %zu support
try_run(ZEUSDEBUG_POINTER_SIZE_TEST_RUN_RESULT ZEUSDEBUG_POINTER_SIZE_TEST_COMPILE_RESULT "${PROJECT_BINARY_DIR}/testPointerSize" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_ptr_size.cpp" COMPILE_OUTPUT_VARIABLE ZEUSDEBUG_POINTER_SIZE_TEST_COMPILE_OUTPUT RUN_OUTPUT_VARIABLE ZEUSDEBUG_POINTER_SIZE_TEST_RUN_OUTPUT)
if (NOT ${ZEUSDEBUG_POINTER_SIZE_TEST_COMPILE_RESULT})
	message(FATAL_ERROR "Failed to compile pointer size testing program, please contact a developer! Extended Information: ${ZEUSDEBUG_POINTER_SIZE_TEST_COMPILE_OUTPUT}") 
elseif(${ZEUSDEBUG_POINTER_SIZE_TEST_RUN_RESULT} LESS 0 OR ${ZEUSDEBUG_POINTER_SIZE_TEST_RUN_RESULT} GREATER 0)
	message(FATAL_ERROR "Failed to run pointer size testing program, please contact a developer! Extended Information: ${ZEUSDEBUG_POINTER_SIZE_TEST_RUN_OUTPUT}") 
else()
	set(ZEUSDEBUG_CPP_POINTER_SIZE "${ZEUSDEBUG_POINTER_SIZE_TEST_RUN_OUTPUT}")
	message(STATUS "Collected pointer size information (pointer has ${ZEUSDEBUG_POINTER_SIZE_TEST_RUN_OUTPUT} Bits)")
endif()

# Check for std::make_unique support
try_run(ZEUSDEBUG_MAKE_UNIQUE_TEST_RUN_RESULT ZEUSDEBUG_MAKE_UNIQUE_TEST_COMPILE_RESULT "${PROJECT_BINARY_DIR}/testMakeUnique" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_make_unique.cpp" COMPILE_OUTPUT_VARIABLE ZEUSDEBUG_MAKE_UNIQUE_TEST_COMPILE_OUTPUT RUN_OUTPUT_VARIABLE ZEUSDEBUG_MAKE_UNIQUE_TEST_RUN_OUTPUT)
if (NOT ${ZEUSDEBUG_MAKE_UNIQUE_TEST_COMPILE_RESULT})
	set(ZEUSDEBUG_CONFIG_HAVE_MAKE_UNIQUE OFF)
	message(STATUS "Compiler does NOT support std::make_unique, using replacement. ${ZEUSDEBUG_MAKE_UNIQUE_TEST_COMPILE_OUTPUT}") 
elseif(${ZEUSDEBUG_MAKE_UNIQUE_TEST_RUN_RESULT} EQUAL 0)
	set(ZEUSDEBUG_CONFIG_HAVE_MAKE_UNIQUE ON)
	message(STATUS "Compiler does support std::make_unique.") 
else()
	set(ZEUSDEBUG_CONFIG_HAVE_MAKE_UNIQUE OFF)
	message(FATAL_ERROR "Failed to run std::make_unique testing program, please contact a developer! Extended Information: ${ZEUSDEBUG_MAKE_UNIQUE_TEST_RUN_RESULT} - ${ZEUSDEBUG_MAKE_UNIQUE_TEST_RUN_OUTPUT}") 
endif()

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

# Sources of Build Tools
file(GLOB_RECURSE ZEUSDEBUG_BUILDTOOLS_VERSIONINFO_HEADERS ${PROJECT_SOURCE_DIR}/buildTools/versionInfo/*.h)
file(GLOB_RECURSE ZEUSDEBUG_BUILDTOOLS_VERSIONINFO_SOURCES_CPP ${PROJECT_SOURCE_DIR}/buildTools/versionInfo/*.cpp)

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

add_executable(zeusDebugVersionInfo ${ZEUSDEBUG_BUILDTOOLS_VERSIONINFO_HEADERS} ${ZEUSDEBUG_BUILDTOOLS_VERSIONINFO_SOURCES_CPP} "${PROJECT_BINARY_DIR}/include/version.cpp")

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

target_link_libraries(zeusDebugVersionInfo Qt5::Core)

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

# Targets, CPACK...
include(InstallRequiredSystemLibraries)

install(TARGETS zeusDebug RUNTIME DESTINATION bin COMPONENT ZeusUtilities)
install(TARGETS zeusDebugPlot RUNTIME DESTINATION bin COMPONENT ZeusUtilities)
install(TARGETS zeusDebugVersionInfo RUNTIME DESTINATION bin COMPONENT ZeusUtilities)

if (MSVC)
	# Custom target for packaging.
	if(ZEUSDEBUG_USE_NSIS)
		set(CPACK_GENERATOR "NSIS")
	else()
		set(CPACK_GENERATOR "ZIP")
	endif()

	set(ZEUSDEBUG_WINDEPLOYQT_EXE "${Qt5Core_DIR}/../../../bin/windeployqt.exe")
	
	if ("$ENV{VCINSTALLDIR}" STREQUAL "")
		get_filename_component(ZEUSDEBUG_VCINSTALLDIR "${CMAKE_LINKER}" DIRECTORY)
		file(TO_NATIVE_PATH "${ZEUSDEBUG_VCINSTALLDIR}/../../" VCINSTALLDIR_NATIVEPATH)
		
		STRING(REGEX REPLACE "^(.+)\\\\VC\\\\.+" "\\1\\\\VC" VCINSTALLDIR_NATIVEPATH ${VCINSTALLDIR_NATIVEPATH})
		
		set(ENV{VCINSTALLDIR} "${VCINSTALLDIR_NATIVEPATH}")
	endif()
	
	add_custom_command(TARGET zeusDebug POST_BUILD COMMAND "${CMAKE_SOURCE_DIR}/cmake/callwindeployqt.bat" "${ZEUSDEBUG_WINDEPLOYQT_EXE}" --dir "${PROJECT_BINARY_DIR}/windeployqt" "$<TARGET_FILE:zeusDebug>" "$ENV{VCINSTALLDIR}" COMMENT "Preparing Qt runtime dependencies")
	add_custom_command(TARGET zeusDebugPlot POST_BUILD COMMAND "${CMAKE_SOURCE_DIR}/cmake/callwindeployqt.bat" "${ZEUSDEBUG_WINDEPLOYQT_EXE}" --dir "${PROJECT_BINARY_DIR}/windeployqt" "$<TARGET_FILE:zeusDebugPlot>" "$ENV{VCINSTALLDIR}" COMMENT "Preparing Qt runtime dependencies")
	install(DIRECTORY "${PROJECT_BINARY_DIR}/windeployqt/" DESTINATION bin COMPONENT ZeusUtilities)
endif(MSVC)
   
if (NOT "${ZEUSDEBUG_ADDITIONAL_INSTALL_FILES}" STREQUAL "")
	foreach(ADD_INST_FILE IN LISTS ZEUSDEBUG_ADDITIONAL_INSTALL_FILES)
		if(IS_DIRECTORY "${ADD_INST_FILE}")
			message(STATUS "Also installing folder ${ADD_INST_FILE}")
			install(DIRECTORY "${ADD_INST_FILE}/" DESTINATION bin COMPONENT ZeusUtilities)
		else()
			message(STATUS "Also installing file ${ADD_INST_FILE}")
			install(FILES "${ADD_INST_FILE}" DESTINATION bin COMPONENT ZeusUtilities)
		endif()
	endforeach()
endif()

# CPack
set(CPACK_PACKAGE_NAME "zeusutilities")
set(CPACK_PACKAGE_VENDOR "ZeusUtilities project team")

set(CPACK_PACKAGE_VERSION_MAJOR "${ZEUSDEBUG_CPP_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${ZEUSDEBUG_CPP_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${ZEUSDEBUG_CPP_VERSION_PATCH}")

if(WIN32)
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "ZeusUtilities")
endif()

#set(CPACK_NSIS_INSTALLED_ICON_NAME "${PROJECT_SOURCE_DIR}/resources/icon-small-for-ico.ico")
set(CPACK_NSIS_HELP_LINK "https://github.com/blizzard4591/zeusUtilities/")
set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/blizzard4591/zeusUtilities/")
set(CPACK_NSIS_CONTACT "packaging@openmittsu.de")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(CPACK_NSIS_DISPLAY_NAME "ZeusUtilities x64")
else()
	set(CPACK_NSIS_DISPLAY_NAME "ZeusUtilities x86")
endif()

set(CPACK_PACKAGE_VERSION "${ZEUSDEBUG_CPP_VERSION_MAJOR}.${ZEUSDEBUG_CPP_VERSION_MINOR}.${ZEUSDEBUG_CPP_VERSION_PATCH}plus${ZEUSDEBUG_CPP_VERSION_COMMITS_AHEAD}")

SET(CPACK_PACKAGE_EXECUTABLES "zeusDebug;ZeusDebug;zeusDebugPlot;ZeusDebug Plotter")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
if (WIN32 AND NOT UNIX)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait '\\\"$INSTDIR\\\\bin\\\\vcredist_x64.exe\\\" /install /passive /norestart'")
	else()
		SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait '\\\"$INSTDIR\\\\bin\\\\vcredist_x86.exe\\\" /install /passive /norestart'")
	endif()
else(WIN32 AND NOT UNIX)
	SET(CPACK_STRIP_FILES ON)
	SET(CPACK_SOURCE_STRIP_FILES "")
endif(WIN32 AND NOT UNIX)
set(CPACK_SOURCE_IGNORE_FILES ".git")
set(CPACK_BUNDLE_NAME "zeusDebug")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL ZeusUtilities)

INCLUDE(CPack)