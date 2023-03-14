/*  =========================================================================
    secw_login_and_token - Document parsers for LoginAndToken document

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
    secw_login_and_token - Document parsers for LoginAndToken document
@discuss
@end
*/

#include "secw_login_and_token.h"
#include "secw_exception.h"
#include "secw_utf8_cxxtools.h"
#include <cxxtools/jsonserializer.h>

namespace secw {
/*-----------------------------------------------------------------------------*/
/*   LoginAndToken Document                                                              */
/*-----------------------------------------------------------------------------*/
// Public
LoginAndToken::LoginAndToken()
    : Document(LOGIN_AND_TOKEN_TYPE)
{
}

LoginAndToken::LoginAndToken(const std::string& name, const std::string& login, const std::string& token)
    : Document(LOGIN_AND_TOKEN_TYPE)
    , m_login(login)
    , m_token(token)
{
    m_name = name;
}

DocumentPtr LoginAndToken::clone() const
{
    return std::dynamic_pointer_cast<Document>(std::make_shared<LoginAndToken>(*this));
}

void LoginAndToken::validate() const
{
    if (!m_containPrivateData)
        throw SecwInvalidDocumentFormatException(DOC_LOGIN_AND_TOKEN_TOKEN);
    if (m_token.empty())
        throw SecwInvalidDocumentFormatException(DOC_LOGIN_AND_TOKEN_TOKEN);
}

// Private
void LoginAndToken::fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const
{  
    if (!m_token.empty()) {
        si.addMember(DOC_LOGIN_AND_TOKEN_TOKEN) <<= StdStringToCxxString(m_token);
    }
}

void LoginAndToken::fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const
{
    if(!m_login.empty()) {
        si.addMember(DOC_LOGIN_AND_TOKEN_LOGIN) <<= StdStringToCxxString(m_login);
    }
}

void LoginAndToken::updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
{
    try {
        const cxxtools::SerializationInfo* token = si.findMember(DOC_LOGIN_AND_TOKEN_TOKEN);
        if (token != nullptr) {
            m_token = GetSiMemberCxxString(si, DOC_LOGIN_AND_TOKEN_TOKEN);
        }
    } catch (const std::exception& e) {
        throw SecwInvalidDocumentFormatException(DOC_LOGIN_AND_TOKEN_TOKEN);
    }
}

void LoginAndToken::updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
{
    try {
        const cxxtools::SerializationInfo* login = si.findMember(DOC_LOGIN_AND_TOKEN_LOGIN);
        if (login != nullptr) {
            m_login = GetSiMemberCxxString(si, DOC_LOGIN_AND_TOKEN_LOGIN);
        }
    } catch (const std::exception& e) {
        throw SecwInvalidDocumentFormatException(DOC_LOGIN_AND_TOKEN_LOGIN);
    }
}

LoginAndTokenPtr LoginAndToken::tryToCast(DocumentPtr doc)
{
    LoginAndTokenPtr ptr(nullptr);

    if ((doc != nullptr) && (doc->getType() == LOGIN_AND_TOKEN_TYPE)) {
        ptr = std::dynamic_pointer_cast<LoginAndToken>(doc);
    }

    return ptr;
}

} // namespace secw
