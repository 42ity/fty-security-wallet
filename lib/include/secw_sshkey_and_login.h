/*  =========================================================================
    secw_sshkey_and_login - Document parsers for SshKeyAndLogin document
    Copyright (C) 2023 Eaton

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

#define SSHKEY_AND_LOGIN_TYPE "SshKeyAndLogin"
#include "secw_document.h"

namespace secw {
 
class SshKeyAndLogin;

using SshKeyAndLoginPtr = std::shared_ptr<SshKeyAndLogin>;

/// Some key definition for serialization
static constexpr const char* DOC_SSHKEY_AND_LOGIN_SSHKEY = "secw_sshkey_and_login_sshkey";
static constexpr const char* DOC_SSHKEY_AND_LOGIN_LOGIN = "secw_sshkey_and_login_login";


class SshKeyAndLogin final : public Document
{
public:
    SshKeyAndLogin();

    SshKeyAndLogin(const std::string& name, const std::string& sshKey = "", const std::string& login = "");

    DocumentPtr clone() const override;

    void validate() const override;

    // Public secw elements
    const std::string& getLogin() const
    {
        return m_login;
    }
    void setLogin(const std::string& login)
    {
        m_login = login;
    }

    // Private secw elements
    const std::string& getSshKey() const
    {
        return m_sshKey;
    }
    void setSshKey(const std::string& sshKey)
    {
        m_sshKey           = sshKey;
        m_containPrivateData = true;
    }

    /// try to cast a document to a SshKeyAndLogin shared ptr
    /// @return shared ptr on SshKeyAndLogin or null shared ptr in case of error
    static SshKeyAndLoginPtr tryToCast(DocumentPtr doc);

private:
    // Private secw elements
    std::string m_sshKey;

    // Public secw elements
    std::string m_login;

    void fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const override;
    void fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const override;

    void updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
    void updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
};

} // namespace secw
