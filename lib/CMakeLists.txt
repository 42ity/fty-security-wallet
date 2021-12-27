cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

########################################################################################################################

#Create the target
etn_target(shared ${PROJECT_NAME_UNDERSCORE}
    SOURCES
        src/cam_helpers.cc
        src/cam_helpers.h
        src/cam_accessor.cc
        src/secw_configuration.cc
        include/secw_configuration.h
        src/secw_security_wallet.cc
        include/secw_security_wallet.h
        src/secw_security_wallet_server.cc
        include/secw_security_wallet_server.h
        src/fty_security_wallet_mlm_agent.cc
        src/fty_security_wallet_mlm_agent.h
        src/cam_credential_asset_mapping_storage.cc
        src/cam_credential_asset_mapping_storage.h
        src/secw_helpers.cc
        src/secw_helpers.h
        src/cam_exception.cc
        src/secw_exception.cc
        src/secw_portfolio.cc
        src/cam_credential_asset_mapping.cc
        src/secw_consumer_accessor.cc
        include/secw_portfolio.h
        src/cam_credential_asset_mapping_server.cc
        src/secw_client_accessor.cc
        src/secw_document.cc
        src/secw_internal_certificate.cc
        src/secw_producer_accessor.cc
        src/secw_snmpv1.cc
        src/cam_credential_asset_mapping_server.h
        include/secw_client_accessor.h
        src/secw_openssl_wrapper.cc
        src/secw_snmpv3.cc
        src/fty_credential_asset_mapping_mlm_agent.cc
        src/secw_external_certificate.cc
        src/secw_openssl_wrapper.h
        src/secw_user_and_password.cc
    PUBLIC_INCLUDE_DIR
        include
    PUBLIC
        cam_accessor.h
        cam_credential_asset_mapping.h
        cam_exception.h
        fty_credential_asset_mapping_mlm_agent.h
        fty_security_wallet.h
        fty_security_wallet_socket_agent.h
        secw_consumer_accessor.h
        secw_document.h
        secw_exception.h
        secw_external_certificate.h
        secw_internal_certificate.h
        secw_producer_accessor.h
        secw_snmpv1.h
        secw_snmpv3.h
        secw_user_and_password.h
    USES
        pthread
        ssl #>=1.1
        crypto
        protobuf
        czmq
        mlm
        cxxtools
        fty_common
        fty_common_logging
        fty_common_mlm
        fty_common_socket
        fty_common_messagebus
        fty_common_dto
        fty_lib_certificate
)

set_target_properties(${PROJECT_NAME_UNDERSCORE} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

etn_test_target(${PROJECT_NAME_UNDERSCORE}
    CONFIGS
        tests/selftest-ro/configuration.json
        tests/selftest-ro/data.json
        tests/selftest-ro/mapping.json
    SOURCES
        tests/main.cc
        tests/cam_accessor.cpp
        tests/cam_accessor.h
        tests/asset_mapping_mlm_agent.cpp
        tests/wallet_mlm_agent.cpp
        tests/socket_agent.cpp
        tests/consumer_accessor.cpp
        tests/consumer_accessor.h
        tests/producer_accessor.cpp
        tests/producer_accessor.h
    INCLUDE_DIR
        include
        src
    PREPROCESSOR -DCATCH_CONFIG_FAST_COMPILE
    USES
        pthread
    SUBDIR
        tests
)

if (BUILD_TESTING)
    etn_target(shared ${PROJECT_NAME_UNDERSCORE}_server
        SOURCES
            src/secw_openssl_wrapper.cc
            src/secw_openssl_wrapper.h
            src/secw_helpers.cc
            src/secw_helpers.h
            src/cam_exception.cc
            src/cam_helpers.cc
            src/cam_helpers.h
            src/secw_snmpv1.cc
            src/secw_snmpv3.cc
            src/secw_exception.cc
            src/secw_client_accessor.cc
            src/secw_portfolio.cc
            src/secw_document.cc
            src/secw_configuration.cc
            src/secw_security_wallet.cc
            src/secw_user_and_password.cc
            src/secw_external_certificate.cc
            src/secw_internal_certificate.cc
            src/secw_security_wallet_server.cc
        PUBLIC_INCLUDE_DIR
            include
        PUBLIC
            cam_exception.h
            secw_snmpv1.h
            secw_snmpv3.h
            secw_exception.h
            secw_client_accessor.h
            secw_portfolio.h
            secw_document.h
            secw_configuration.h
            secw_security_wallet.h
            secw_user_and_password.h
            secw_external_certificate.h
            secw_internal_certificate.h
            secw_security_wallet_server.h
        USES
            pthread
            ssl #>=1.1
            crypto
            protobuf
            czmq
            mlm
            cxxtools
            fty_common
            fty_common_logging
            fty_common_mlm
            fty_common_socket
            fty_common_messagebus
            fty_common_dto
            fty_lib_certificate
    )

    set_target_properties(${PROJECT_NAME_UNDERSCORE}_server PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
endif()