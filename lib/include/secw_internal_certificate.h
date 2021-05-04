/*  =========================================================================
    secw_internal_certificate - Document parsers for internal certificate document

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

#ifndef SECW_INTERNAL_CERTIFICATE_H_INCLUDED
#define SECW_INTERNAL_CERTIFICATE_H_INCLUDED

#define INTERNAL_CERTIFICATE_TYPE "InternalCertificate"

namespace secw
{
    class InternalCertificate;

    using InternalCertificatePtr   = std::shared_ptr<InternalCertificate>;

    /**
     * Some key definition for serialization
     *
     */
    static constexpr const char* DOC_INTERNAL_CERTIFICATE_PEM = "secw_internal_certificate_pem";
    static constexpr const char* DOC_INTERNAL_CERTIFICATE_PRIVATE_KEY_PEM = "secw_internal_certificate_private_key_pem";


    class InternalCertificate  final : public Document
    {
    public:

        InternalCertificate();

        InternalCertificate( const std::string & name,
                const std::string & pem = "",
                const std::string & privateKeyPem = "");

        DocumentPtr clone() const override;

        void validate() const override;

        //Public secw elements
        const std::string & getPem() const { return m_pem; }
        void setPem(const std::string & pem) { m_pem = pem; }

        //Private secw elements
        const std::string & getPrivateKeyPem() const { return m_privateKeyPem; }
        void setPrivateKeyPem(const std::string & privateKeyPem) { m_privateKeyPem = privateKeyPem; }

        /**
         * \brief try to cast a document to a UserAndPassword shared ptr
         *
         * \return shared ptr on UserAndPassword or null shared ptr in case of error
         */
        static InternalCertificatePtr tryToCast(DocumentPtr doc);

    private:
        //Public secw elements
        std::string m_pem;

        //Private secw elements
        std::string m_privateKeyPem;

        void fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const override;
        void fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const override;

        void updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
        void updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
    };

} // namepsace secw

#endif
