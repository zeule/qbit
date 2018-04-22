set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD "11")
add_definitions(-DBOOST_NO_CXX11_RVALUE_REFERENCES)

include(MacroQbtCompilerSettings)
qbt_set_compiler_options()

include(CheckIncludeFileCXX)
include(MacroLinkQtComponents)
include(QbtTargetSources)

find_package(Boost 1.56 REQUIRED)
find_package(LibtorrentRasterbar REQUIRED)

set(CMAKE_REQUIRED_INCLUDES ${LibtorrentRasterbar_INCLUDE_DIR})
check_include_file_cxx(libtorrent/string_view.hpp HAVE_LIBTORRENT_STRING_VIEW_HPP)

# Qt
list(APPEND QBT_QT_COMPONENTS Core Network Xml)
if (GUI)
    list (APPEND QBT_QT_COMPONENTS Gui Svg Widgets)
    if (WIN32)
        list (APPEND QBT_QT_COMPONENTS WinExtras)
    endif(WIN32)
    if (APPLE)
        list (APPEND QBT_GUI_OPTIONAL_LINK_LIBRARIES objc)
        list (APPEND QBT_QT_COMPONENTS MacExtras)
    endif (APPLE)
endif (GUI)
if (DBUS)
    list (APPEND QBT_QT_COMPONENTS DBus)
endif (DBUS)
find_package(Qt5 5.9 COMPONENTS ${QBT_QT_COMPONENTS} REQUIRED)

set(CMAKE_AUTOMOC True)
list(APPEND CMAKE_AUTORCC_OPTIONS -compress 9 -threshold 5)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# defines
add_definitions(-DQT_NO_CAST_TO_ASCII)
# Efficient construction for QString & QByteArray (Qt >= 4.8)
add_definitions(-DQT_USE_QSTRINGBUILDER)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wformat -Wformat-security)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Project is built in DEBUG mode.")
else()
    message(STATUS "Project is built in RELEASE mode.")
    message(STATUS "Disabling debug output.")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

set(QBT_USE_GUI ${GUI})
set(QBT_USE_WEBUI ${WEBUI})

configure_file(config.h.cmakein ${CMAKE_CURRENT_BINARY_DIR}/config.h)

if (SYSTEM_QTSINGLEAPPLICATION)
    find_package(QtSingleApplication REQUIRED)
else (SYSTEM_QTSINGLEAPPLICATION)
    add_subdirectory(app/qtsingleapplication)
endif (SYSTEM_QTSINGLEAPPLICATION)

add_subdirectory(app)
add_subdirectory(base)

if (GUI)
    add_subdirectory(gui)
endif (GUI)

if (WEBUI)
    add_subdirectory(webui)
endif (WEBUI)

