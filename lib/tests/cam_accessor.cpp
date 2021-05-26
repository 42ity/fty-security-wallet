#include "cam_accessor.h"
#include "fty_security_wallet.h"
#include <catch2/catch.hpp>

void camAccessorTest(mlm::MlmSyncClient& syncClient)
{
    // test 1.X
    {
        // test 1.1 => test retrieve a mapping
        {
            try {
                cam::Accessor accessor(syncClient);

                cam::AssetId   assetId("asset-1");
                cam::ServiceId serviceId("test-usage");
                cam::Protocol  protocol("http");

                const cam::CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

                if (mapping.m_assetId != assetId) {
                    throw std::runtime_error(
                        "Wrong asset id. Received '" + mapping.m_assetId + "' expected '" + assetId + "'");
                }

                if (mapping.m_serviceId != serviceId) {
                    throw std::runtime_error(
                        "Wrong usage id. Received '" + mapping.m_serviceId + "' expected '" + serviceId + "'");
                }

                if (mapping.m_protocol != protocol) {
                    throw std::runtime_error(
                        "Wrong protocol. Received '" + mapping.m_protocol + "' expected '" + protocol + "'");
                }

                if (mapping.m_port != "80") {
                    throw std::runtime_error("Wrong port. Received '" + mapping.m_port + "' expected '80'");
                }

                if (mapping.m_credentialId != "cred-1") {
                    throw std::runtime_error(
                        "Wrong credential id. Received '" + mapping.m_credentialId + "' expected 'cred-1'");
                }


                if (mapping.m_status != cam::Status::UNKNOWN) {
                    throw std::runtime_error("Wrong credential status");
                }

                if ((mapping.m_extendedInfo.size() != 1) || (mapping.m_extendedInfo.at("port") != "80")) {
                    throw std::runtime_error("Wrong extended info");
                }

            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 1.2 => test exception CamMappingDoesNotExistException
        {
            cam::AssetId   assetId("asset-XXXXX");
            cam::ServiceId serviceId("usage-XXXXX");
            cam::Protocol  protocol("protocol-XXXXX");

            try {
                cam::Accessor                     accessor(syncClient);
                const cam::CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);
                throw std::runtime_error("Mapping is returned");
            } catch (const cam::CamMappingDoesNotExistException& e) {
                CHECK(e.getAssetId() == assetId);
                CHECK(e.getServiceId() == serviceId);
                CHECK(e.getProtocol() == protocol);
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }
    } // 1.X

    // test 2.X
    {
        cam::AssetId      assetId("asset-2");
        cam::ServiceId    serviceId("test-usage-2");
        cam::Protocol     protocol("test-proto");
        cam::Port         port("80");
        cam::CredentialId credId("Test-mapping");
        cam::Status       status(cam::Status::VALID);

        std::string     key("key");
        std::string     data("data");
        cam::MapExtendedInfo extendedInfo;
        extendedInfo[key] = data;

        // test 2.1 => test create
        {
            try {
                cam::Accessor accessor(syncClient);
                accessor.createMapping(assetId, serviceId, protocol, port, credId, status, extendedInfo);
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 2.2 => test retrieve a the created mapping
        {
            try {
                cam::Accessor accessor(syncClient);

                const cam::CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

                if (mapping.m_assetId != assetId) {
                    throw std::runtime_error(
                        "Wrong asset id. Received '" + mapping.m_assetId + "' expected '" + assetId + "'");
                }

                if (mapping.m_serviceId != serviceId) {
                    throw std::runtime_error(
                        "Wrong usage id. Received '" + mapping.m_serviceId + "' expected '" + serviceId + "'");
                }

                if (mapping.m_credentialId != credId) {
                    throw std::runtime_error(
                        "Wrong credential id. Received '" + mapping.m_credentialId + "' expected '" + credId + "'");
                }

                if (mapping.m_status != status) {
                    throw std::runtime_error("Wrong credential status");
                }

                if (mapping.m_protocol != protocol) {
                    throw std::runtime_error(
                        "Wrong protocol. Received '" + mapping.m_protocol + "' expected '" + protocol + "'");
                }

                if (mapping.m_port != "80") {
                    throw std::runtime_error("Wrong port. Received '" + mapping.m_port + "' expected '80'");
                }

                if ((mapping.m_extendedInfo.size() != extendedInfo.size()) ||
                    (mapping.m_extendedInfo.at(key) != data)) {
                    throw std::runtime_error("Wrong extended info");
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 2.3 => test exception CamMappingAlreadyExistsException
        {
            try {
                cam::Accessor accessor(syncClient);
                accessor.createMapping(assetId, serviceId, protocol, port, credId, status, extendedInfo);
                throw std::runtime_error("Mapping is created");
            } catch (const cam::CamMappingAlreadyExistsException& e) {
                CHECK(e.getAssetId() == assetId);
                CHECK(e.getServiceId() == serviceId);
                CHECK(e.getProtocol() == protocol);
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }
    } // 2.X

    // test 3.X -> need 2.X success
    {

        cam::AssetId   assetId("asset-2");
        cam::ServiceId serviceId("test-usage-2");
        cam::Protocol  protocol("test-proto");

        // test 3.1 => test remove
        {
            try {
                cam::Accessor accessor(syncClient);
                accessor.removeMapping(assetId, serviceId, protocol);
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 3.2 => test exception CamMappingDoesNotExistException
        {
            try {
                cam::Accessor                     accessor(syncClient);
                const cam::CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);
                throw std::runtime_error("Mapping is returned");
            } catch (const cam::CamMappingDoesNotExistException& e) {
                CHECK(e.getAssetId() == assetId);
                CHECK(e.getServiceId() == serviceId);
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 3.3 => test exception CamMappingDoesNotExistException
        {
            try {
                cam::Accessor accessor(syncClient);
                accessor.removeMapping(assetId, serviceId, protocol);
                throw std::runtime_error("Mapping is removed");
            } catch (const cam::CamMappingDoesNotExistException& e) {
                CHECK(e.getAssetId() == assetId);
                CHECK(e.getServiceId() == serviceId);
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }
    } // 3.X

    // test 4.X -> need 2.X success
    {
        cam::AssetId      assetId("asset-4");
        cam::ServiceId    serviceId("test-usage-4");
        cam::Protocol     protocol("http");
        cam::Port         port("80");
        cam::CredentialId credId("Test-mapping-update");
        cam::Status       status(cam::Status::VALID);

        std::string     key("key");
        std::string     data("data");
        cam::MapExtendedInfo extendedInfo;
        extendedInfo[key] = data;

        // test 4.1 => test create
        {
            try {
                cam::Accessor accessor(syncClient);
                accessor.createMapping(assetId, serviceId, protocol, port, credId, status, extendedInfo);
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 4.2 => test update mapping
        {
            try {
                cam::Accessor accessor(syncClient);
                credId = "new_cred";
                accessor.updateCredentialId(assetId, serviceId, protocol, credId);
                const cam::CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

                if (mapping.m_assetId != assetId) {
                    throw std::runtime_error(
                        "Wrong asset id. Received '" + mapping.m_assetId + "' expected '" + assetId + "'");
                }

                if (mapping.m_serviceId != serviceId) {
                    throw std::runtime_error(
                        "Wrong usage id. Received '" + mapping.m_serviceId + "' expected '" + serviceId + "'");
                }

                if (mapping.m_credentialId != credId) {
                    throw std::runtime_error(
                        "Wrong credential id. Received '" + mapping.m_credentialId + "' expected '" + credId + "'");
                }

                if (mapping.m_protocol != protocol) {
                    throw std::runtime_error(
                        "Wrong protocol. Received '" + mapping.m_protocol + "' expected '" + protocol + "'");
                }

                if (mapping.m_port != port) {
                    throw std::runtime_error("Wrong port. Received '" + mapping.m_port + "' expected '" + port + "'");
                }

                if (mapping.m_status != cam::Status::UNKNOWN) {
                    throw std::runtime_error("Wrong credential status");
                }

                if ((mapping.m_extendedInfo.size() != extendedInfo.size()) ||
                    (mapping.m_extendedInfo.at(key) != data)) {
                    throw std::runtime_error("Wrong extended info");
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 4.3 => test exception CamMappingDoesNotExistException
        {
            try {
                cam::Accessor accessor(syncClient);
                accessor.updateCredentialId("XXXXXX", "XXXXXX", "XXXXX", credId);

                throw std::runtime_error("Mapping is updated");
            } catch (const cam::CamMappingDoesNotExistException&) {
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 4.4 => test update mapping
        {
            try {
                cam::Accessor accessor(syncClient);

                status = cam::Status::ERROR;

                accessor.updateStatus(assetId, serviceId, protocol, status);

                const cam::CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

                if (mapping.m_assetId != assetId) {
                    throw std::runtime_error(
                        "Wrong asset id. Received '" + mapping.m_assetId + "' expected '" + assetId + "'");
                }

                if (mapping.m_serviceId != serviceId) {
                    throw std::runtime_error(
                        "Wrong usage id. Received '" + mapping.m_serviceId + "' expected '" + serviceId + "'");
                }

                if (mapping.m_protocol != protocol) {
                    throw std::runtime_error(
                        "Wrong protocol. Received '" + mapping.m_protocol + "' expected '" + protocol + "'");
                }

                if (mapping.m_port != port) {
                    throw std::runtime_error("Wrong port. Received '" + mapping.m_port + "' expected '" + port + "'");
                }

                if (mapping.m_credentialId != credId) {
                    throw std::runtime_error(
                        "Wrong credential id. Received '" + mapping.m_credentialId + "' expected '" + credId + "'");
                }

                if (mapping.m_status != status) {
                    throw std::runtime_error("Wrong credential status");
                }

                if ((mapping.m_extendedInfo.size() != extendedInfo.size()) ||
                    (mapping.m_extendedInfo.at(key) != data)) {
                    throw std::runtime_error("Wrong extended info");
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 4.5 => test exception CamMappingDoesNotExistException
        {
            try {
                cam::Accessor accessor(syncClient);
                accessor.updateStatus("XXXXXX", "XXXXXX", "XXXX", status);

                throw std::runtime_error("Mapping is updated");
            } catch (const cam::CamMappingDoesNotExistException&) {
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 4.6 => test update mapping
        {
            try {
                cam::Accessor accessor(syncClient);

                data = "45";
                std::string extraKey("update-key");
                std::string extraData("update-data");

                extendedInfo[key]      = data;
                extendedInfo[extraKey] = extraData;

                accessor.updateExtendedInfo(assetId, serviceId, protocol, extendedInfo);

                const cam::CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

                if (mapping.m_assetId != assetId) {
                    throw std::runtime_error(
                        "Wrong asset id. Received '" + mapping.m_assetId + "' expected '" + assetId + "'");
                }

                if (mapping.m_serviceId != serviceId) {
                    throw std::runtime_error(
                        "Wrong usage id. Received '" + mapping.m_serviceId + "' expected '" + serviceId + "'");
                }

                if (mapping.m_protocol != protocol) {
                    throw std::runtime_error(
                        "Wrong protocol. Received '" + mapping.m_protocol + "' expected '" + protocol + "'");
                }

                if (mapping.m_port != port) {
                    throw std::runtime_error("Wrong port. Received '" + mapping.m_port + "' expected '" + port + "'");
                }

                if (mapping.m_credentialId != credId) {
                    throw std::runtime_error(
                        "Wrong credential id. Received '" + mapping.m_credentialId + "' expected '" + credId + "'");
                }

                if (mapping.m_status != status) {
                    throw std::runtime_error("Wrong credential status");
                }

                if ((mapping.m_extendedInfo.size() != extendedInfo.size()) ||
                    (mapping.m_extendedInfo.at(key) != data) || mapping.m_extendedInfo.at(extraKey) != extraData) {
                    throw std::runtime_error("Wrong extended info");
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 4.7 => test exception CamMappingDoesNotExistException
        {
            try {
                cam::Accessor accessor(syncClient);
                accessor.updateExtendedInfo("XXXXXX", "XXXXXX", "XXXXXX", extendedInfo);

                throw std::runtime_error("Mapping is updated");
            } catch (const cam::CamMappingDoesNotExistException&) {
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 4.8 => test update mapping
        {
            try {
                cam::Accessor accessor(syncClient);
                port = "443";
                accessor.updatePort(assetId, serviceId, protocol, port);
                const cam::CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

                if (mapping.m_assetId != assetId) {
                    throw std::runtime_error(
                        "Wrong asset id. Received '" + mapping.m_assetId + "' expected '" + assetId + "'");
                }

                if (mapping.m_serviceId != serviceId) {
                    throw std::runtime_error(
                        "Wrong usage id. Received '" + mapping.m_serviceId + "' expected '" + serviceId + "'");
                }

                if (mapping.m_protocol != protocol) {
                    throw std::runtime_error(
                        "Wrong protocol. Received '" + mapping.m_protocol + "' expected '" + protocol + "'");
                }

                if (mapping.m_port != port) {
                    throw std::runtime_error("Wrong port. Received '" + mapping.m_port + "' expected '" + port + "'");
                }

                if (mapping.m_credentialId != credId) {
                    throw std::runtime_error(
                        "Wrong credential id. Received '" + mapping.m_credentialId + "' expected '" + credId + "'");
                }

                if (mapping.m_status != cam::Status::UNKNOWN) {
                    throw std::runtime_error("Wrong credential status");
                }

                if ((mapping.m_extendedInfo.size() != extendedInfo.size()) ||
                    (mapping.m_extendedInfo.at(key) != data)) {
                    throw std::runtime_error("Wrong extended info");
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 4.9 => test exception CamMappingDoesNotExistException
        {
            try {
                cam::Accessor accessor(syncClient);
                accessor.updatePort("XXXXXX", "XXXXXX", "XXXX", port);

                throw std::runtime_error("Mapping is updated");
            } catch (const cam::CamMappingDoesNotExistException&) {
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 4.10 => test remove
        {
            try {
                cam::Accessor accessor(syncClient);
                accessor.removeMapping(assetId, serviceId, protocol);
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }
    } // 4.X

    // test 5.X
    {
        // test 5.1
        {
            try {
                cam::Accessor accessor(syncClient);

                cam::AssetId   assetId("asset-1");
                cam::ServiceId serviceId("test-usage");
                cam::Protocol  protocol("http");

                if (!accessor.isMappingExisting(assetId, serviceId, protocol)) {
                    throw std::runtime_error("Mapping does not exist");
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 5.2
        {
            try {
                cam::Accessor accessor(syncClient);

                cam::AssetId   assetId("asset-XXXXX");
                cam::ServiceId serviceId("usage-XXXXX");
                cam::Protocol  protocol("protocol-XXXXXX");


                if (accessor.isMappingExisting(assetId, serviceId, protocol)) {
                    throw std::runtime_error("Mapping exists");
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }
    } // 5.X


    // test 6.X
    {
        // test 6.1
        {
            try {
                cam::Accessor accessor(syncClient);
                cam::AssetId assetId("assetA");
                const std::vector<cam::CredentialAssetMapping> mappings = accessor.getAssetMappings(assetId);

                if (mappings.size() != 3) {
                    throw std::runtime_error("Expected 3 mappings received " + std::to_string(mappings.size()));
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 6.2
        {
            try {
                cam::Accessor accessor(syncClient);

                cam::AssetId assetId("XXXXXX");

                const std::vector<cam::CredentialAssetMapping> mappings = accessor.getAssetMappings(assetId);

                if (mappings.size() != 0) {
                    throw std::runtime_error("Expected 0 mapping received " + std::to_string(mappings.size()));
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }
    } // 6.X

    // test 7.X
    {
        // test 7.1
        {
            try {
                cam::Accessor accessor(syncClient);

                cam::CredentialId credId("credC");

                const std::vector<cam::CredentialAssetMapping> mappings = accessor.getCredentialMappings(credId);

                if (mappings.size() != 3) {
                    throw std::runtime_error("Expected 3 mappings received " + std::to_string(mappings.size()));
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 7.2
        {
            try {
                cam::Accessor accessor(syncClient);

                cam::CredentialId credId("XXXXXX");

                const std::vector<cam::CredentialAssetMapping> mappings = accessor.getCredentialMappings(credId);

                if (mappings.size() != 0) {
                    throw std::runtime_error("Expected 0 mapping received " + std::to_string(mappings.size()));
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }
    } // 7.X

    // test 8.X
    {
        // test 8.1
        {
            try {
                cam::Accessor accessor(syncClient);

                cam::CredentialId credId("credC");

                uint32_t counter = accessor.countCredentialMappingsForCredential(credId);

                if (counter != 3) {
                    throw std::runtime_error("Expected 3 mappings received " + std::to_string(counter));
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 8.2
        {
            try {
                cam::Accessor accessor(syncClient);

                cam::CredentialId credId("XXXXXX");

                uint32_t counter = accessor.countCredentialMappingsForCredential(credId);

                if (counter != 0) {
                    throw std::runtime_error("Expected 0 mapping received " + std::to_string(counter));
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }
    } // 8.X

    // test 9.X
    {
        // test 9.1
        {
            try {
                cam::Accessor accessor(syncClient);

                const std::map<cam::CredentialId, uint32_t> counters = accessor.getAllCredentialCounter();

                cam::CredentialId credId("credC");

                if (counters.count(credId) == 0) {
                    throw std::runtime_error("No mapping counted for " + credId);
                }

                if (counters.at(credId) != 3) {
                    throw std::runtime_error("Expected 3 mappings received " + std::to_string(counters.at(credId)));
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 9.2
        {
            try {
                cam::Accessor accessor(syncClient);

                cam::CredentialId                           credId("XXXXXX");
                const std::map<cam::CredentialId, uint32_t> counters = accessor.getAllCredentialCounter();

                if (counters.count(credId) != 0) {
                    throw std::runtime_error("Mapping counted for " + credId);
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }
    } // 9.X

    // test 10.X
    {
        // test 10.1
        {
            try {
                cam::Accessor accessor0(syncClient);

                for (uint8_t index = 0; index < 5; index++) {
                    accessor0.getAllCredentialCounter();
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 10.2
        {
            try {
                for (uint8_t index = 0; index < 5; index++) {
                    cam::Accessor accessor0(syncClient);
                    accessor0.getAllCredentialCounter();
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }

        // test 10.3
        {
            try {
                cam::Accessor accessor0(syncClient);
                cam::Accessor accessor1(syncClient);

                for (uint8_t index = 0; index < 5; index++) {
                    accessor0.getAllCredentialCounter();
                    accessor1.getAllCredentialCounter();
                }
            } catch (const std::exception& e) {
                FAIL(e.what());
            }
        }
    } // 10.X
}
