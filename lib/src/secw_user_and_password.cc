/*  =========================================================================
    secw_user_and_password - Document parsers for user and password document

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
    secw_user_and_password - Document parsers for user and password document
@discuss
@end
*/

#include "secw_user_and_password.h"
#include "secw_exception.h"
#include "secw_utf8_cxxtools.h"
#include <cxxtools/jsonserializer.h>

namespace secw {
/*-----------------------------------------------------------------------------*/
/*   UserAndPassword Document                                                  */
/*-----------------------------------------------------------------------------*/
// Public
UserAndPassword::UserAndPassword()
    : Document(USER_AND_PASSWORD_TYPE)
{
}

UserAndPassword::UserAndPassword(const std::string& name, const std::string& username, const std::string& password)
    : Document(USER_AND_PASSWORD_TYPE)
    , m_username(username)
    , m_password(password)
{
    m_name = name;
}

DocumentPtr UserAndPassword::clone() const
{
    return std::dynamic_pointer_cast<Document>(std::make_shared<UserAndPassword>(*this));
}

void UserAndPassword::validate() const
{
    if (!m_containPrivateData)
        throw SecwInvalidDocumentFormatException(DOC_USER_AND_PASSWORD_PASSWORD);
    if (m_username.empty())
        throw SecwInvalidDocumentFormatException(DOC_USER_AND_PASSWORD_USERNAME);
    if (m_password.empty())
        throw SecwInvalidDocumentFormatException(DOC_USER_AND_PASSWORD_PASSWORD);
}

// Private
void UserAndPassword::fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const
{
    if (!m_password.empty()) {
        si.addMember(DOC_USER_AND_PASSWORD_PASSWORD) <<= StdStringToCxxString(m_password);
    }
}

void UserAndPassword::fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const
{
    si.addMember(DOC_USER_AND_PASSWORD_USERNAME) <<= StdStringToCxxString(m_username);
}

void UserAndPassword::updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
{
    try {
        const cxxtools::SerializationInfo* password = si.findMember(DOC_USER_AND_PASSWORD_PASSWORD);
        if (password != nullptr) {
            m_password = GetSiMemberCxxString(si, DOC_USER_AND_PASSWORD_PASSWORD);
        }
    } catch (const std::exception& e) {
        throw SecwInvalidDocumentFormatException(DOC_USER_AND_PASSWORD_PASSWORD);
    }
}

void UserAndPassword::updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
{
    try {
        m_username = GetSiMemberCxxString(si, DOC_USER_AND_PASSWORD_USERNAME);
    } catch (const std::exception& e) {
        throw SecwInvalidDocumentFormatException(DOC_USER_AND_PASSWORD_USERNAME);
    }
}

UserAndPasswordPtr UserAndPassword::tryToCast(DocumentPtr doc)
{
    UserAndPasswordPtr ptr(nullptr);

    if ((doc != nullptr) && (doc->getType() == USER_AND_PASSWORD_TYPE)) {
        ptr = std::dynamic_pointer_cast<UserAndPassword>(doc);
    }

    return ptr;
}

} // namespace secw
