/*  =========================================================================
    secw_external_certificate - Document parsers for external certificate document

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
    secw_external_certificate - Document parsers for external certificate document
@discuss
@end
*/
#include "fty_security_wallet_classes.h"

#include "libcert_certificate_X509.h"

namespace secw
{
/*-----------------------------------------------------------------------------*/
/*   ExternalCertificate Document                                                           */
/*-----------------------------------------------------------------------------*/
//Public
    ExternalCertificate::ExternalCertificate() :
        Document(EXTERNAL_CERTIFICATE_TYPE)
    {}

    ExternalCertificate::ExternalCertificate( const std::string & name,
                const std::string & pem) :
        Document(EXTERNAL_CERTIFICATE_TYPE),
        m_pem(pem)
    {
        m_name=name;
    }

    DocumentPtr ExternalCertificate::clone() const
    {
        return std::dynamic_pointer_cast<Document>(std::make_shared<ExternalCertificate>(*this));
    }

    void ExternalCertificate::validate() const
    {
        //if(m_containPrivateData) throw SecwInvalidDocumentFormatException(DOC_EXTERNAL_CERTIFICATE_PEM);
        if(m_pem.empty()) throw SecwInvalidDocumentFormatException(DOC_EXTERNAL_CERTIFICATE_PEM);

        //check the certificate
        try
        {
            fty::CertificateX509 cert(m_pem);
        }
        catch(...)
        {
            throw SecwInvalidDocumentFormatException(DOC_EXTERNAL_CERTIFICATE_PEM);
        }

    }

//Private
    void ExternalCertificate::fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& /*si*/) const
    {
    }

    void ExternalCertificate::fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const
    {
        si.addMember(DOC_EXTERNAL_CERTIFICATE_PEM) <<= m_pem;
    }

    void ExternalCertificate::updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& /*si*/)
    {
    }

    void ExternalCertificate::updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
    {
        try
        {
            si.getMember(DOC_EXTERNAL_CERTIFICATE_PEM) >>= m_pem;
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(DOC_EXTERNAL_CERTIFICATE_PEM);
        }
    }

    ExternalCertificatePtr ExternalCertificate::tryToCast(DocumentPtr doc)
    {
        ExternalCertificatePtr ptr(nullptr);

        if((doc != nullptr) && (doc->getType() == EXTERNAL_CERTIFICATE_TYPE))
        {
            ptr = std::dynamic_pointer_cast<ExternalCertificate>(doc);
        }

        return ptr;
    }

} // namespace secw