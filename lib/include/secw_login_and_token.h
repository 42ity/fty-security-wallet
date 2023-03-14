/*  =========================================================================
    secw_login_and_token - Document parsers for LoginAndToken document
    Copyright (C) 2019 - 2023 Eaton

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

#define LOGIN_AND_TOKEN_TYPE "LoginAndToken"
#include "secw_document.h"

namespace secw {
 
class LoginAndToken;

using LoginAndTokenPtr = std::shared_ptr<LoginAndToken>;

/// Some key definition for serialization
static constexpr const char* DOC_LOGIN_AND_TOKEN_TOKEN = "secw_login_and_token_token";
static constexpr const char* DOC_LOGIN_AND_TOKEN_LOGIN = "secw_login_and_token_login";


class LoginAndToken final : public Document
{
public:
    LoginAndToken();

    LoginAndToken(const std::string& name, const std::string& login = "", const std::string& token = "");

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
    const std::string& getToken() const
    {
        return m_token;
    }
    void setToken(const std::string& token)
    {
        m_token           = token;
        m_containPrivateData = true;
    }

    /// try to cast a document to a LoginAndToken shared ptr
    /// @return shared ptr on LoginAndToken or null shared ptr in case of error
    static LoginAndTokenPtr tryToCast(DocumentPtr doc);

private:
    // Public secw elements
    std::string m_login;


    // Private secw elements
    std::string m_token;

    void fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const override;
    void fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const override;

    void updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
    void updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
};

} // namespace secw
