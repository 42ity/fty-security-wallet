/*  =========================================================================
    cam_credential_asset_mapping_storage - Credential asset mapping class to manage the storage

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
#include <memory>

namespace cam {
using Hash = std::string;

class CredentialAssetMappingStorage
{
public:
    explicit CredentialAssetMappingStorage(const std::string& databasePath);
    void save() const;

    const CredentialAssetMapping& getMapping(
        const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol) const;
    void setMapping(const CredentialAssetMapping& mapping);

    void removeMapping(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol);

    bool isMappingExisting(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol) const;

    std::vector<CredentialAssetMapping> getMappings(const AssetId& assetId, const ServiceId& serviceId) const;
    std::vector<CredentialAssetMapping> getAllMappings() const;

    std::vector<CredentialAssetMapping> getCredentialMappingsForService(
        const CredentialId& credentialId, const ServiceId& serviceId) const;

    std::vector<CredentialAssetMapping> getCredentialMappings(const CredentialId& credentialId) const;

    std::vector<CredentialAssetMapping> getAssetMappings(const AssetId& assetId) const;

    static constexpr const uint8_t MAPPING_VERSION = 1;

private:
    std::string                            m_pathDatabase;
    std::map<Hash, CredentialAssetMapping> m_mappings;

    static Hash computeHash(const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol);
};

} // namespace cam
