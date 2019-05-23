/*  =========================================================================
    secw_exception - secw exception

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

#ifndef SECW_EXCEPTION_H_INCLUDED
#define SECW_EXCEPTION_H_INCLUDED

#include <exception>
#include <string>

#include <cxxtools/serializationinfo.h>

namespace secw
{
    enum ErrorCode : uint8_t
    {
        GENERIC = 0,
        UNSUPPORTED_COMMAND,
        PROTOCOL_ERROR,
        BAD_COMMAND_ARGUMENT,
        UNKNOWN_DOCUMENT_TYPE,
        UNKNOWN_PORTFOLIO,
        INVALID_DOCUMENT_FORMAT,
        IMPOSSIBLE_TO_LOAD_PORTFOLIO,
        UNKNOWN_TAG,
        DOCUMENT_DO_NOT_EXIST,
        ILLEGAL_ACCESS,
    };

    class SecwException : public std::exception
    {
    public:
        explicit SecwException(const std::string & whatArg, ErrorCode code = ErrorCode::GENERIC);
        
        virtual ~SecwException(){}
        
        const char* what() const noexcept override;

        inline ErrorCode getErrorCode() const { return m_code; }

        //return an error payload from an exception
        std::string toJson() const;

        friend void operator<<= (cxxtools::SerializationInfo& si, const SecwException & exception);

        //throw the good exception base on the error payload
        static void throwSecwException(const std::string & data);
        
    protected:
        virtual void fillSerializationInfo(cxxtools::SerializationInfo& si) const;
        
    private:
        ErrorCode m_code;
        std::string m_whatArg; 

    };

    void operator<<= (cxxtools::SerializationInfo& si, const SecwException & exception);

// Command is not supported
    class SecwUnsupportedCommandException : public SecwException
    {
    public:
        explicit SecwUnsupportedCommandException(const std::string & whatArg) :
            SecwException(whatArg, ErrorCode::UNSUPPORTED_COMMAND)
        {}
    };

// Wrong message format => the message do not comply to the protocol
    class SecwProtocolErrorException : public SecwException
    {
    public:
        explicit SecwProtocolErrorException(const std::string & whatArg) :
            SecwException(whatArg, ErrorCode::PROTOCOL_ERROR)
        {}
    };


// Arguments for the command are not good
    class SecwBadCommandArgumentException : public SecwException
    {
    public:
        explicit SecwBadCommandArgumentException(const std::string & whatArg) :
            SecwException(whatArg, ErrorCode::BAD_COMMAND_ARGUMENT)
        {}
    };

// Type of document is unknown
    class SecwUnknownDocumentTypeException : public SecwException
    {
    public:
        explicit SecwUnknownDocumentTypeException(const std::string & whatArg) :
            SecwException(whatArg, ErrorCode::UNKNOWN_DOCUMENT_TYPE)
        {}
    };

// Portfolio is unknown
    class SecwUnknownPortfolioException : public SecwException
    {
    private:
        std::string m_portfolioName;

        void fillSerializationInfo(cxxtools::SerializationInfo& si) const override
        {
            si.addMember("portfolioName") <<= m_portfolioName;
        }

    public:
        explicit SecwUnknownPortfolioException(const std::string & portfolioName) :
            SecwException("Unknown portfolio '"+portfolioName+"'", ErrorCode::UNKNOWN_PORTFOLIO),
            m_portfolioName(portfolioName)
        {}

        SecwUnknownPortfolioException(const cxxtools::SerializationInfo& extraData, const std::string & whatArg) :
            SecwException(whatArg, ErrorCode::UNKNOWN_PORTFOLIO)
        {
            extraData.getMember("portfolioName") >>= m_portfolioName;
        }

        inline std::string getPortfolioName() const { return m_portfolioName; }
    };

// Invalid format of document
    class SecwInvalidDocumentFormatException : public SecwException
    {
    public:
        explicit SecwInvalidDocumentFormatException(const std::string & whatArg) :
            SecwException(whatArg, ErrorCode::INVALID_DOCUMENT_FORMAT)
        {}
    };

// Impossible to load the portfolio
    class SecwImpossibleToLoadPortfolioException : public SecwException
    {
    public:
        explicit SecwImpossibleToLoadPortfolioException(const std::string & whatArg) :
            SecwException(whatArg, ErrorCode::IMPOSSIBLE_TO_LOAD_PORTFOLIO)
        {}
    };
    
// Tag is unknown
    class SecwUnknownTagException : public SecwException
    {
    public:
        explicit SecwUnknownTagException(const std::string & whatArg) :
            SecwException(whatArg, ErrorCode::UNKNOWN_TAG)
        {}
    };
    
// document do not exist
    class SecwDocumentDoNotExistException : public SecwException
    {
    public:
        explicit SecwDocumentDoNotExistException(const std::string & whatArg) :
            SecwException(whatArg, ErrorCode::DOCUMENT_DO_NOT_EXIST)
        {}
    };
    
// Illegal action by the client
    class SecwIllegalAccess : public SecwException
    {
    public:
        explicit SecwIllegalAccess(const std::string & whatArg) :
            SecwException(whatArg, ErrorCode::ILLEGAL_ACCESS)
        {}
    };

} // namepsace secw

#endif
