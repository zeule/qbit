cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

project(qbit
     VERSION 0.0.1.0
)

include(GNUInstallDirs)
include(FeatureSummary)

# version requirements
set(requiredBoostVersion 1.56)
set(requiredQtVersion 5.9.0)
set(requiredOpensslVersion 1.0)
set(requiredLibtorrentVersion 1.2.0)

if(WIN32)
    include(winconf)
endif(WIN32)

macro(feature_option _name _description _default)
    option(${_name} "${_description}" ${_default})
    add_feature_info(${_name} ${_name} "${_description}")
endmacro()

# we need options here, at the top level, because they are used not only in "src" subdir, but in the "dist" dir too
include(CMakeDependentOption)

feature_option(COUNTRIES_RESOLUTION "Enable resolving peers IP addresses to countries" ON)
feature_option(STACKTRACE "Enable stacktraces" ON)
feature_option(WEBUI "Enables built-in HTTP server for headless use" ON)

if (UNIX)
    option(PLASMA_INTEGRATION "Enable KDE/Plasma integration" ON)
endif(UNIX)

add_subdirectory(src)
add_subdirectory(dist)

feature_summary(DESCRIPTION "\nConfiguration results:" WHAT ALL)
