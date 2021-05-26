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

#pragma once

#include "cam_credential_asset_mapping.h"
#include <fty_common_client.h>
#include <fty_common_mlm_sync_client.h>
#include <memory>

namespace cam {

using ClientId = std::string;
class ClientAccessor;

class Accessor
{
public:
    explicit Accessor(mlm::MlmSyncClient& requestClient);

    /// Construct a new Accessor object using malamute
    /// @param clientId
    /// @param timeout
    /// @param endPoint
    [[deprecated]]
    Accessor(const ClientId& clientId, uint32_t timeout, const std::string& endPoint);

    ~Accessor();

    void createMapping(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol, const Port& port,
        const CredentialId& credentialId, Status status = Status::UNKNOWN, const MapExtendedInfo& extendedInfo = {});

    void createMapping(const CredentialAssetMapping& mapping);
    void updateMapping(const CredentialAssetMapping& mapping);

    const CredentialAssetMapping getMapping(
        const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol) const;

    void removeMapping(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol);

    void updateCredentialId(
        const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol, const CredentialId& credentialId);
    void updatePort(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol, const Port& port);
    void updateStatus(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol, Status status);
    void updateExtendedInfo(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol,
        const MapExtendedInfo& extendedInfo);

    bool isMappingExisting(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol) const;

    /*const std::vector<CredentialAssetMapping> getCredentialMappingsForService( const CredentialId & credentialId,
                                                                        const ServiceId & serviceId) const;*/

    const std::vector<CredentialAssetMapping> getCredentialMappings(const CredentialId& credentialId) const;

    const std::vector<CredentialAssetMapping> getAssetMappings(const AssetId& assetId) const;

    const std::vector<CredentialAssetMapping> getMappings(const AssetId& assetId, const ServiceId& serviceId) const;

    const std::vector<CredentialAssetMapping> getAllMappings() const;

    uint32_t countCredentialMappingsForCredential(const CredentialId& credentialId) const;

    const std::map<CredentialId, uint32_t> getAllCredentialCounter() const;


private:
    // for backward compatibility
    std::shared_ptr<mlm::MlmSyncClient> m_mlmClient;

    fty::SyncClient& m_requestClient;

    std::vector<std::string> sendCommand(const std::string& command, const std::vector<std::string>& frames) const;
};

} // namespace cam
