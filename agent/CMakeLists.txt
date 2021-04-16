cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

########################################################################################################################

#Create the main target
etn_target(exe ${PROJECT_NAME}
    SOURCES
        src/fty_security_wallet.cc
    USES
        czmq
        mlm
        fty_common_mlm
        fty_common_socket
        fty_common_logging
    USES_PRIVATE
        ${PROJECT_NAME}-lib
)

#link with systemd (optional)
target_link_libraries(${PROJECT_NAME} LINK_PRIVATE systemd)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_LIBSYSTEMD)
