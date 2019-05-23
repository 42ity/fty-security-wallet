/*  =========================================================================
    cam_credential_asset_mapping - Credential asset mapping class

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

#ifndef CAM_CREDENTIAL_ASSET_MAPPING_H_INCLUDED
#define CAM_CREDENTIAL_ASSET_MAPPING_H_INCLUDED

#include "cxxtools/serializationinfo.h"
#include <memory>
#include <functional>
#include <iostream>

#include "cam_exception.h"

namespace cam
{
  
  /**
   * Some typedef to make the code more clear
   */

  using CredentialId = std::string;
  using AssetId = std::string;
  using UsageId = std::string;
  using MapExtendedInfo = std::map<std::string, std::string>;

  enum CredentialStatus : uint8_t
  {
    UNKNOWN = 0,
    VALID,
    ERROR
  };


  /**
   * Some key definition for serialization
   * 
   */
  static constexpr const char* USAGE_ID_ENTRY = "cam_usage";
  static constexpr const char* ASSET_ID_ENTRY = "cam_asset";
  static constexpr const char* CREDENTIAL_ID_ENTRY = "cam_credential";
  static constexpr const char* CREDENTIAL_STATUS_ENTRY = "cam_status";
  static constexpr const char* EXTENDED_INFO_ENTRY = "cam_extended_info";

  /**
   * \brief CredentialAssetMapping: Public interface
   */
  class CredentialAssetMapping
  {
  public:
    explicit CredentialAssetMapping();
    
    void fillSerializationInfo(cxxtools::SerializationInfo& si) const;
    void fromSerializationInfo(const cxxtools::SerializationInfo& si);

    std::string toString() const;

    // Attributs 
    AssetId m_assetId;
    UsageId m_usageId;
    CredentialId m_credentialId;
    CredentialStatus m_credentialStatus;
    MapExtendedInfo m_extendedInfo;

  };

  void operator<<= (cxxtools::SerializationInfo& si, const CredentialAssetMapping & mapping);
  void operator>>= (const cxxtools::SerializationInfo& si, CredentialAssetMapping & mapping);

  //add a stream operator to display the CredentialAssetMapping in debug for example
  std::ostream& operator<< (std::ostream& os, const CredentialAssetMapping & mapping);

} // namepsace cam

#endif
