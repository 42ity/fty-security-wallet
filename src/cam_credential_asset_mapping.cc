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

#include "cam_credential_asset_mapping.h"

#include <cxxtools/jsonserializer.h>
#include <cxxtools/jsondeserializer.h>

namespace cam
{
  CredentialAssetMapping::CredentialAssetMapping()
  {}

  void CredentialAssetMapping::fillSerializationInfo(cxxtools::SerializationInfo& si) const
  {
    si.addMember(USAGE_ID_ENTRY) <<= m_usageId;
    si.addMember(ASSET_ID_ENTRY) <<= m_assetId;
    si.addMember(CREDENTIAL_ID_ENTRY) <<= m_credentialId;
    si.addMember(CREDENTIAL_STATUS_ENTRY) <<= m_credentialStatus;
    si.addMember(EXTENDED_INFO_ENTRY) <<= m_extendedInfo;
  }

  void CredentialAssetMapping::fromSerializationInfo(const cxxtools::SerializationInfo& si)
  {
    //try
    {
      si.getMember(USAGE_ID_ENTRY) >>= m_usageId;
      si.getMember(ASSET_ID_ENTRY) >>= m_assetId;
      si.getMember(CREDENTIAL_ID_ENTRY) >>= m_credentialId;
      
      uint8_t status = 0;
      si.getMember(CREDENTIAL_STATUS_ENTRY) >>= status;
      m_credentialStatus = static_cast<CredentialStatus>(status);
      
      si.getMember(EXTENDED_INFO_ENTRY) >>= m_extendedInfo;
    }
    /*catch(const std::exception& e)
    {
        throw SecwInvalidDocumentFormatException(e.what());
    }*/
  }

  std::string CredentialAssetMapping::toString() const
  {
    std::string returnData("");

    //try
    {
      cxxtools::SerializationInfo si;
      fillSerializationInfo(si);

      std::stringstream output;
      cxxtools::JsonSerializer serializer(output);
      serializer.serialize(si);

      returnData = output.str();
    }
    /*catch(const std::exception& e)
    {
      throw SecwException("Error while creating json "+std::string(e.what()));
    }*/

    return returnData;
  }

  void operator<<= (cxxtools::SerializationInfo& si, const CredentialAssetMapping & mapping)
  {
    mapping.fillSerializationInfo(si);
  }

  void operator>>= (const cxxtools::SerializationInfo& si, CredentialAssetMapping & mapping)
  {
    mapping.fromSerializationInfo(si);
  }

  //add a stream operator to display the CredentialAssetMapping in debug for example
  std::ostream& operator<< (std::ostream& os, const CredentialAssetMapping & mapping)
  {
    os << mapping.toString() << std::endl;
    return os;
  }

} // namepsace cam
