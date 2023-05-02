/*  =========================================================================
    secw_exception - secw exception

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

/*
@header
    secw_exception - secw exception
@discuss
@end
*/

#include "secw_exception.h"
#include "cxxtools/serializationinfo.h"
#include <fty_common_json.h>
#include <sstream>

namespace secw {

/*-----------------------------------------------------------------------------*/
/*   Exception                                                                 */
/*-----------------------------------------------------------------------------*/
// Public

// =====================================================================================================================

void SecwException::throwSecwException(const std::string& data)
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
            throw SecwUnsupportedCommandException(whatArg);
        case PROTOCOL_ERROR:
            throw SecwProtocolErrorException(whatArg);
        case BAD_COMMAND_ARGUMENT:
            throw SecwBadCommandArgumentException(whatArg);
        case UNKNOWN_DOCUMENT_TYPE:
            throw SecwUnknownDocumentTypeException(extraData, whatArg);
        case UNKNOWN_PORTFOLIO:
            throw SecwUnknownPortfolioException(extraData, whatArg);
        case INVALID_DOCUMENT_FORMAT:
            throw SecwInvalidDocumentFormatException(extraData, whatArg);
        case IMPOSSIBLE_TO_LOAD_PORTFOLIO:
            throw SecwImpossibleToLoadPortfolioException(whatArg);
        case UNKNOWN_TAG:
            throw SecwUnknownTagException(whatArg);
        case DOCUMENT_DO_NOT_EXIST:
            throw SecwDocumentDoNotExistException(extraData, whatArg);
        case ILLEGAL_ACCESS:
            throw SecwIllegalAccess(whatArg);
        case UNKNOWN_USAGE_ID:
            throw SecwUnknownUsageIDException(extraData, whatArg);
        case NAME_ALREADY_EXISTS:
            throw SecwNameAlreadyExistsException(extraData, whatArg);
        case NAME_DOES_NOT_EXIST:
            throw SecwNameDoesNotExistException(extraData, whatArg);
        default:
            throw SecwException(whatArg);
    }
}

// =====================================================================================================================

SecwException::SecwException(const std::string& whatArg, ErrorCode code)
    : m_code(code)
    , m_whatArg(whatArg)
{
}

const char* SecwException::what() const noexcept
{
    return m_whatArg.c_str();
}

std::string SecwException::toJson() const
{
    cxxtools::SerializationInfo si;
    si <<= *(this);
    
    return JSON::writeToString(si, true);
}

void operator<<=(cxxtools::SerializationInfo& si, const SecwException& exception)
{
    si.addMember("errorCode") <<= uint8_t(exception.m_code);
    si.addMember("whatArg") <<= exception.m_whatArg;
    exception.fillSerializationInfo(si.addMember("extraData"));
}

// Protected
void SecwException::fillSerializationInfo(cxxtools::SerializationInfo& /*si*/) const
{
}

// =====================================================================================================================

void SecwUnknownDocumentTypeException::fillSerializationInfo(cxxtools::SerializationInfo& si) const
{
    si.addMember("documentType") <<= m_documentType;
}

SecwUnknownDocumentTypeException::SecwUnknownDocumentTypeException(const std::string& documentType)
    : SecwException("Unknown document type '" + documentType + "'", ErrorCode::UNKNOWN_DOCUMENT_TYPE)
    , m_documentType(documentType)
{
}

SecwUnknownDocumentTypeException::SecwUnknownDocumentTypeException(
    const cxxtools::SerializationInfo& extraData, const std::string& whatArg)
    : SecwException(whatArg, ErrorCode::UNKNOWN_DOCUMENT_TYPE)
{
    extraData.getMember("documentType") >>= m_documentType;
}

// =====================================================================================================================

void SecwUnknownPortfolioException::fillSerializationInfo(cxxtools::SerializationInfo& si) const
{
    si.addMember("portfolioName") <<= m_portfolioName;
}

SecwUnknownPortfolioException::SecwUnknownPortfolioException(const std::string& portfolioName)
    : SecwException("Unknown portfolio '" + portfolioName + "'", ErrorCode::UNKNOWN_PORTFOLIO)
    , m_portfolioName(portfolioName)
{
}

