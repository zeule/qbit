cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

project(qbit
     VERSION 0.0.1.0
)

include(GNUInstallDirs)
include(FeatureSummary)

if(WIN32)
    include(winconf)
endif(WIN32)

macro(feature_option _name _description _default)
    option(${_name} "${_description}" ${_default})
    add_feature_info(${_name} ${_name} "${_description}")
endmacro()

# we need options here, because they are used not only in "src" subdir, but in the "dist" dir too
include(CMakeDependentOption)

feature_option(WEBUI "Enables built-in HTTP server for headless use" ON)
feature_option(STACKTRACE "Enable stacktraces" ON)

if (UNIX)
    option(PLASMA_INTEGRATION "Enable KDE/Plasma integration" ON)
    feature_option(SYSTEMD "Install the systemd service file (headless only)" OFF)
endif(UNIX)

add_subdirectory(src)
add_subdirectory(dist)

feature_summary(DESCRIPTION "\nConfiguration results:" WHAT ALL)
