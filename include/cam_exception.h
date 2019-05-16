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

#ifndef CAM_EXCEPTION_H_INCLUDED
#define CAM_EXCEPTION_H_INCLUDED


#include <exception>
#include <string>

#include <cxxtools/serializationinfo.h>


namespace cam
{
    enum ErrorCode : uint8_t
    {
        GENERIC = 0,
        UNSUPPORTED_COMMAND,
        PROTOCOL_ERROR,
        BAD_COMMAND_ARGUMENT,
        MAPPING_DO_NOT_EXIST,
        MAPPING_ALREADY_EXIST,
    };

    class CamException : public std::exception
    {
    public:
        explicit CamException(const std::string & whatArg, ErrorCode code = ErrorCode::GENERIC);
        
        virtual ~CamException(){}
        
        const char* what() const noexcept override;

        inline ErrorCode getErrorCode() const { return m_code; }

        //return an error payload from an exception
        std::string toJson() const;

        friend void operator<<= (cxxtools::SerializationInfo& si, const CamException & exception);

        //throw the good exception base on the error payload
        static void throwCamException(const std::string & data);
        
    protected:
        virtual void fillSerializationInfo(cxxtools::SerializationInfo& si) const;
        
    private:
        ErrorCode m_code;
        std::string m_whatArg; 

    };

    void operator<<= (cxxtools::SerializationInfo& si, const CamException & exception);

// Command is not supported
    class CamUnsupportedCommandException : public CamException
    {
    public:
        explicit CamUnsupportedCommandException(const std::string & whatArg) :
            CamException(whatArg, ErrorCode::UNSUPPORTED_COMMAND)
        {}
    };

// Wrong message format => the message do not comply to the protocol
    class CamProtocolErrorException : public CamException
    {
    public:
        explicit CamProtocolErrorException(const std::string & whatArg) :
            CamException(whatArg, ErrorCode::PROTOCOL_ERROR)
        {}
    };


// Arguments for the command are not good
    class CamBadCommandArgumentException : public CamException
    {
    public:
        explicit CamBadCommandArgumentException(const std::string & whatArg) :
            CamException(whatArg, ErrorCode::BAD_COMMAND_ARGUMENT)
        {}
    };
    
// Mapping do not exist
    class CamMappingDoNotExistException : public CamException
    {
    public:
        explicit CamMappingDoNotExistException(const std::string & whatArg) :
            CamException(whatArg, ErrorCode::MAPPING_DO_NOT_EXIST)
        {}
    };

// Mapping already exist
    class CamMappingAlreadyExistException : public CamException
    {
    public:
        explicit CamMappingAlreadyExistException(const std::string & whatArg) :
            CamException(whatArg, ErrorCode::MAPPING_ALREADY_EXIST)
        {}
    };

} // namepsace cam

#endif
