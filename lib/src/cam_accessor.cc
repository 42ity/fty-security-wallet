/*  =========================================================================
    cam_accessor - Accessor to Credential asset mapping

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

#include "cam_accessor.h"
#include "cam_credential_asset_mapping_server.h"
#include "cam_helpers.h"
#include "fty_security_wallet.h"
#include <fty_common_mlm_sync_client.h>

namespace cam {

Accessor::Accessor(mlm::MlmSyncClient& requestClient)
    : m_mlmClient()
    , m_requestClient(requestClient)
{
}

Accessor::Accessor(const ClientId& clientId, uint32_t timeout, const std::string& endPoint)
    : m_mlmClient(std::make_shared<mlm::MlmSyncClient>(clientId, MAPPING_AGENT, timeout, endPoint))
    , m_requestClient(*m_mlmClient)
{
}

Accessor::~Accessor()
{
}

void Accessor::createMapping(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol,
    const Port& port, const CredentialId& credentialId, Status status, const MapExtendedInfo& extendedInfo)
{
    CredentialAssetMapping mapping;

    mapping.m_assetId      = assetId;
    mapping.m_serviceId    = serviceId;
    mapping.m_protocol     = protocol;
    mapping.m_port         = port;
    mapping.m_credentialId = credentialId;
    mapping.m_status       = status;
    mapping.m_extendedInfo = extendedInfo;

    createMapping(mapping);
}

void Accessor::createMapping(const CredentialAssetMapping& mapping)
{
    cxxtools::SerializationInfo si;
    si <<= mapping;
    sendCommand(CredentialAssetMappingServer::CREATE_MAPPING, {serialize(si)});
}

void Accessor::updateMapping(const CredentialAssetMapping& mapping)
{
    cxxtools::SerializationInfo si;
    si <<= mapping;
    sendCommand(CredentialAssetMappingServer::UPDATE_MAPPING, {serialize(si)});
}

const CredentialAssetMapping Accessor::getMapping(
    const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol) const
{
    CredentialAssetMapping mapping;

    std::vector<std::string> payload;

    payload = sendCommand(CredentialAssetMappingServer::GET_MAPPING, {assetId, serviceId, protocol});

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= mapping;

    return mapping;
}

void Accessor::removeMapping(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol)
{
    sendCommand(CredentialAssetMappingServer::REMOVE_MAPPING, {assetId, serviceId, protocol});
}

/*bool isMappingExisting(const AssetId & assetId, const ServiceId & serviceId) const;*/

void Accessor::updateCredentialId(
    const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol, const CredentialId& credentialId)
{
    CredentialAssetMapping mapping;
    mapping.m_assetId      = assetId;
    mapping.m_serviceId    = serviceId;
    mapping.m_protocol     = protocol;
    mapping.m_credentialId = credentialId;

    cxxtools::SerializationInfo si;

    si <<= mapping;

    sendCommand(CredentialAssetMappingServer::UPDATE_CREDENTIAL_MAPPING, {serialize(si)});
}

void Accessor::updatePort(
    const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol, const Port& port)
{
    CredentialAssetMapping mapping;
    mapping.m_assetId   = assetId;
    mapping.m_serviceId = serviceId;
    mapping.m_protocol  = protocol;
    mapping.m_port      = port;

    cxxtools::SerializationInfo si;

    si <<= mapping;

    sendCommand(CredentialAssetMappingServer::UPDATE_PORT_MAPPING, {serialize(si)});
}

void Accessor::updateStatus(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol, Status status)
{
    CredentialAssetMapping mapping;
    mapping.m_assetId   = assetId;
    mapping.m_serviceId = serviceId;
    mapping.m_protocol  = protocol;
    mapping.m_status    = status;

    cxxtools::SerializationInfo si;

    si <<= mapping;

    sendCommand(CredentialAssetMappingServer::UPDATE_STATUS_MAPPING, {serialize(si)});
}

void Accessor::updateExtendedInfo(
    const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol, const MapExtendedInfo& extendedInfo)
{
    CredentialAssetMapping mapping;
    mapping.m_assetId      = assetId;
    mapping.m_serviceId    = serviceId;
    mapping.m_protocol     = protocol;
    mapping.m_extendedInfo = extendedInfo;

    cxxtools::SerializationInfo si;

    si <<= mapping;

    sendCommand(CredentialAssetMappingServer::UPDATE_INFO_MAPPING, {serialize(si)});
}

bool Accessor::isMappingExisting(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol) const
{
    bool exist = true;

    try {
        getMapping(assetId, serviceId, protocol);
    } catch (const CamMappingDoesNotExistException&) {
        exist = false;
    }

    return exist;
}

const std::vector<CredentialAssetMapping> Accessor::getAssetMappings(const AssetId& assetId) const
{
    std::vector<CredentialAssetMapping> mappings;
    std::vector<std::string>            payload;

    payload = sendCommand(CredentialAssetMappingServer::GET_ASSET_MAPPINGS, {assetId});

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= mappings;

    return mappings;
}

const std::vector<CredentialAssetMapping> Accessor::getMappings(
    const AssetId& assetId, const ServiceId& serviceId) const
{
    std::vector<CredentialAssetMapping> mappings;
    std::vector<std::string>            payload;

    payload = sendCommand(CredentialAssetMappingServer::GET_MAPPINGS, {assetId, serviceId});

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= mappings;

    return mappings;
}

const std::vector<CredentialAssetMapping> Accessor::getAllMappings() const
{
    std::vector<CredentialAssetMapping> mappings;
    std::vector<std::string>            payload;

    payload = sendCommand(CredentialAssetMappingServer::GET_ALL_MAPPINGS, {});

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= mappings;

    return mappings;
}

const std::vector<CredentialAssetMapping> Accessor::getCredentialMappings(const CredentialId& credentialId) const
{
    std::vector<CredentialAssetMapping> mappings;
    std::vector<std::string>            payload;

    payload = sendCommand(CredentialAssetMappingServer::GET_CRED_MAPPINGS, {credentialId});

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= mappings;

    return mappings;
}

uint32_t Accessor::countCredentialMappingsForCredential(const CredentialId& credentialId) const
{
    uint32_t counter = 0;

    std::vector<std::string> payload;

    payload = sendCommand(CredentialAssetMappingServer::COUNT_CRED_MAPPINGS, {credentialId});

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= counter;

    return counter;
}

const std::map<CredentialId, uint32_t> Accessor::getAllCredentialCounter() const
{
    std::map<CredentialId, uint32_t> counters;

    const std::vector<CredentialAssetMapping> mappings = getAllMappings();

    for (const CredentialAssetMapping& mapping : mappings) {
        if (!mapping.m_credentialId.empty()) {
            if (counters.count(mapping.m_credentialId) == 0) {
                counters[mapping.m_credentialId] = 1;
            } else {
                counters[mapping.m_credentialId] = counters[mapping.m_credentialId] + 1;
            }
        }
    }

    return counters;
}

std::vector<std::string> Accessor::sendCommand(const std::string& command, const std::vector<std::string>& frames) const
{
    std::vector<std::string> payload = {command};
    std::copy(frames.begin(), frames.end(), back_inserter(payload));

    std::vector<std::string> receivedFrames = m_requestClient.syncRequestWithReply(payload);

    // check if the first frame we get is an error
    if (receivedFrames[0] == "ERROR") {
        // It's an error and we will throw directly the exceptions
        if (receivedFrames.size() == 2) {
            CamException::throwCamException(receivedFrames.at(1));
        } else {
            throw CamProtocolErrorException("Missing data for error");
        }
    }

    return receivedFrames;
    
}

} // namespace cam
