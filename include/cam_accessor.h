/*  =========================================================================
    cam_accessor - Accessor to Credential asset mapping

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

#ifndef CAM_ACCESSOR_H_INCLUDED
#define CAM_ACCESSOR_H_INCLUDED

#include "cam_credential_asset_mapping.h"

#include <czmq.h>
#include <malamute.h>

namespace cam
{
  using ClientId = std::string;

  class ClientAccessor;
  
  class Accessor
  {
  public:
    /**
     * @brief Construct a new Accessor object
     *
     * @param clientId
     * @param timeout
     * @param endPoint
     */
    explicit Accessor(const ClientId & clientId,
                uint32_t timeout,
                const std::string & endPoint);
    
    ~Accessor();

    void createMapping( const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol,
                        const Port & port, const CredentialId & credentialId, Status status = Status::UNKNOWN,
                        const MapExtendedInfo & extendedInfo = {});

    const CredentialAssetMapping getMapping(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol) const;

    void removeMapping(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol);

    void updateCredentialId(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol, const CredentialId & credentialId);
    void updatePort(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol, const Port & port);
    void updateStatus(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol, Status status);
    void updateExtendedInfo(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol, const MapExtendedInfo & extendedInfo);

    bool isMappingExisting(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol) const;

    /*const std::vector<CredentialAssetMapping> getCredentialMappingsForService( const CredentialId & credentialId,
                                                                        const ServiceId & serviceId) const;*/

    const std::vector<CredentialAssetMapping> getCredentialMappings(const CredentialId & credentialId) const;

    const std::vector<CredentialAssetMapping> getAssetMappings(const AssetId & assetId) const;

    const std::vector<CredentialAssetMapping> getMappings(const AssetId & assetId, const ServiceId & serviceId) const;

    const std::vector<CredentialAssetMapping> getAllMappings() const;

    uint32_t countCredentialMappingsForCredential(const CredentialId & credentialId) const;

  
  private:
    ClientId m_clientId;
    uint32_t m_timeout;
    mutable mlm_client_t * m_client;

    std::vector<std::string> sendCommand(const std::string & command, const std::vector<std::string> & frames) const;
  };
  
} //namespace cam

std::vector<std::pair<std::string,bool>> cam_accessor_test();

#endif