SecwUnknownPortfolioException::SecwUnknownPortfolioException(
    const cxxtools::SerializationInfo& extraData, const std::string& whatArg)
    : SecwException(whatArg, ErrorCode::UNKNOWN_PORTFOLIO)
{
    extraData.getMember("portfolioName") >>= m_portfolioName;
}

// =====================================================================================================================

void SecwInvalidDocumentFormatException::fillSerializationInfo(cxxtools::SerializationInfo& si) const
{
    si.addMember("documentField") <<= m_documentField;
}

SecwInvalidDocumentFormatException::SecwInvalidDocumentFormatException(const std::string& documentField)
    : SecwException("Error in field '" + documentField + "'", ErrorCode::INVALID_DOCUMENT_FORMAT)
    , m_documentField(documentField)
{
}

SecwInvalidDocumentFormatException::SecwInvalidDocumentFormatException(
    const cxxtools::SerializationInfo& extraData, const std::string& whatArg)
    : SecwException(whatArg, ErrorCode::INVALID_DOCUMENT_FORMAT)
{
    extraData.getMember("documentField") >>= m_documentField;
}

// =====================================================================================================================

void SecwDocumentDoNotExistException::fillSerializationInfo(cxxtools::SerializationInfo& si) const
{
    si.addMember("documentId") <<= m_documentId;
}

SecwDocumentDoNotExistException::SecwDocumentDoNotExistException(const Id& documentId)
    : SecwException("Document '" + documentId + "'does not exist", ErrorCode::DOCUMENT_DO_NOT_EXIST)
    , m_documentId(documentId)
{
}

SecwDocumentDoNotExistException::SecwDocumentDoNotExistException(
    const cxxtools::SerializationInfo& extraData, const std::string& whatArg)
    : SecwException(whatArg, ErrorCode::DOCUMENT_DO_NOT_EXIST)
{
    extraData.getMember("documentId") >>= m_documentId;
}

// =====================================================================================================================

void SecwUnknownUsageIDException::fillSerializationInfo(cxxtools::SerializationInfo& si) const
{
    si.addMember("usageId") <<= m_usageId;
}

SecwUnknownUsageIDException::SecwUnknownUsageIDException(const UsageId& usageId)
    : SecwException("Unknown usage ID '" + usageId + "'", ErrorCode::UNKNOWN_USAGE_ID)
    , m_usageId(usageId)
{
}

SecwUnknownUsageIDException::SecwUnknownUsageIDException(
    const cxxtools::SerializationInfo& extraData, const std::string& whatArg)
    : SecwException(whatArg, ErrorCode::UNKNOWN_USAGE_ID)
{
    extraData.getMember("usageId") >>= m_usageId;
}

// =====================================================================================================================

void SecwNameAlreadyExistsException::fillSerializationInfo(cxxtools::SerializationInfo& si) const
{
    si.addMember("name") <<= m_name;
}

SecwNameAlreadyExistsException::SecwNameAlreadyExistsException(const std::string& name)
    : SecwException("Document with name '" + name + "' already exists", ErrorCode::NAME_ALREADY_EXISTS)
    , m_name(name)
{
}

SecwNameAlreadyExistsException::SecwNameAlreadyExistsException(
    const cxxtools::SerializationInfo& extraData, const std::string& whatArg)
    : SecwException(whatArg, ErrorCode::NAME_ALREADY_EXISTS)
{
    extraData.getMember("name") >>= m_name;
}

// =====================================================================================================================

void SecwNameDoesNotExistException::fillSerializationInfo(cxxtools::SerializationInfo& si) const
{
    si.addMember("name") <<= m_name;
}

SecwNameDoesNotExistException::SecwNameDoesNotExistException(const std::string& name)
    : SecwException("Document with name '" + name + "' does not exist", ErrorCode::NAME_DOES_NOT_EXIST)
    , m_name(name)
{
}

SecwNameDoesNotExistException::SecwNameDoesNotExistException(
    const cxxtools::SerializationInfo& extraData, const std::string& whatArg)
    : SecwException(whatArg, ErrorCode::NAME_DOES_NOT_EXIST)
{
    extraData.getMember("name") >>= m_name;
}

// =====================================================================================================================

} // namespace secw
