cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

########################################################################################################################

#Create the main target
etn_target(exe ${PROJECT_NAME}
    SOURCES
        src/fty_security_wallet.cc
    USES
        #systemd #optional see HAVE_LIBSYSTEMD
        czmq
        mlm
        fty_common_mlm
        fty_common_socket
        fty_common_logging
    USES_PRIVATE
        ${PROJECT_NAME}-lib
)

#see src/fty_security_wallet_classes.h (HAVE_LIBSYSTEMD)
#target_compile_definitions(${PROJECT_NAME}-lib PRIVATE HAVE_LIBSYSTEMD)
