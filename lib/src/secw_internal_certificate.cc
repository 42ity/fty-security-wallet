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

/*
@header
    secw_internal_certificate - Document parsers for internal certificate document
@discuss
@end
*/

#include "secw_internal_certificate.h"
#include "secw_exception.h"
#include <cxxtools/jsonserializer.h>
#include <fty-lib-certificate.h>
#include <libcert_certificate_X509.h>

namespace secw {
/*-----------------------------------------------------------------------------*/
/*   InternalCertificate Document                                                           */
/*-----------------------------------------------------------------------------*/
// Public
InternalCertificate::InternalCertificate()
    : Document(INTERNAL_CERTIFICATE_TYPE)
{
}

InternalCertificate::InternalCertificate(
    const std::string& name, const std::string& pem, const std::string& privateKeyPem)
    : Document(INTERNAL_CERTIFICATE_TYPE)
    , m_pem(pem)
    , m_privateKeyPem(privateKeyPem)
{
    m_name = name;
}

DocumentPtr InternalCertificate::clone() const
{
    return std::dynamic_pointer_cast<Document>(std::make_shared<InternalCertificate>(*this));
}

void InternalCertificate::validate() const
{
    if (!m_containPrivateData)
        throw SecwInvalidDocumentFormatException(DOC_INTERNAL_CERTIFICATE_PRIVATE_KEY_PEM);
    if (m_pem.empty())
        throw SecwInvalidDocumentFormatException(DOC_INTERNAL_CERTIFICATE_PEM);
    if (m_privateKeyPem.empty())
        throw SecwInvalidDocumentFormatException(DOC_INTERNAL_CERTIFICATE_PRIVATE_KEY_PEM);

    //#Check the certificate
    try {
        fty::CertificateX509 cert(m_pem);
        fty::Keys            keys(m_privateKeyPem);

        if (keys.getPublicKey() != cert.getPublicKey())
            throw SecwInvalidDocumentFormatException(DOC_INTERNAL_CERTIFICATE_PEM);
    } catch (...) {
        throw SecwInvalidDocumentFormatException(DOC_INTERNAL_CERTIFICATE_PEM);
    }
}

// Private
void InternalCertificate::fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const
{
    if (!m_privateKeyPem.empty()) {
        si.addMember(DOC_INTERNAL_CERTIFICATE_PRIVATE_KEY_PEM) <<= m_privateKeyPem;
    }
}

void InternalCertificate::fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const
{
    si.addMember(DOC_INTERNAL_CERTIFICATE_PEM) <<= m_pem;
}

void InternalCertificate::updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
{
    try {
        const cxxtools::SerializationInfo* privateKey = si.findMember(DOC_INTERNAL_CERTIFICATE_PRIVATE_KEY_PEM);
        if (privateKey != nullptr) {
            *privateKey >>= m_privateKeyPem;
        }
    } catch (const std::exception& /*e*/) {
        throw SecwInvalidDocumentFormatException(DOC_INTERNAL_CERTIFICATE_PRIVATE_KEY_PEM);
    }
}

void InternalCertificate::updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
{
    try {
        si.getMember(DOC_INTERNAL_CERTIFICATE_PEM) >>= m_pem;
    } catch (const std::exception& /*e*/) {
        throw SecwInvalidDocumentFormatException(DOC_INTERNAL_CERTIFICATE_PEM);
    }
}

InternalCertificatePtr InternalCertificate::tryToCast(DocumentPtr doc)
{
    InternalCertificatePtr ptr(nullptr);

    if ((doc != nullptr) && (doc->getType() == INTERNAL_CERTIFICATE_TYPE)) {
        ptr = std::dynamic_pointer_cast<InternalCertificate>(doc);
    }

    return ptr;
}

} // namespace secw
