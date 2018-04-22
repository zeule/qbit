cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

project(qbit
     VERSION 0.0.1.0
)

include(GNUInstallDirs)

if(WIN32)
    include(winconf)
endif(WIN32)

# we need options here, because they are used not only in "src" subdir, but in the "dist" dir too
include(CMakeDependentOption)

option(SYSTEM_QTSINGLEAPPLICATION
    "Use the system qtsingleapplication library or shipped one otherwise" ON)

option(GUI "Allows to disable GUI for headless running. Disables QtDBus and the GeoIP Database" ON)

option(WEBUI "Allows to disable the WebUI." ON)

option(STACKTRACE "Enable stacktrace feature" ON)

if (UNIX)
    cmake_dependent_option(SYSTEMD "Install the systemd service file (headless only)" OFF
        "NOT GUI" OFF)
    cmake_dependent_option(DBUS "Enable use of QtDBus (GUI only)" ON "GUI" OFF)
endif(UNIX)

add_subdirectory(src)
add_subdirectory(dist)
