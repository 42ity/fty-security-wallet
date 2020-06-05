/*  =========================================================================
    secw_snmpv3 - Document parsers for snmpv3 document

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
    secw_snmpv3 - Document parsers for snmpv3 document
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

namespace secw
{
/*-----------------------------------------------------------------------------*/
/*   Snmpv3 Document                                                           */
/*-----------------------------------------------------------------------------*/
//Public
    Snmpv3::Snmpv3() :
        Document(SNMPV3_TYPE)
    {}

    Snmpv3::Snmpv3( const std::string & name,
                Snmpv3SecurityLevel securityLevel,
                const std::string & securityName,
                Snmpv3AuthProtocol authProtocol,
                const std::string & authPassword,
                Snmpv3PrivProtocol privProtocol,
                const std::string & privPassword) :
        Document(SNMPV3_TYPE),
        m_securityLevel(securityLevel),
        m_authProtocol(authProtocol),
        m_privProtocol(privProtocol),
        m_securityName(securityName),
        m_authPassword(authPassword),
        m_privPassword(privPassword)
    {
        m_name=name;
    }

    DocumentPtr Snmpv3::clone() const
    {
        return std::dynamic_pointer_cast<Document>(std::make_shared<Snmpv3>(*this));
    }

    void Snmpv3::setAuthPassword(const std::string & authPassword)
    {
        m_authPassword = authPassword;
        m_containPrivateData = true;
    }

    void Snmpv3::setPrivPassword(const std::string & privPassword)
    {
        m_privPassword = privPassword;
        m_containPrivateData = true;
    }

    void Snmpv3::validate() const
    {
        //at least one password is missing here
        if(!m_containPrivateData)
        {
            if(m_authPassword.empty()) throw SecwInvalidDocumentFormatException(DOC_SNMPV3_AUTH_PASSWORD);
            if(m_privPassword.empty()) throw SecwInvalidDocumentFormatException(DOC_SNMPV3_PRIV_PASSWORD);
        }

        if(m_securityName.empty()) throw SecwInvalidDocumentFormatException(DOC_SNMPV3_SECURITY_NAME);

        if(m_securityLevel == Snmpv3SecurityLevel::AUTH_PRIV)
        {
            if(m_authPassword.empty()) throw SecwInvalidDocumentFormatException(DOC_SNMPV3_AUTH_PASSWORD);
            if(m_privPassword.empty()) throw SecwInvalidDocumentFormatException(DOC_SNMPV3_PRIV_PASSWORD);
        }
        else if(m_securityLevel == Snmpv3SecurityLevel::AUTH_NO_PRIV)
        {
            if(m_authPassword.empty()) throw SecwInvalidDocumentFormatException(DOC_SNMPV3_AUTH_PASSWORD);
        }
    }

//Private
    void Snmpv3::fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const
    {
        if(!m_authPassword.empty())
        {
            si.addMember(DOC_SNMPV3_AUTH_PASSWORD) <<= m_authPassword;
        }

        if(!m_privPassword.empty())
        {
            si.addMember(DOC_SNMPV3_PRIV_PASSWORD) <<= m_privPassword;
        }
    }

    void Snmpv3::fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const
    {
        si.addMember(DOC_SNMPV3_SECURITY_LEVEL) <<= uint8_t(m_securityLevel);
        si.addMember(DOC_SNMPV3_SECURITY_NAME) <<= m_securityName;
        si.addMember(DOC_SNMPV3_AUTH_PROTOCOL) <<= uint8_t(m_authProtocol);
        si.addMember(DOC_SNMPV3_PRIV_PROTOCOL) <<= uint8_t(m_privProtocol);
    }

    void Snmpv3::updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
    {
        try
        {
            const cxxtools::SerializationInfo * authPassword = si.findMember(DOC_SNMPV3_AUTH_PASSWORD);
            if(authPassword != nullptr)
            {
                *authPassword >>= m_authPassword;
            }
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(DOC_SNMPV3_AUTH_PASSWORD);
        }


        try
        {
            const cxxtools::SerializationInfo * authPriv = si.findMember(DOC_SNMPV3_PRIV_PASSWORD);
            if(authPriv != nullptr)
            {
                *authPriv >>= m_privPassword;
            }
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(DOC_SNMPV3_PRIV_PASSWORD);
        }
    }

    void Snmpv3::updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
    {
        try
        {
            uint8_t tmp = MAX_SECURITY_LEVEL;
            si.getMember(DOC_SNMPV3_SECURITY_LEVEL) >>= tmp;
            if(tmp >= MAX_SECURITY_LEVEL) throw SecwInvalidDocumentFormatException(DOC_SNMPV3_SECURITY_LEVEL);
            m_securityLevel = static_cast<Snmpv3SecurityLevel>(tmp);
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(DOC_SNMPV3_SECURITY_LEVEL);
        }

        try
        {
            si.getMember(DOC_SNMPV3_SECURITY_NAME) >>= m_securityName;
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(DOC_SNMPV3_SECURITY_NAME);
        }

        try
        {
            uint8_t tmp = MAX_AUTH_PROTOCOL;
            si.getMember(DOC_SNMPV3_AUTH_PROTOCOL) >>= tmp;
            if(tmp >= MAX_AUTH_PROTOCOL) throw SecwInvalidDocumentFormatException(DOC_SNMPV3_AUTH_PROTOCOL);
            m_authProtocol = static_cast<Snmpv3AuthProtocol>(tmp);
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(DOC_SNMPV3_AUTH_PROTOCOL);
        }

        try
        {
            uint8_t tmp = MAX_PRIV_PROTOCOL;
            si.getMember(DOC_SNMPV3_PRIV_PROTOCOL) >>= tmp;
            if(tmp >= MAX_PRIV_PROTOCOL) throw SecwInvalidDocumentFormatException(DOC_SNMPV3_PRIV_PROTOCOL);
            m_privProtocol = static_cast<Snmpv3PrivProtocol>(tmp);
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(DOC_SNMPV3_PRIV_PROTOCOL);
        }
    }

    Snmpv3Ptr Snmpv3::tryToCast(DocumentPtr doc)
    {
        Snmpv3Ptr ptr(nullptr);

        if((doc != nullptr) && (doc->getType() == SNMPV3_TYPE))
        {
            ptr = std::dynamic_pointer_cast<Snmpv3>(doc);
        }

        return ptr;
    }

} // namespace secw
