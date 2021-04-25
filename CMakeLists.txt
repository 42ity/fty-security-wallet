cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

##############################################################################################################

project(fty-security-wallet VERSION 1.0.0)

#WA for library name
set(PROJECT_NAME_UNDERSCORE fty_security_wallet)

########################################################################################################################

find_package(fty-cmake PATHS ${CMAKE_BINARY_DIR})

########################################################################################################################

add_subdirectory(lib)
add_subdirectory(agent)

## agent configuration
## https://cmake.org/cmake/help/v3.0/module/GNUInstallDirs.html

set(AGENT_USER "secw-daemon")
set(AGENT_VAR_DIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/fty/${PROJECT_NAME}")
set(AGENT_ETC_DIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME}")
set(AGENT_ETC_FTY_DIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}/fty/${PROJECT_NAME}")
set(AGENT_SHARE_BIOS_SETUP_DIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/bios/setup")

set(AGENT_socketSecurityWallet "/run/${PROJECT_NAME}/secw.socket")
set(AGENT_systemd_service_type "notify")

set(res "resources")

# 30-secw-update-database-right.sh -> bios/setup
set(file "30-secw-update-database-right.sh")
configure_file("${PROJECT_SOURCE_DIR}/${res}/${file}.in" "${PROJECT_BINARY_DIR}/${res}/${file}" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/${res}/${file}" DESTINATION ${AGENT_SHARE_BIOS_SETUP_DIR})

# configuration.json -> etc/fty
set(file "configuration.json")
configure_file("${PROJECT_SOURCE_DIR}/${res}/${file}" "${PROJECT_BINARY_DIR}/${res}/${file}" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/${res}/${file}" DESTINATION ${AGENT_ETC_FTY_DIR})

# .conf file -> lib/tmpfiles.d/
set(file "${PROJECT_NAME}.conf")
configure_file("${PROJECT_SOURCE_DIR}/${res}/${file}.in" "${PROJECT_BINARY_DIR}/${res}/${file}" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/${res}/${file}" DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/tmpfiles.d/")

# cfg file -> etc/
set(file "${PROJECT_NAME}.cfg")
configure_file("${PROJECT_SOURCE_DIR}/${res}/${file}.in" "${PROJECT_BINARY_DIR}/${res}/${file}" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/${res}/${file}" DESTINATION ${AGENT_ETC_DIR})

# service -> lib/systemd/system
set(file "${PROJECT_NAME}.service")
configure_file("${PROJECT_SOURCE_DIR}/${res}/${file}.in" "${PROJECT_BINARY_DIR}/${res}/${file}" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/${res}/${file}" DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system/)
