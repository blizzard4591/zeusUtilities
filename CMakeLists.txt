cmake_minimum_required (VERSION 2.8.12)

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


# Add base folder for better inclusion paths
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")


if(CMAKE_COMPILER_IS_GNUCC)
    # Set standard flags for GCC
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -funroll-loops")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic")
elseif(MSVC)
	add_definitions(/DNOMINMAX)
	
	# Disable "C:\Program Files (x86)\Windows Kits\8.1\Include\um\DbgHelp.h(1544): warning C4091: 'typedef ': ignored on left of '' when no variable is declared"
	add_definitions(/wd4091)
	
	# For the Windows header mess
	add_definitions(/D_WINSOCKAPI_)
else(CLANG)
	# As CLANG is not set as a variable, we need to set it in case we have not matched another compiler.
	set (CLANG ON)
    # Set standard flags for clang
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -funroll-loops -O3")
    if(UNIX AND NOT APPLE AND NOT OPENMITTSU_USE_LIBCXX)
		set(CLANG_STDLIB libstdc++)
    else()
		set(CLANG_STDLIB libc++)
		# Set up some Xcode specific settings
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
		set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    endif()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=${CLANG_STDLIB} -Wall -pedantic -Wno-unused-variable -ftemplate-depth=1024")
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
endif()

