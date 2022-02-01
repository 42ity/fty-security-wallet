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

#pragma once
#include "secw_document.h"

#define SNMPV3_TYPE "Snmpv3"

namespace secw {
class Snmpv3;

using Snmpv3Ptr = std::shared_ptr<Snmpv3>;

enum Snmpv3SecurityLevel : uint8_t
{
    NO_AUTH_NO_PRIV = 0,
    AUTH_NO_PRIV,
    AUTH_PRIV,

    MAX_SECURITY_LEVEL // size of the enum
};

enum Snmpv3AuthProtocol : uint8_t
{
    MD5 = 0,
    SHA,
    SHA256,
    SHA384,
    SHA512,

    MAX_AUTH_PROTOCOL // size of the enum
};

enum Snmpv3PrivProtocol : uint8_t
{
    DES = 0,
    AES,
    AES192,
    AES256,

    MAX_PRIV_PROTOCOL // size of the enum
};

/// Some key definition for serialization
static constexpr const char* DOC_SNMPV3_SECURITY_LEVEL = "secw_snmpv3_security_level";
static constexpr const char* DOC_SNMPV3_SECURITY_NAME  = "secw_snmpv3_security_name";
static constexpr const char* DOC_SNMPV3_AUTH_PROTOCOL  = "secw_snmpv3_auth_protocol";
static constexpr const char* DOC_SNMPV3_AUTH_PASSWORD  = "secw_snmpv3_auth_password";
static constexpr const char* DOC_SNMPV3_PRIV_PROTOCOL  = "secw_snmpv3_priv_protocol";
static constexpr const char* DOC_SNMPV3_PRIV_PASSWORD  = "secw_snmpv3_priv_password";

/// snmpv3 implementation
class Snmpv3 final : public Document
{
public:
    Snmpv3();

    Snmpv3(const std::string& name, Snmpv3SecurityLevel securityLevel = NO_AUTH_NO_PRIV,
        const std::string& securityName = "", Snmpv3AuthProtocol authProtocol = MD5,
        const std::string& authPassword = "", Snmpv3PrivProtocol privProtocol = DES,
        const std::string& privPassword = "");

    DocumentPtr clone() const override;

    void validate() const override;

    // Public secw elements
    Snmpv3SecurityLevel getSecurityLevel() const
    {
        return m_securityLevel;
    }
    Snmpv3AuthProtocol getAuthProtocol() const
    {
        return m_authProtocol;
    }
    Snmpv3PrivProtocol getPrivProtocol() const
    {
        return m_privProtocol;
    }
    const std::string& getSecurityName() const
    {
        return m_securityName;
    }

    void setSecurityLevel(Snmpv3SecurityLevel securityLevel)
    {
        m_securityLevel = securityLevel;
    }
    void setAuthProtocol(Snmpv3AuthProtocol authProtocol)
    {
        m_authProtocol = authProtocol;
    }
    void setPrivProtocol(Snmpv3PrivProtocol privProtocol)
    {
        m_privProtocol = privProtocol;
    }
    void setSecurityName(const std::string& securityName)
    {
        m_securityName = securityName;
    }

    // Private secw elements
    const std::string& getAuthPassword() const
    {
        return m_authPassword;
    }
    const std::string& getPrivPassword() const
    {
        return m_privPassword;
    }

    void setAuthPassword(const std::string& authPassword);
    void setPrivPassword(const std::string& privPassword);

    /// @brief try to cast a document to a Snmpv3 shared ptr
    ///
    /// @return shared ptr on Snmpv3 or null shared ptr in case of error
    static Snmpv3Ptr tryToCast(DocumentPtr doc);

private:
    // Public secw elements
    Snmpv3SecurityLevel m_securityLevel = NO_AUTH_NO_PRIV;
    Snmpv3AuthProtocol  m_authProtocol  = MD5;
    Snmpv3PrivProtocol  m_privProtocol  = DES;

    // Private secw elements
    std::string m_securityName = "";
    std::string m_authPassword = "";
    std::string m_privPassword = "";

    void fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const override;
    void fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const override;

    void updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
    void updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
};

} // namespace secw
