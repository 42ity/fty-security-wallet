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

/*
@header
    secw_sshkey_and_login - Document parsers for SshKeyAndLogin document
@discuss
@end
*/

#include "secw_sshkey_and_login.h"
#include "secw_exception.h"
#include "secw_utf8_cxxtools.h"
#include <cxxtools/jsonserializer.h>

namespace secw {
/*-----------------------------------------------------------------------------*/
/*   SshKeyAndLogin Document                                                    */
/*-----------------------------------------------------------------------------*/
// Public
SshKeyAndLogin::SshKeyAndLogin()
    : Document(SSHKEY_AND_LOGIN_TYPE)
{
}

SshKeyAndLogin::SshKeyAndLogin(const std::string& name, const std::string& sshKey, const std::string& login)
    : Document(SSHKEY_AND_LOGIN_TYPE)
    , m_sshKey(sshKey)
    , m_login(login)
{
    m_name = name;
}

DocumentPtr SshKeyAndLogin::clone() const
{
    return std::dynamic_pointer_cast<Document>(std::make_shared<SshKeyAndLogin>(*this));
}

void SshKeyAndLogin::validate() const
{
    if (!m_containPrivateData) {
        throw SecwInvalidDocumentFormatException(DOC_SSHKEY_AND_LOGIN_SSHKEY);
    }
    if ( m_sshKey.empty() ) {
        throw SecwInvalidDocumentFormatException(DOC_SSHKEY_AND_LOGIN_SSHKEY);
    }
}

// Private
void SshKeyAndLogin::fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const
{  
    if ( !m_sshKey.empty() ) {
        si.addMember(DOC_SSHKEY_AND_LOGIN_SSHKEY) <<= StdStringToCxxString(m_sshKey);
    }
}

void SshKeyAndLogin::fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const
{
    if( !m_login.empty() ) {
        si.addMember(DOC_SSHKEY_AND_LOGIN_LOGIN) <<= StdStringToCxxString(m_login);
    }
}

void SshKeyAndLogin::updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
{
    try {
        const cxxtools::SerializationInfo* sshKey = si.findMember(DOC_SSHKEY_AND_LOGIN_SSHKEY);
        if (sshKey != nullptr) {
            m_sshKey = GetSiMemberCxxString(si, DOC_SSHKEY_AND_LOGIN_SSHKEY);
        }
    } catch (const std::exception& /*e*/) {
        throw SecwInvalidDocumentFormatException(DOC_SSHKEY_AND_LOGIN_SSHKEY);
    }
}

void SshKeyAndLogin::updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
{
    try {
        const cxxtools::SerializationInfo* login = si.findMember(DOC_SSHKEY_AND_LOGIN_LOGIN);
        if (login != nullptr) {
            m_login = GetSiMemberCxxString(si, DOC_SSHKEY_AND_LOGIN_LOGIN);
        }
    } catch (const std::exception& /*e*/) {
        throw SecwInvalidDocumentFormatException(DOC_SSHKEY_AND_LOGIN_LOGIN);
    }
}

SshKeyAndLoginPtr SshKeyAndLogin::tryToCast(DocumentPtr doc)
{
    SshKeyAndLoginPtr ptr(nullptr);

    if ((doc != nullptr) && (doc->getType() == SSHKEY_AND_LOGIN_TYPE)) {
        ptr = std::dynamic_pointer_cast<SshKeyAndLogin>(doc);
    }

    return ptr;
}

} // namespace secw
