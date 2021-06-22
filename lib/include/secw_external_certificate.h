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

#pragma once

#define EXTERNAL_CERTIFICATE_TYPE "ExternalCertificate"
#include "secw_document.h"

namespace secw {
class ExternalCertificate;

using ExternalCertificatePtr = std::shared_ptr<ExternalCertificate>;

/**
 * Some key definition for serialization
 *
 */
static constexpr const char* DOC_EXTERNAL_CERTIFICATE_PEM = "secw_external_certificate_pem";


class ExternalCertificate final : public Document
{
public:
    ExternalCertificate();

    ExternalCertificate(const std::string& name, const std::string& pem = "");

    DocumentPtr clone() const override;

    void validate() const override;

    // Public secw elements
    const std::string& getPem() const
    {
        return m_pem;
    }
    void setPem(const std::string& pem)
    {
        m_pem = pem;
    }

    /// try to cast a document to a UserAndPassword shared ptr
    /// @return shared ptr on UserAndPassword or null shared ptr in case of error
    static ExternalCertificatePtr tryToCast(DocumentPtr doc);

private:
    // Public secw elements
    std::string m_pem;

    void fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const override;
    void fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const override;

    void updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
    void updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si) override;
};

} // namespace secw
