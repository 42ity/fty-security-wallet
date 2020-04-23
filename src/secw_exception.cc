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

#include "fty_security_wallet_classes.h"

#include <sstream>
#include <cxxtools/jsondeserializer.h>
#include <cxxtools/jsonserializer.h>

namespace secw
{
/*-----------------------------------------------------------------------------*/
/*   Exception                                                                 */
/*-----------------------------------------------------------------------------*/
//Public
    void SecwException::throwSecwException(const std::string & data)
    {
        //get the serializationInfo
        std::stringstream input;
        input << data;

        cxxtools::SerializationInfo si;
        cxxtools::JsonDeserializer deserializer(input);
        deserializer.deserialize(si);

        //extract the error code, the message and the extra data
        uint8_t errorCode = 0;
        std::string whatArg;
        cxxtools::SerializationInfo extraData;
        
        try
        {
            si.getMember("errorCode") >>= errorCode;
            si.getMember("whatArg") >>= whatArg;
            extraData = si.getMember("extraData");
        }
        catch(...)
        {}
              
        switch(errorCode)
        {
            case UNSUPPORTED_COMMAND:           throw SecwUnsupportedCommandException(whatArg);
            case PROTOCOL_ERROR:                throw SecwProtocolErrorException(whatArg);
            case BAD_COMMAND_ARGUMENT:          throw SecwBadCommandArgumentException(whatArg);
            case UNKNOWN_DOCUMENT_TYPE:         throw SecwUnknownDocumentTypeException(extraData, whatArg);
            case UNKNOWN_PORTFOLIO:             throw SecwUnknownPortfolioException(extraData, whatArg);
            case INVALID_DOCUMENT_FORMAT:       throw SecwInvalidDocumentFormatException(extraData, whatArg);
            case IMPOSSIBLE_TO_LOAD_PORTFOLIO:  throw SecwImpossibleToLoadPortfolioException(whatArg);
            case UNKNOWN_TAG:                   throw SecwUnknownTagException(whatArg);
            case DOCUMENT_DO_NOT_EXIST:         throw SecwDocumentDoNotExistException(extraData, whatArg);
            case ILLEGAL_ACCESS:                throw SecwIllegalAccess(whatArg);
            case UNKNOWN_USAGE_ID:              throw SecwUnknownUsageIDException(extraData, whatArg);
            case NAME_ALREADY_EXISTS:           throw SecwNameAlreadyExistsException(extraData, whatArg);
            case NAME_DOES_NOT_EXIST:          throw SecwNameDoesNotExistException(extraData, whatArg);
            default: throw SecwException(whatArg);
        }
    }

    SecwException::SecwException(const std::string & whatArg, ErrorCode code) :
        m_code(code),
        m_whatArg(whatArg)
    {}
    
    const char* SecwException::what() const noexcept
    {
        return m_whatArg.c_str();
    }

    std::string SecwException::toJson() const
    {
        std::stringstream output;
        cxxtools::JsonSerializer serializer(output);
        serializer.beautify(true);

        cxxtools::SerializationInfo si;
        si <<= *(this);
        serializer.serialize(si);

        return output.str();
    }

    void operator<<= (cxxtools::SerializationInfo& si, const SecwException & exception)
    {
        si.addMember("errorCode") <<= exception.m_code;
        si.addMember("whatArg") <<= exception.m_whatArg;
        exception.fillSerializationInfo(si.addMember("extraData"));
    }

//Protected 
    void SecwException::fillSerializationInfo(cxxtools::SerializationInfo& /*si*/) const
    {}



} // namespace secw

