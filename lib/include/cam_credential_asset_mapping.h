/*  =========================================================================
    cam_credential_asset_mapping - Credential asset mapping class

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
#include "cam_exception.h"
#include <functional>
#include <iostream>
#include <map>
#include <memory>

namespace cxxtools {
class SerializationInfo;
}

namespace cam {

/**
 * Some typedef to make the code more clear
 */

using CredentialId    = std::string;
using AssetId         = std::string;
using ServiceId       = std::string;
using Protocol        = std::string;
using Port            = std::string;
using MapExtendedInfo = std::map<std::string, std::string>;

enum Status : uint8_t
{
    UNKNOWN = 0,
    VALID,
    ERROR
};

/**
 * Some key definition for serialization
 *
 */
static constexpr const char* SERVICE_ID_ENTRY        = "cam_service";
static constexpr const char* ASSET_ID_ENTRY          = "cam_asset";
static constexpr const char* PROTOCOL_ENTRY          = "cam_protocol";
static constexpr const char* PORT_ENTRY              = "cam_port";
static constexpr const char* CREDENTIAL_ID_ENTRY     = "cam_credential";
static constexpr const char* CREDENTIAL_STATUS_ENTRY = "cam_status";
static constexpr const char* EXTENDED_INFO_ENTRY     = "cam_extended_info";

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
    AssetId         m_assetId;
    ServiceId       m_serviceId;
    Protocol        m_protocol;
    Port            m_port;
    CredentialId    m_credentialId;
    Status          m_status;
    MapExtendedInfo m_extendedInfo;
};

void operator<<=(cxxtools::SerializationInfo& si, const CredentialAssetMapping& mapping);
void operator>>=(const cxxtools::SerializationInfo& si, CredentialAssetMapping& mapping);
void operator>>=(const std::string& str, CredentialAssetMapping& mapping);

// add a stream operator to display the CredentialAssetMapping in debug for example
std::ostream& operator<<(std::ostream& os, const CredentialAssetMapping& mapping);

} // namespace cam
