cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

##############################################################################################################

project(fty-security-wallet VERSION 1.0.0)

########################################################################################################################

find_package(fty-cmake PATHS ${CMAKE_BINARY_DIR})

########################################################################################################################

add_subdirectory(lib)
add_subdirectory(agent)

if (__xOx__)

  ## agent configuration
  ## https://cmake.org/cmake/help/v3.0/module/GNUInstallDirs.html

  set(AGENT_USER "bios")
  set(AGENT_VAR_DIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/fty/${PROJECT_NAME}")

  # var/lib/fty runtime directory
  install(DIRECTORY DESTINATION ${AGENT_VAR_DIR})

  # service -> lib/systemd/system
  configure_file("${PROJECT_SOURCE_DIR}/resources/fty-alert-list.service.in" "${PROJECT_BINARY_DIR}/resources/fty-alert-list.service" @ONLY)
  install(FILES "${PROJECT_BINARY_DIR}/resources/fty-alert-list.service" DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system/)

endif()
