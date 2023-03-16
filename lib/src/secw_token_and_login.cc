/*  =========================================================================
    secw_login_and_token - Document parsers for TokenAndLogin document

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
    secw_token_and_login - Document parsers for TokenAndLogin document
@discuss
@end
*/

#include "secw_token_and_login.h"
#include "secw_exception.h"
#include "secw_utf8_cxxtools.h"
#include <cxxtools/jsonserializer.h>

namespace secw {
/*-----------------------------------------------------------------------------*/
/*   TokenAndLogin Document                                                              */
/*-----------------------------------------------------------------------------*/
// Public
TokenAndLogin::TokenAndLogin()
    : Document(TOKEN_AND_LOGIN_TYPE)
{
}

TokenAndLogin::TokenAndLogin(const std::string& name, const std::string& login, const std::string& token)
    : Document(TOKEN_AND_LOGIN_TYPE)
    , m_login(login)
    , m_token(token)
{
    m_name = name;
}

DocumentPtr TokenAndLogin::clone() const
{
    return std::dynamic_pointer_cast<Document>(std::make_shared<TokenAndLogin>(*this));
}

void TokenAndLogin::validate() const
{
    if (!m_containPrivateData)
        throw SecwInvalidDocumentFormatException(DOC_TOKEN_AND_LOGIN_TOKEN);
    if (m_token.empty())
        throw SecwInvalidDocumentFormatException(DOC_TOKEN_AND_LOGIN_TOKEN);
}

// Private
void TokenAndLogin::fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const
{  
    if (!m_token.empty()) {
        si.addMember(DOC_TOKEN_AND_LOGIN_TOKEN) <<= StdStringToCxxString(m_token);
    }
}

void TokenAndLogin::fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const
{
    if(!m_login.empty()) {
        si.addMember(DOC_TOKEN_AND_LOGIN_LOGIN) <<= StdStringToCxxString(m_login);
    }
}

void TokenAndLogin::updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
{
    try {
        const cxxtools::SerializationInfo* token = si.findMember(DOC_TOKEN_AND_LOGIN_TOKEN);
        if (token != nullptr) {
            m_token = GetSiMemberCxxString(si, DOC_TOKEN_AND_LOGIN_TOKEN);
        }
    } catch (const std::exception& e) {
        throw SecwInvalidDocumentFormatException(DOC_TOKEN_AND_LOGIN_TOKEN);
    }
}

void TokenAndLogin::updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
{
    try {
        const cxxtools::SerializationInfo* login = si.findMember(DOC_TOKEN_AND_LOGIN_LOGIN);
        if (login != nullptr) {
            m_login = GetSiMemberCxxString(si, DOC_TOKEN_AND_LOGIN_LOGIN);
        }
    } catch (const std::exception& e) {
        throw SecwInvalidDocumentFormatException(DOC_TOKEN_AND_LOGIN_LOGIN);
    }
}

TokenAndLoginPtr TokenAndLogin::tryToCast(DocumentPtr doc)
{
    TokenAndLoginPtr ptr(nullptr);

    if ((doc != nullptr) && (doc->getType() == TOKEN_AND_LOGIN_TYPE)) {
        ptr = std::dynamic_pointer_cast<TokenAndLogin>(doc);
    }

    return ptr;
}

} // namespace secw
