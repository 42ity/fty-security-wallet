/*  =========================================================================
    cam_credential_asset_mapping_server - Credential Asset Mapping Server

    Copyright (C) 2019 - 2020 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

/*
@header
    cam_credential_asset_mapping_server - Credential Asset Mapping Server
@discuss
@end
*/

#include "cam_credential_asset_mapping_server.h"
#include "cam_credential_asset_mapping.h"
#include "cam_credential_asset_mapping_storage.h"
#include "cam_exception.h"
#include "cam_helpers.h"
#include "fty_security_wallet.h"
#include <cxxtools/jsonserializer.h>
#include <sstream>
#include <fty_log.h>
#include <fty_common.h>
#include <fty/convert.h>

static constexpr auto MSGBUS_SRR_SECW_CAM_QUEUE = "ETN.Q.IPMCORE.CREDASSETMAPPING";
static constexpr auto FEATURE_SRR_SECW_CAM = "credential-asset-mapping";
static constexpr auto ACTIVE_SRR_SECW_CAM_VERSION = "1.0";

using namespace std::placeholders;

namespace cam {

CredentialAssetMappingServer::CredentialAssetMappingServer(
    const std::string& storagePath,
    const std::string& srrEndpoint,
    const std::string& srrAgentName
)
    : m_activeMapping(storagePath)
    , m_srrProcessor(new dto::srr::SrrQueryProcessor)
{
    // initiate the commands handlers
    m_supportedCommands[CREATE_MAPPING] = std::bind(&CredentialAssetMappingServer::handleCreateMapping, this, _1, _2);

    m_supportedCommands[GET_MAPPING] = std::bind(&CredentialAssetMappingServer::handleGetMapping, this, _1, _2);

    m_supportedCommands[UPDATE_PORT_MAPPING] =
        std::bind(&CredentialAssetMappingServer::handleUpdatePortMapping, this, _1, _2);
    m_supportedCommands[UPDATE_CREDENTIAL_MAPPING] =
        std::bind(&CredentialAssetMappingServer::handleUpdateCredentialMapping, this, _1, _2);
    m_supportedCommands[UPDATE_STATUS_MAPPING] =
        std::bind(&CredentialAssetMappingServer::handleUpdateStatusMapping, this, _1, _2);
    m_supportedCommands[UPDATE_INFO_MAPPING] =
        std::bind(&CredentialAssetMappingServer::handleUpdateInfoMapping, this, _1, _2);
    m_supportedCommands[UPDATE_MAPPING] = std::bind(&CredentialAssetMappingServer::handleUpdateMapping, this, _1, _2);

    m_supportedCommands[REMOVE_MAPPING] = std::bind(&CredentialAssetMappingServer::handleRemoveMapping, this, _1, _2);

    m_supportedCommands[GET_ASSET_MAPPINGS] =
        std::bind(&CredentialAssetMappingServer::handleGetAssetMappings, this, _1, _2);
    m_supportedCommands[GET_CRED_MAPPINGS] =
        std::bind(&CredentialAssetMappingServer::handleGetCredentialMappings, this, _1, _2);
    m_supportedCommands[GET_MAPPINGS] = std::bind(&CredentialAssetMappingServer::handleGetMappings, this, _1, _2);
    m_supportedCommands[GET_ALL_MAPPINGS] =
        std::bind(&CredentialAssetMappingServer::handleGetAllMappings, this, _1, _2);

    m_supportedCommands[COUNT_CRED_MAPPINGS] =
        std::bind(&CredentialAssetMappingServer::handleCountCredentialMappingsForCredential, this, _1, _2);

    if ((!srrEndpoint.empty()) && (!srrAgentName.empty())) {
        log_debug("CAM SRR connect %s to %s", srrAgentName.c_str(), srrEndpoint.c_str());
        m_msgBus.reset(messagebus::MlmMessageBus(srrEndpoint, srrAgentName));
        m_msgBus->connect();

        m_srrProcessor->saveHandler    = std::bind(&CredentialAssetMappingServer::handleSRRSave, this, _1);
        m_srrProcessor->restoreHandler = std::bind(&CredentialAssetMappingServer::handleSRRRestore, this, _1);

        // Listen all incoming request
        log_debug("CAM SRR listen to %s", MSGBUS_SRR_SECW_CAM_QUEUE);
        m_msgBus->receive(MSGBUS_SRR_SECW_CAM_QUEUE, std::bind(&CredentialAssetMappingServer::handleSRRRequest, this, _1));
    }
    else {
        log_info("CAM SRR not activated (endpoint: %s, address: %s)", srrEndpoint.c_str(), srrAgentName.c_str());
    }
}

CredentialAssetMappingServer::~CredentialAssetMappingServer()
{
    // ensure nothing else is on going
    std::unique_lock<std::mutex> lock(m_lock);
}

std::vector<std::string> CredentialAssetMappingServer::handleRequest(
    const Sender& sender, const std::vector<std::string>& payload)
{
    try {
        if (payload.size() == 0) {
            throw CamProtocolErrorException("Command frame is empty");
        }

        Command cmd = payload.at(0);

        if (cmd == "ERROR" || cmd == "OK") {
            // avoid loop
            return {};
        }

        // check if the command exist in the system
        if (m_supportedCommands.count(cmd) == 0) {
            throw CamUnsupportedCommandException(cmd + " not supported");
        }

        FctCommandHandler cmdHandler = m_supportedCommands[cmd];

        // Declaring new vector
        std::vector<std::string> params(payload.begin() + 1, payload.end());

        std::string result = cmdHandler(sender, params);

        return {result};
    } catch (CamException& e) {
        log_warning("%s", e.what());
        return {"ERROR", e.toJson()};
    } catch (std::exception& e) {
        log_error("Unexpected error: %s", e.what());
        return {"ERROR", ""};
    } catch (...) // show must go one => Log and ignore the unknown error
    {
        log_error("Unexpected error: unknown");
        return {"ERROR", ""};
    }
}

/* Commands implementation section*/
// TODO remove: throw CamException("Command is not implemented yet!!");

std::string CredentialAssetMappingServer::handleGetMapping(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. asset id
     * 1. usage id
     * 2. protocol
     */

    if (params.size() < 2) {
        throw CamBadCommandArgumentException("", "Command needs at least 2 arguments");
    }

    const AssetId&   assetId   = params.at(0);
    const ServiceId& serviceId = params.at(1);
    const Protocol&  protocol  = params.at(2);

    const CredentialAssetMapping& mapping = m_activeMapping.getMapping(assetId, serviceId, protocol);

    cxxtools::SerializationInfo si;

    mapping.fillSerializationInfo(si);

    return serialize(si);
}

std::string CredentialAssetMappingServer::handleCreateMapping(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. CredentialMapping object in json
     */

    if (params.size() < 1) {
        throw CamBadCommandArgumentException("", "Command needs at least 1 argument");
    }

    const cxxtools::SerializationInfo si = deserialize(params.at(0));

    CredentialAssetMapping mapping;

    mapping.fromSerializationInfo(si);

    if (m_activeMapping.isMappingExisting(mapping.m_assetId, mapping.m_serviceId, mapping.m_protocol)) {
        throw CamMappingAlreadyExistsException(mapping.m_assetId, mapping.m_serviceId, mapping.m_protocol);
    }

    m_activeMapping.setMapping(mapping);

    m_activeMapping.save();

    return "";
}

std::string CredentialAssetMappingServer::handleRemoveMapping(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. asset id
     * 1. usage id
     * 2. protocol
     */

    if (params.size() < 2) {
        throw CamBadCommandArgumentException("", "Command needs at least 2 arguments");
    }

    const AssetId&   assetId   = params.at(0);
    const ServiceId& serviceId = params.at(1);
    const Protocol&  protocol  = params.at(2);

    m_activeMapping.removeMapping(assetId, serviceId, protocol);

    m_activeMapping.save();

    return "";
}

std::string CredentialAssetMappingServer::handleUpdateMapping(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. CredentialMapping object in json
     */

    if (params.size() < 1) {
        throw CamBadCommandArgumentException("", "Command needs at least 1 argument");
    }

    const cxxtools::SerializationInfo si = deserialize(params.at(0));

    CredentialAssetMapping mapping;

    mapping.fromSerializationInfo(si);

    if (!m_activeMapping.isMappingExisting(mapping.m_assetId, mapping.m_serviceId, mapping.m_protocol)) {
        throw CamMappingDoesNotExistException(mapping.m_assetId, mapping.m_serviceId, mapping.m_protocol);
    }

    m_activeMapping.setMapping(mapping);

    m_activeMapping.save();

    return "";
}

std::string CredentialAssetMappingServer::handleUpdatePortMapping(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. CredentialMapping object in json
     */

    if (params.size() < 1) {
        throw CamBadCommandArgumentException("", "Command needs at least 1 argument");
    }

    const cxxtools::SerializationInfo si = deserialize(params.at(0));

    CredentialAssetMapping receivedMapping;

    receivedMapping.fromSerializationInfo(si);

    CredentialAssetMapping existingMapping =
        m_activeMapping.getMapping(receivedMapping.m_assetId, receivedMapping.m_serviceId, receivedMapping.m_protocol);

    // update the mapping
    existingMapping.m_port   = receivedMapping.m_port;
    existingMapping.m_status = Status::UNKNOWN;

    m_activeMapping.setMapping(existingMapping);

    m_activeMapping.save();

    return "";
}

std::string CredentialAssetMappingServer::handleUpdateCredentialMapping(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. CredentialMapping object in json
     */

    if (params.size() < 1) {
        throw CamBadCommandArgumentException("", "Command needs at least 1 argument");
    }

    const cxxtools::SerializationInfo si = deserialize(params.at(0));

    CredentialAssetMapping receivedMapping;

    receivedMapping.fromSerializationInfo(si);

    CredentialAssetMapping existingMapping =
        m_activeMapping.getMapping(receivedMapping.m_assetId, receivedMapping.m_serviceId, receivedMapping.m_protocol);

    // update the mapping
    existingMapping.m_credentialId = receivedMapping.m_credentialId;
    existingMapping.m_status       = Status::UNKNOWN;

    m_activeMapping.setMapping(existingMapping);

    m_activeMapping.save();

    return "";
}

std::string CredentialAssetMappingServer::handleUpdateStatusMapping(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. CredentialMapping object in json
     */

    if (params.size() < 1) {
        throw CamBadCommandArgumentException("", "Command needs at least 1 argument");
    }

    const cxxtools::SerializationInfo si = deserialize(params.at(0));

    CredentialAssetMapping receivedMapping;

    receivedMapping.fromSerializationInfo(si);

    CredentialAssetMapping existingMapping =
        m_activeMapping.getMapping(receivedMapping.m_assetId, receivedMapping.m_serviceId, receivedMapping.m_protocol);

    // update the mapping
    existingMapping.m_status = receivedMapping.m_status;

    m_activeMapping.setMapping(existingMapping);

    m_activeMapping.save();

    return "";
}

std::string CredentialAssetMappingServer::handleUpdateInfoMapping(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. CredentialMapping object in json
     */

    if (params.size() < 1) {
        throw CamBadCommandArgumentException("", "Command needs at least 1 argument");
    }

    const cxxtools::SerializationInfo si = deserialize(params.at(0));

    CredentialAssetMapping receivedMapping;

    receivedMapping.fromSerializationInfo(si);

    CredentialAssetMapping existingMapping =
        m_activeMapping.getMapping(receivedMapping.m_assetId, receivedMapping.m_serviceId, receivedMapping.m_protocol);

    // update the mapping
    existingMapping.m_extendedInfo = receivedMapping.m_extendedInfo;

    m_activeMapping.setMapping(existingMapping);

    m_activeMapping.save();

    return "";
}

std::string CredentialAssetMappingServer::handleGetAssetMappings(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. asset id
     */

    if (params.size() < 1) {
        throw CamBadCommandArgumentException("", "Command needs at least 1 argument");
    }

    const AssetId& assetId = params.at(0);

    std::vector<CredentialAssetMapping> mappings = m_activeMapping.getAssetMappings(assetId);

    cxxtools::SerializationInfo si;

    si <<= mappings;

    return serialize(si);
}

std::string CredentialAssetMappingServer::handleGetMappings(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. asset id
     * 1. service id
     */

    if (params.size() < 2) {
        throw CamBadCommandArgumentException("", "Command needs at least 2 arguments");
    }

    const AssetId&   assetId   = params.at(0);
    const ServiceId& serviceId = params.at(1);

    std::vector<CredentialAssetMapping> mappings = m_activeMapping.getMappings(assetId, serviceId);

    cxxtools::SerializationInfo si;

    si <<= mappings;

    return serialize(si);
}

std::string CredentialAssetMappingServer::handleGetAllMappings(
    const Sender& /*sender*/, const std::vector<std::string>& /*params*/)
{
    /*
     * Parameters for this command:
     */

    std::vector<CredentialAssetMapping> mappings = m_activeMapping.getAllMappings();

    cxxtools::SerializationInfo si;

    si <<= mappings;

    return serialize(si);
}


std::string CredentialAssetMappingServer::handleGetCredentialMappings(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. credential id
     */

    if (params.size() < 1) {
        throw CamBadCommandArgumentException("", "Command needs at least 1 argument");
    }

    const CredentialId& credentialId = params.at(0);

    std::vector<CredentialAssetMapping> mappings = m_activeMapping.getCredentialMappings(credentialId);

    cxxtools::SerializationInfo si;

    si <<= mappings;

    return serialize(si);
}

std::string CredentialAssetMappingServer::handleCountCredentialMappingsForCredential(
    const Sender& /*sender*/, const std::vector<std::string>& params)
{
    /*
     * Parameters for this command:
     *
     * 0. credential id
     */

    if (params.size() < 1) {
        throw CamBadCommandArgumentException("", "Command needs at least 1 argument");
    }

    const CredentialId& credentialId = params.at(0);

    std::vector<CredentialAssetMapping> mappings = m_activeMapping.getCredentialMappings(credentialId);

    cxxtools::SerializationInfo si;

    si <<= static_cast<uint32_t>(mappings.size());

    return serialize(si);
}

// SRR

//fwd decl.
static void sendResponse(
    std::unique_ptr<messagebus::MessageBus>& msgBus, const messagebus::Message& msg, const dto::UserData& userData);

void CredentialAssetMappingServer::handleSRRRequest(messagebus::Message msg)
{
    log_debug("CAM Configuration handle request");

    try {
        // Get request
        dto::UserData data = msg.userData();
        dto::srr::Query query;
        data >> query;
        // Process request
        dto::UserData response;
        response << (m_srrProcessor->processQuery(query));
        // Send response
        sendResponse(m_msgBus, msg, response);
    } catch (std::exception& e) {
        log_error("Unexpected error: %s", e.what());
    } catch (...) {
        log_error("Unexpected error: unknown");
    }
}

dto::srr::SaveResponse CredentialAssetMappingServer::handleSRRSave(const dto::srr::SaveQuery& query)
{
    using namespace dto;
    using namespace dto::srr;

    log_debug("Saving CAM configuration");

    std::map<FeatureName, FeatureAndStatus> mapFeaturesData;

    for (const auto& featureName : query.features()) {
        FeatureAndStatus fs1;
        Feature&         f1 = *(fs1.mutable_feature());

        log_debug("feature: %s", featureName.c_str());

        if (featureName == FEATURE_SRR_SECW_CAM) {
            f1.set_version(ACTIVE_SRR_SECW_CAM_VERSION);
            try {
                std::unique_lock<std::mutex> lock(m_lock);

                cxxtools::SerializationInfo si;
                m_activeMapping.serialize(si);
                std::string data = JSON::writeToString(si, false);
                log_debug("Si=\n%s", data.c_str());

                f1.set_data(data);
                fs1.mutable_status()->set_status(dto::srr::Status::SUCCESS);
            } catch (std::exception& e) {
                log_debug("Save CAM failed (%s)", e.what());
                fs1.mutable_status()->set_status(dto::srr::Status::FAILED);
                fs1.mutable_status()->set_error(e.what());
            }
        } else {
            fs1.mutable_status()->set_status(dto::srr::Status::FAILED);
            fs1.mutable_status()->set_error("Feature is not supported!");
        }

        mapFeaturesData[featureName] = fs1;
    }

    log_debug("Save CAM configuration done");

    return (createSaveResponse(mapFeaturesData, ACTIVE_SRR_SECW_CAM_VERSION)).save();
}

dto::srr::RestoreResponse CredentialAssetMappingServer::handleSRRRestore(const dto::srr::RestoreQuery& query)
{
    using namespace dto;
    using namespace dto::srr;

    log_debug("Restoring CAM configuration");

    std::map<FeatureName, FeatureStatus> mapStatus;

    for (const auto& item : query.map_features_data()) {
        const FeatureName& featureName = item.first;
        const Feature&     feature     = item.second;

        log_debug("feature: %s", featureName.c_str());

        FeatureStatus featureStatus;
        if (featureName == FEATURE_SRR_SECW_CAM) {
            try {
                std::unique_lock<std::mutex> lock(m_lock);
                log_debug("Si=\n%s", feature.data().c_str());

                std::string version = feature.version();
                if (fty::convert<float>(version) > fty::convert<float>(ACTIVE_SRR_SECW_CAM_VERSION)) {
                    throw std::runtime_error("Version " + version + " is not supported");
                }

                const cxxtools::SerializationInfo si = deserialize(feature.data());
                m_activeMapping.parse(si);
                m_activeMapping.save(); // persist in database

                featureStatus.set_status(dto::srr::Status::SUCCESS);
            } catch (std::exception& e) {
                log_debug("Restore CAM failed (%s)", e.what());
                featureStatus.set_status(dto::srr::Status::FAILED);
                featureStatus.set_error(e.what());
            }
        } else {
            featureStatus.set_status(dto::srr::Status::FAILED);
            featureStatus.set_error("Feature is not supported!");
        }

        mapStatus[featureName] = featureStatus;
    }

    log_debug("Restore CAM configuration done");

    return (createRestoreResponse(mapStatus)).restore();
}


/**
 * Send FEATURE_SRR_SECW_CAM response on message bus.
 * @param msgBus
 * @param msg : the request
 * @param userData : the response payload
 */
static void sendResponse(
    std::unique_ptr<messagebus::MessageBus>& msgBus, const messagebus::Message& msg, const dto::UserData& userData)
{
    try {
        messagebus::Message resp;
        resp.userData() = userData;
        resp.metaData().emplace(
            messagebus::Message::SUBJECT, msg.metaData().find(messagebus::Message::SUBJECT)->second);
        resp.metaData().emplace(messagebus::Message::FROM, FEATURE_SRR_SECW_CAM);
        resp.metaData().emplace(messagebus::Message::TO, msg.metaData().find(messagebus::Message::FROM)->second);
        resp.metaData().emplace(
            messagebus::Message::CORRELATION_ID, msg.metaData().find(messagebus::Message::CORRELATION_ID)->second);
        msgBus->sendReply(msg.metaData().find(messagebus::Message::REPLY_TO)->second, resp);
    } catch (messagebus::MessageBusException& ex) {
        log_error("Message bus error: %s", ex.what());
    } catch (...) {
        log_error("Unexpected error: unknown");
    }
}

} // namespace cam
