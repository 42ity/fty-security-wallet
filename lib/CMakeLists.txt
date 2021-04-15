cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

########################################################################################################################

#Create the target
etn_target(static ${PROJECT_NAME}-lib
    SOURCES
        src/*.cc
    PUBLIC_INCLUDE_DIR
        include
    PUBLIC
        *.h
    USES_PRIVATE
        #systemd #optional see HAVE_LIBSYSTEMD
        pthread
        ssl #>=1.1
        crypto
        protobuf
        czmq
        mlm
        cxxtools
        fty_common_logging
        fty_common_mlm
        fty_common_socket
        fty_common_messagebus
        fty_common_dto
        fty_lib_certificate
    PRIVATE
)

#see src/fty_security_wallet_classes.h (HAVE_LIBSYSTEMD)
#target_compile_definitions(${PROJECT_NAME}-lib PRIVATE HAVE_LIBSYSTEMD)

if (BUILD_TESTING)

    etn_test(${PROJECT_NAME}-test
        SOURCES
            tests/main.cc
            src/*.cc
        INCLUDE_DIR
            include
        PREPROCESSOR -DCATCH_CONFIG_FAST_COMPILE
        USES
            #systemd #optional see HAVE_LIBSYSTEMD
            pthread
            ssl #>=1.1
            crypto
            protobuf
            czmq
            mlm
            cxxtools
            fty_common_logging
            fty_common_mlm
            fty_common_socket
            fty_common_messagebus
            fty_common_dto
            fty_lib_certificate
            Catch2::Catch2
            stdc++fs
            log4cplus
    )

    #see src/fty_security_wallet_classes.h (HAVE_LIBSYSTEMD)
    #target_compile_definitions(${PROJECT_NAME}-test PRIVATE HAVE_LIBSYSTEMD)

    #copy selftest-ro, build selftest-rw for test in/out
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/selftest-ro DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/selftest-rw)

    #enable coverage
    etn_coverage(${PROJECT_NAME}-test)

    #copy selftest-ro, build selftest-rw for test in/out
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/selftest-ro DESTINATION ${CMAKE_BINARY_DIR})
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/selftest-rw)

endif()
