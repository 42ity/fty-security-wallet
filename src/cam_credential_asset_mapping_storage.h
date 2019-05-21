/*  =========================================================================
    cam_credential_asset_mapping_storage - Credential asset mapping class to manage the storage

    Copyright (C) 2019 Eaton

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

#ifndef CAM_CREDENTIAL_ASSET_MAPPING_STORAGE_H_INCLUDED
#define CAM_CREDENTIAL_ASSET_MAPPING_STORAGE_H_INCLUDED

#include <memory>

#include "cam_credential_asset_mapping.h"

namespace cam
{
  using Hash = std::string;
  
  class CredentialAssetMappingStorage
  {
  public:
    explicit CredentialAssetMappingStorage(const std::string & databasePath);
    void save() const;
    
    const CredentialAssetMapping & getMapping(const AssetId & assetId, const UsageId & usageId) const;
    void setMapping(const CredentialAssetMapping & mapping);

    void removeMapping(const AssetId & assetId, const UsageId & usageId);

    bool isMappingExisting(const AssetId & assetId, const UsageId & usageId) const;

    std::vector<CredentialAssetMapping> getCredentialMappingsForUsage( const CredentialId & credentialId,
                                                                        const UsageId & usageId) const;

    std::vector<CredentialAssetMapping> getCredentialMappings(const CredentialId & credentialId) const;

    std::vector<CredentialAssetMapping> getAssetMappings(const AssetId & assetId) const;

    static constexpr const uint8_t MAPPING_VERSION = 1;

  private:
    std::string m_pathDatabase;
    std::map<Hash, CredentialAssetMapping> m_mappings;

    static Hash computeHash(const AssetId & assetId, const UsageId & usageId);
  };

} // namepsace cam 

#endif
