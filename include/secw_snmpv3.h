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

#ifndef SECW_SNMPV3_H_INCLUDED
#define SECW_SNMPV3_H_INCLUDED

#include "secw_document.h"

#define SNMPV3_TYPE "Snmpv3"

namespace secw
{
    /**
     * \brief snmpv3 implementation
     */

    enum Snmpv3SecurityLevel : uint8_t
    {
        NO_AUTH_NO_PRIV = 0,
        AUTH_NO_PRIV,
        AUTH_PRIV,

        MAX_SECURITY_LEVEL //size of the enum
    };

    enum Snmpv3AuthProtocol : uint8_t
    {
        MD5 = 0,
        SHA,

        MAX_AUTH_PROTOCOL //size of the enum
    };

    enum Snmpv3PrivProtocol : uint8_t
    {
        DES = 0,
        AES,

        MAX_PRIV_PROTOCOL//size of the enum
    };

    /**
     * Some key definition for serialization
     * 
     */
    static constexpr const char* DOC_SNMPV3_SECURITY_LEVEL = "secw_snmpv3_security_level";
    static constexpr const char* DOC_SNMPV3_SECURITY_NAME = "secw_snmpv3_security_name";
    static constexpr const char* DOC_SNMPV3_AUTH_PROTOCOL = "secw_snmpv3_auth_protocol";
    static constexpr const char* DOC_SNMPV3_AUTH_PASSWORD = "secw_snmpv3_auth_password";
    static constexpr const char* DOC_SNMPV3_PRIV_PROTOCOL = "secw_snmpv3_priv_protocol";
    static constexpr const char* DOC_SNMPV3_PRIV_PASSWORD = "secw_snmpv3_priv_password";

    class Snmpv3  final : public Document
    {
    public:

        Snmpv3();

        Snmpv3( const std::string & name,
                Snmpv3SecurityLevel securityLevel = NO_AUTH_NO_PRIV,
                const std::string & securityName = "",
                Snmpv3AuthProtocol authProtocol = MD5,
                const std::string & authPassword = "",
                Snmpv3PrivProtocol privProtocol = DES,
                const std::string & privPassword = "",
                const Id & id = "");

        DocumentPtr clone() const override;

        void validate() const override {} //TODO fill this function

        //Public elements
        Snmpv3SecurityLevel getSecurityLevel() const { return m_securityLevel; }
        Snmpv3AuthProtocol getAuthProtocol() const { return m_authProtocol; }
        Snmpv3PrivProtocol getPrivProtocol() const { return m_privProtocol; }
        const std::string & getSecurityName() const { return m_securityName; }

        //Private elements
        const std::string & getAuthPassword() const { return m_authPassword; }
        const std::string & getPrivPassword() const { return m_privPassword; }

    private:
        //Public elements
        Snmpv3SecurityLevel m_securityLevel = NO_AUTH_NO_PRIV;
        Snmpv3AuthProtocol m_authProtocol = MD5;
        Snmpv3PrivProtocol m_privProtocol = DES;

        //Private elements
        std::string m_securityName = "";
        std::string m_authPassword = "";
        std::string m_privPassword = "";

        void fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const override;
        void fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const override;

        void UpdatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
        void UpdatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
    };

} // namepsace secw


#endif
