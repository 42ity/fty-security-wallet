/*  =========================================================================
    secw_snmpv3 - Document parsers for snmpv3 document

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
                const std::string & privPassword,
                const Id & id) :
        Document(SNMPV3_TYPE),
        m_securityLevel(securityLevel),
        m_authProtocol(authProtocol),
        m_privProtocol(privProtocol),
        m_securityName(securityName),
        m_authPassword(authPassword),
        m_privPassword(privPassword)
    {
        m_name=name;
        m_id=id;
    }

    DocumentPtr Snmpv3::clone() const
    {
        return std::dynamic_pointer_cast<Document>(std::make_shared<Snmpv3>(*this));
    }

//Private
    void Snmpv3::fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const
    {
        si.addMember(DOC_SNMPV3_AUTH_PASSWORD) <<= m_authPassword;
        si.addMember(DOC_SNMPV3_PRIV_PASSWORD) <<= m_privPassword;
    }

    void Snmpv3::fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const
    {
        si.addMember(DOC_SNMPV3_SECURITY_LEVEL) <<= m_securityLevel;
        si.addMember(DOC_SNMPV3_SECURITY_NAME) <<= m_securityName;
        si.addMember(DOC_SNMPV3_AUTH_PROTOCOL) <<= m_authProtocol;
        si.addMember(DOC_SNMPV3_PRIV_PROTOCOL) <<= m_privProtocol;
    }

    void Snmpv3::UpdatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
    {
        try
        {
            si.getMember(DOC_SNMPV3_AUTH_PASSWORD) >>= m_authPassword;
            si.getMember(DOC_SNMPV3_PRIV_PASSWORD) >>= m_privPassword;
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(e.what());
        }
    }

    void Snmpv3::UpdatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
    {
        try
        {
            {
                uint8_t tmp = MAX_SECURITY_LEVEL;
                si.getMember(DOC_SNMPV3_SECURITY_LEVEL) >>= tmp;
                if(tmp >= MAX_SECURITY_LEVEL) throw SecwInvalidDocumentFormatException("Security level invalid value)");
                m_securityLevel = static_cast<Snmpv3SecurityLevel>(tmp);
            }

            {
                si.getMember(DOC_SNMPV3_SECURITY_NAME) >>= m_securityName;
            }

            {
                uint8_t tmp = MAX_AUTH_PROTOCOL;
                si.getMember(DOC_SNMPV3_AUTH_PROTOCOL) >>= tmp;
                if(tmp >= MAX_AUTH_PROTOCOL) throw SecwInvalidDocumentFormatException("Auth protocol invalid value");
                m_authProtocol = static_cast<Snmpv3AuthProtocol>(tmp);
            }

            {
                uint8_t tmp = MAX_PRIV_PROTOCOL;
                si.getMember(DOC_SNMPV3_PRIV_PROTOCOL) >>= tmp;
                if(tmp >= MAX_PRIV_PROTOCOL) throw SecwInvalidDocumentFormatException("Priv Protocol invalid value");
                m_privProtocol = static_cast<Snmpv3PrivProtocol>(tmp);
            }
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(e.what());
        }
    }

} // namespace secw