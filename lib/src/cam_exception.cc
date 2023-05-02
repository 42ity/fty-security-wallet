/*  =========================================================================
    cam_exception - Credential asset mapping exceptions

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

#include "cam_exception.h"
#include "cxxtools/serializationinfo.h"
#include <fty_common_json.h>
#include <sstream>

namespace cam {
/*-----------------------------------------------------------------------------*/
/*   Exception                                                                 */
/*-----------------------------------------------------------------------------*/
// Public
void CamException::throwCamException(const std::string& data)
{
    // get the serializationInfo
    cxxtools::SerializationInfo si;

    JSON::readFromString(data, si);

    // extract the error code, the message and the extra data
    uint8_t                     errorCode = 0;
    std::string                 whatArg;
    cxxtools::SerializationInfo extraData;

    try {
        si.getMember("errorCode") >>= errorCode;
        si.getMember("whatArg") >>= whatArg;
        extraData = si.getMember("extraData");
    } catch (...) {
    }


    switch (errorCode) {
        case UNSUPPORTED_COMMAND:
            throw CamUnsupportedCommandException(whatArg);
        case PROTOCOL_ERROR:
            throw CamProtocolErrorException(whatArg);
        case BAD_COMMAND_ARGUMENT:
            throw CamBadCommandArgumentException(extraData, whatArg);
        case MAPPING_DOES_NOT_EXIST:
            throw CamMappingDoesNotExistException(extraData, whatArg);
        case MAPPING_ALREADY_EXISTS:
            throw CamMappingAlreadyExistsException(extraData, whatArg);
        case MAPPING_INVALID:
            throw CamMappingInvalidException(whatArg);
        default:
            throw CamException(whatArg);
    }
}

CamException::CamException(const std::string& whatArg, ErrorCode code)
    : m_code(code)
    , m_whatArg(whatArg)
{
}

CamException::CamException(ErrorCode code)
    : m_code(code)
    , m_whatArg("Unknown error")
{
}

const char* CamException::what() const noexcept
{
    return m_whatArg.c_str();
}

std::string CamException::toJson() const
{
    cxxtools::SerializationInfo si;

    si <<= *(this);

    return JSON::writeToString(si);
}

void operator<<=(cxxtools::SerializationInfo& si, const CamException& exception)
{
    si.addMember("errorCode") <<= uint8_t(exception.m_code);
    si.addMember("whatArg") <<= exception.m_whatArg;
    exception.fillSerializationInfo(si.addMember("extraData"));
}

// Protected
void CamException::fillSerializationInfo(cxxtools::SerializationInfo& /*si*/) const
{
}

// =====================================================================================================================

void CamUnsupportedCommandException::fillSerializationInfo(cxxtools::SerializationInfo& si) const
{
    si.addMember("command") <<= m_command;
}

CamUnsupportedCommandException::CamUnsupportedCommandException(
    const cxxtools::SerializationInfo& extraData, const std::string& whatArg)
    : CamException(whatArg, ErrorCode::UNSUPPORTED_COMMAND)
{
    extraData.getMember("command") >>= m_command;
}

CamUnsupportedCommandException::CamUnsupportedCommandException(const std::string& command)
    : CamException(ErrorCode::UNSUPPORTED_COMMAND)
    , m_command(command)
{
    m_whatArg = "Unsupported command: '" + m_command + "'";
}

// =====================================================================================================================

void CamBadCommandArgumentException::fillSerializationInfo(cxxtools::SerializationInfo& si) const
{
    si.addMember("argument") <<= m_argument;
}

CamBadCommandArgumentException::CamBadCommandArgumentException(const std::string& argument, const std::string& reason)
    : CamException(ErrorCode::BAD_COMMAND_ARGUMENT)
    , m_argument(argument)
{
    m_whatArg = "Command argument error: '" + m_argument + "' " + reason;
}

CamBadCommandArgumentException::CamBadCommandArgumentException(
    const cxxtools::SerializationInfo& extraData, const std::string& whatArg)
    : CamException(whatArg, ErrorCode::BAD_COMMAND_ARGUMENT)
{
    extraData.getMember("argument") >>= m_argument;
}

// =====================================================================================================================

void CamMappingDoesNotExistException::fillSerializationInfo(cxxtools::SerializationInfo& si) const
{
    si.addMember("asset") <<= m_asset;
    si.addMember("service") <<= m_service;
    si.addMember("protocol") <<= m_protocol;
}

CamMappingDoesNotExistException::CamMappingDoesNotExistException(
    const std::string& asset, const std::string& service, const std::string& protocol)
    : CamException(ErrorCode::MAPPING_DOES_NOT_EXIST)
    , m_asset(asset)
    , m_service(service)
    , m_protocol(protocol)
{
    m_whatArg = "Mapping for asset '" + m_asset + "', service '" + m_service + "' and protocol '" + m_protocol +
                "' do not exist";
}

CamMappingDoesNotExistException::CamMappingDoesNotExistException(
    const cxxtools::SerializationInfo& extraData, const std::string& whatArg)
    : CamException(whatArg, ErrorCode::MAPPING_DOES_NOT_EXIST)
{
    extraData.getMember("asset") >>= m_asset;
    extraData.getMember("service") >>= m_service;
    extraData.getMember("protocol") >>= m_protocol;
}

// =====================================================================================================================

void CamMappingAlreadyExistsException::fillSerializationInfo(cxxtools::SerializationInfo& si) const
{
    si.addMember("asset") <<= m_asset;
    si.addMember("service") <<= m_service;
    si.addMember("protocol") <<= m_protocol;
}

CamMappingAlreadyExistsException::CamMappingAlreadyExistsException(
    const std::string& asset, const std::string& service, const std::string& protocol)
    : CamException(ErrorCode::MAPPING_ALREADY_EXISTS)
    , m_asset(asset)
    , m_service(service)
    , m_protocol(protocol)
{
    m_whatArg = "Mapping for asset '" + m_asset + "', service '" + m_service + "' and protocol '" + m_protocol +
                "' already exist";
}

CamMappingAlreadyExistsException::CamMappingAlreadyExistsException(
    const cxxtools::SerializationInfo& extraData, const std::string& whatArg)
    : CamException(whatArg, ErrorCode::MAPPING_ALREADY_EXISTS)
{
    extraData.getMember("asset") >>= m_asset;
    extraData.getMember("service") >>= m_service;
    extraData.getMember("protocol") >>= m_protocol;
}

// =====================================================================================================================

} // namespace cam
