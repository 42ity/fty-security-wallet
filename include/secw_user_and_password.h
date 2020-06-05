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

#ifndef SECW_USER_AND_PASSWORD_H_INCLUDED
#define SECW_USER_AND_PASSWORD_H_INCLUDED
#define USER_AND_PASSWORD_TYPE "UserAndPassword"

namespace secw
{
    class UserAndPassword;

    using UserAndPasswordPtr   = std::shared_ptr<UserAndPassword>;

    /**
     * Some key definition for serialization
     *
     */
    static constexpr const char* DOC_USER_AND_PASSWORD_USERNAME = "secw_user_and_password_username";
    static constexpr const char* DOC_USER_AND_PASSWORD_PASSWORD = "secw_user_and_password_password";


    class UserAndPassword  final : public Document
    {
    public:

        UserAndPassword();

        UserAndPassword( const std::string & name,
                const std::string & username = "",
                const std::string & password = "");

        DocumentPtr clone() const override;

        void validate() const override;

        //Public secw elements
        const std::string & getUsername() const { return m_username; }
        void setUsername(const std::string & username) { m_username = username; }

        //Private secw elements
        const std::string & getPassword() const { return m_password; }
        void setPassword(const std::string & password) { m_password = password; m_containPrivateData = true; }

        /**
         * \brief try to cast a document to a UserAndPassword shared ptr
         *
         * \return shared ptr on UserAndPassword or null shared ptr in case of error
         */
        static UserAndPasswordPtr tryToCast(DocumentPtr doc);

    private:
        //Public secw elements
        std::string m_username = "";


        //Private secw elements
        std::string m_password = "";

        void fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const override;
        void fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const override;

        void updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
        void updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
    };

} // namepsace secw

#endif
