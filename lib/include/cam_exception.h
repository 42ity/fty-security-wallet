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

#pragma once

#include <exception>
#include <string>

namespace cxxtools {
class SerializationInfo;
}

namespace cam {

enum ErrorCode : uint8_t
{
    GENERIC = 0,
    UNSUPPORTED_COMMAND,
    PROTOCOL_ERROR,
    BAD_COMMAND_ARGUMENT,
    MAPPING_DOES_NOT_EXIST,
    MAPPING_ALREADY_EXISTS,
    MAPPING_INVALID,
    MLM_CLIENT_IS_NULL,
    MLM_INTERRUPTED,
    MLM_FAILED
};

// =====================================================================================================================

class CamException : public std::exception
{
public:
    explicit CamException(const std::string& whatArg, ErrorCode code = ErrorCode::GENERIC);
    explicit CamException(ErrorCode code = ErrorCode::GENERIC);

    virtual ~CamException()
    {
    }

    const char* what() const noexcept override;

    inline ErrorCode getErrorCode() const
    {
        return m_code;
    }

    // return an error payload from an exception
    std::string toJson() const;

    friend void operator<<=(cxxtools::SerializationInfo& si, const CamException& exception);

    // throw the good exception base on the error payload
    static void throwCamException(const std::string& data);

protected:
    virtual void fillSerializationInfo(cxxtools::SerializationInfo& si) const;

private:
    ErrorCode m_code;

protected:
    std::string m_whatArg;
};

void operator<<=(cxxtools::SerializationInfo& si, const CamException& exception);

// =====================================================================================================================

// Command is not supported
class CamUnsupportedCommandException : public CamException
{
private:
    std::string m_command;

    void fillSerializationInfo(cxxtools::SerializationInfo& si) const override;

public:
    explicit CamUnsupportedCommandException(const std::string& command);
    CamUnsupportedCommandException(const cxxtools::SerializationInfo& extraData, const std::string& whatArg);

    inline std::string getCommand() const
    {
        return m_command;
    }
};

// =====================================================================================================================

// Wrong message format => the message do not comply to the protocol
class CamProtocolErrorException : public CamException
{
public:
    explicit CamProtocolErrorException(const std::string& whatArg = "Protocol error")
        : CamException(whatArg, ErrorCode::PROTOCOL_ERROR)
    {
    }
};

// =====================================================================================================================

// Arguments for the command are not good
class CamBadCommandArgumentException : public CamException
{
private:
    std::string m_argument;

    void fillSerializationInfo(cxxtools::SerializationInfo& si) const override;

public:
    explicit CamBadCommandArgumentException(const std::string& argument, const std::string& reason = "");
    explicit CamBadCommandArgumentException(const cxxtools::SerializationInfo& extraData, const std::string& whatArg);

    inline std::string getArgument() const
    {
        return m_argument;
    }
};

// =====================================================================================================================

// Mapping do not exist
class CamMappingDoesNotExistException : public CamException
{
private:
    std::string m_asset, m_service, m_protocol;

    void fillSerializationInfo(cxxtools::SerializationInfo& si) const override;

public:
    CamMappingDoesNotExistException(const std::string& asset, const std::string& service, const std::string& protocol);
    CamMappingDoesNotExistException(const cxxtools::SerializationInfo& extraData, const std::string& whatArg);

    inline std::string getAssetId() const
    {
        return m_asset;
    }
    inline std::string getServiceId() const
    {
        return m_service;
    }
    inline std::string getProtocol() const
    {
        return m_protocol;
    }
};

// =====================================================================================================================

// Mapping already exist
class CamMappingAlreadyExistsException : public CamException
{
private:
    std::string m_asset, m_service, m_protocol;

    void fillSerializationInfo(cxxtools::SerializationInfo& si) const override;

public:
    CamMappingAlreadyExistsException(const std::string& asset, const std::string& service, const std::string& protocol);
    CamMappingAlreadyExistsException(const cxxtools::SerializationInfo& extraData, const std::string& whatArg);

    inline std::string getAssetId() const
    {
        return m_asset;
    }
    inline std::string getServiceId() const
    {
        return m_service;
    }
    inline std::string getProtocol() const
    {
        return m_protocol;
    }
};

// =====================================================================================================================

// Mapping invalid
class CamMappingInvalidException : public CamException
{
public:
    explicit CamMappingInvalidException(const std::string& whatArg)
        : CamException(whatArg, ErrorCode::MAPPING_INVALID)
    {
    }
};

// =====================================================================================================================

// Malamute client is null
class CamMalamuteClientIsNullException : public CamException
{
public:
    explicit CamMalamuteClientIsNullException()
        : CamException("Malamute client is null", ErrorCode::MLM_CLIENT_IS_NULL)
    {
    }
};

// =====================================================================================================================

// Malamute Interrupted
class CamMalamuteInterruptedException : public CamException
{
public:
    explicit CamMalamuteInterruptedException()
        : CamException("Malamute interrupted", ErrorCode::MLM_INTERRUPTED)
    {
    }
};

// =====================================================================================================================

// Malamute Connection Failed Exception
class CamMalamuteConnectionFailedException : public CamException
{
public:
    explicit CamMalamuteConnectionFailedException()
        : CamException("Malamute Connection Failed", ErrorCode::MLM_FAILED)
    {
    }
};

// =====================================================================================================================


} // namespace cam

