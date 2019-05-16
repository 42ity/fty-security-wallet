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

    /*CredentialId getCredentialId(const AssetId & assetId, const UsageId & usageId) const;
    Status getCredentialStatus(const AssetId & assetId, const UsageId & usageId) const;
    MapExtendedInfo getExtendedInfo(const AssetId & assetId, const UsageId & usageId) const;

    bool isMAppingExisting(const AssetId & assetId, const UsageId & usageId) const;
    
    void updateCredentialId(const AssetId & assetId, const UsageId & usageId, const CredentialId & credentialId);
    void updateCredentialStatus(const AssetId & assetId, const UsageId & usageId, Status status);
    void updateExtendedInfo(const AssetId & assetId, const UsageId & usageId, const MapExtendedInfo & extendedInfo);

    void removeMapping(const AssetId & assetId, const UsageId & usageId);*/

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

    const CredentialAssetMapping getMapping(const AssetId & assetId, const UsageId & usageId) const;

    /*CredentialId getCredentialId(const AssetId & assetId, const UsageId & usageId) const;
    CredentialId getCredentialId(const AssetId & assetId, const UsageId & usageId) const;
    Status getCredentialStatus(const AssetId & assetId, const UsageId & usageId) const;
    MapExtendedInfo getExtendedInfo(const AssetId & assetId, const UsageId & usageId) const;*/

    bool isMAppingExisting(const AssetId & assetId, const UsageId & usageId) const;
    
    void updateCredentialId(const AssetId & assetId, const UsageId & usageId, const CredentialId & credentialId);
    void updateCredentialStatus(const AssetId & assetId, const UsageId & usageId, Status status);
    void updateExtendedInfo(const AssetId & assetId, const UsageId & usageId, const MapExtendedInfo & extendedInfo);

    void removeMapping(const AssetId & assetId, const UsageId & usageId);
  
    
  private:
    ClientId m_clientId;
    uint32_t m_timeout;
    mutable mlm_client_t * m_client;

    std::vector<std::string> sendCommand(const std::string & command, const std::vector<std::string> & frames) const;
  };
  
} //namespace cam

std::vector<std::pair<std::string,bool>> cam_accessor_test();

#endif

