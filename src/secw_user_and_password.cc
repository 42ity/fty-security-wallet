/*  =========================================================================
    secw_user_and_password - Document parsers for user and password document

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
    secw_user_and_password - Document parsers for user and password document
@discuss
@end
*/

#include "fty_security_wallet_classes.h"
namespace secw
{
/*-----------------------------------------------------------------------------*/
/*   UserAndPassword Document                                                           */
/*-----------------------------------------------------------------------------*/
//Public
    UserAndPassword::UserAndPassword() :
        Document(USER_AND_PASSWORD_TYPE)
    {}

    UserAndPassword::UserAndPassword( const std::string & name,
                const std::string & username,
                const std::string & password) :
        Document(USER_AND_PASSWORD_TYPE),
        m_username(username),
        m_password(password)
    {
        m_name=name;
    }

    DocumentPtr UserAndPassword::clone() const
    {
        return std::dynamic_pointer_cast<Document>(std::make_shared<UserAndPassword>(*this));
    }

    void UserAndPassword::validate() const
    {
        if(!m_containPrivateData) throw SecwInvalidDocumentFormatException("Private part is missing");
        if(m_username.empty()) throw SecwInvalidDocumentFormatException("Username is empty");
        if(m_password.empty()) throw SecwInvalidDocumentFormatException("Password is empty");
    }

//Private
    void UserAndPassword::fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const
    {
        if(!m_password.empty())
        {
            si.addMember(DOC_USER_AND_PASSWORD_PASSWORD) <<= m_password;
        }
    }

    void UserAndPassword::fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const
    {
        si.addMember(DOC_USER_AND_PASSWORD_USERNAME) <<= m_username;
    }

    void UserAndPassword::UpdatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
    {
        try
        {
            const cxxtools::SerializationInfo * password = si.findMember(DOC_USER_AND_PASSWORD_PASSWORD);
            if(password != nullptr)
            {
                *password >>= m_password;
            }
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(e.what());
        }
    }

    void UserAndPassword::UpdatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
    {
        try
        {
            si.getMember(DOC_USER_AND_PASSWORD_USERNAME) >>= m_username;
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(e.what());
        }
    }

    UserAndPasswordPtr UserAndPassword::tryToCast(DocumentPtr doc)
    {
        UserAndPasswordPtr ptr(nullptr);

        if((doc != nullptr) && (doc->getType() == USER_AND_PASSWORD_TYPE))
        {
            ptr = std::dynamic_pointer_cast<UserAndPassword>(doc);
        }

        return ptr;
    }

} // namespace secw