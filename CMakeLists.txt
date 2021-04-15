cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

##############################################################################################################

project(fty-security-wallet VERSION 1.0.0)

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
#set(AGENT_SHARE_DIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/${PROJECT_NAME}")
set(AGENT_SHARE_BIOS_SETUP_DIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/bios/setup")

set(AGENT_socketSecurityWallet "/run/${PROJECT_NAME}/secw.socket")
set(AGENT_systemd_service_type "simple")  #ZZZ

# var/lib/fty runtime directory
install(DIRECTORY DESTINATION ${AGENT_VAR_DIR})

# 30-secw-update-database-right.sh -> bios/setup
set(file "30-secw-update-database-right.sh")
configure_file("${PROJECT_SOURCE_DIR}/resources/${file}.in" "${PROJECT_BINARY_DIR}/resources/${file}" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/resources/${file}" DESTINATION ${AGENT_SHARE_BIOS_SETUP_DIR})

# configuration.json -> etc/fty
set(file "configuration.json")
configure_file("${PROJECT_SOURCE_DIR}/resources/${file}" "${PROJECT_BINARY_DIR}/resources/${file}" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/resources/${file}" DESTINATION ${AGENT_ETC_FTY_DIR})

# cfg file -> etc/
set(file "${PROJECT_NAME}.cfg")
configure_file("${PROJECT_SOURCE_DIR}/resources/${file}.in" "${PROJECT_BINARY_DIR}/resources/${file}" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/resources/${file}" DESTINATION ${AGENT_ETC_DIR})

# service -> lib/systemd/system
set(file "${PROJECT_NAME}.service")
configure_file("${PROJECT_SOURCE_DIR}/resources/${file}.in" "${PROJECT_BINARY_DIR}/resources/${file}" @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/resources/${file}" DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system/)

