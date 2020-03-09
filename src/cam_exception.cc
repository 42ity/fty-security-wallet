/*  =========================================================================
    cam_exception - Credential asset mapping exceptions

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

#include <sstream>
#include <cxxtools/jsondeserializer.h>
#include <cxxtools/jsonserializer.h>

#include "cam_exception.h"

namespace cam
{
/*-----------------------------------------------------------------------------*/
/*   Exception                                                                 */
/*-----------------------------------------------------------------------------*/
//Public
    void CamException::throwCamException(const std::string & data)
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
            case UNSUPPORTED_COMMAND:           throw CamUnsupportedCommandException(whatArg);
            case PROTOCOL_ERROR:                throw CamProtocolErrorException(whatArg);
            case BAD_COMMAND_ARGUMENT:          throw CamBadCommandArgumentException(extraData, whatArg);
            case MAPPING_DOES_NOT_EXIST:          throw CamMappingDoesNotExistException(extraData, whatArg);
            case MAPPING_ALREADY_EXISTS:         throw CamMappingAlreadyExistsException(extraData, whatArg);
            case MAPPING_INVALID:               throw CamMappingInvalidException(whatArg);
            default: throw CamException(whatArg);
        }
    }

    CamException::CamException(const std::string & whatArg, ErrorCode code) :
        m_code(code),
        m_whatArg(whatArg)
    {}

    CamException::CamException(ErrorCode code) :
        m_code(code),
        m_whatArg("Unknown error")
    {}
    
    const char* CamException::what() const noexcept
    {
        return m_whatArg.c_str();
    }

    std::string CamException::toJson() const
    {
        std::stringstream output;
        cxxtools::JsonSerializer serializer(output);
        serializer.beautify(true);

        cxxtools::SerializationInfo si;
        si <<= *(this);
        serializer.serialize(si);

        return output.str();
    }

    void operator<<= (cxxtools::SerializationInfo& si, const CamException & exception)
    {
        si.addMember("errorCode") <<= uint8_t(exception.m_code);
        si.addMember("whatArg") <<= exception.m_whatArg;
        exception.fillSerializationInfo(si.addMember("extraData"));
    }

//Protected 
    void CamException::fillSerializationInfo(cxxtools::SerializationInfo& /*si*/) const
    {}

} // namespace cam

