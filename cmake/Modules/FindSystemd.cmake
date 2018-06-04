#######
# Find systemd service dir
# sets variables
# Systemd_FOUND
# Systemd_SERVICES_INSTALL_DIR

find_package(PkgConfig QUIET REQUIRED)

if (NOT Systemd_FOUND)
    pkg_check_modules(Systemd "systemd")
endif(NOT Systemd_FOUND)

if (Systemd_FOUND AND "${Systemd_SERVICES_INSTALL_DIR}" STREQUAL "")
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE}
        --variable=systemdsystemunitdir systemd
        OUTPUT_VARIABLE Systemd_SERVICES_INSTALL_DIR)
    string(REGEX REPLACE "[ \t\n]+" "" Systemd_SERVICES_INSTALL_DIR
        "${Systemd_SERVICES_INSTALL_DIR}")
elseif (NOT Systemd_FOUND AND Systemd_SERVICES_INSTALL_DIR)
    message (FATAL_ERROR "Variable Systemd_SERVICES_INSTALL_DIR is\
        defined, but we can't find systemd using pkg-config")
endif()

if (Systemd_FOUND)
    message(STATUS "systemd services install dir: ${Systemd_SERVICES_INSTALL_DIR}")
endif(Systemd_FOUND)
