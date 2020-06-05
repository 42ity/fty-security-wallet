/*  =========================================================================
    secw_snmpv1 - Document parsers for snmpv1 document

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
    secw_snmpv1 - Document parsers for snmpv1 document
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

namespace secw
{
/*-----------------------------------------------------------------------------*/
/*   Snmpv1 Document                                                           */
/*-----------------------------------------------------------------------------*/
//Public
    Snmpv1::Snmpv1() :
        Document(SNMPV1_TYPE)
    {}

    Snmpv1::Snmpv1( const std::string & name,
                const std::string & communityName) :
        Document(SNMPV1_TYPE),
        m_communityName(communityName)
    {
        m_name=name;
    }

    DocumentPtr Snmpv1::clone() const
    {
        return std::dynamic_pointer_cast<Document>(std::make_shared<Snmpv1>(*this));
    }

//Private
    void Snmpv1::fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const
    {

    }

    void Snmpv1::fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const
    {
        si.addMember(DOC_SNMPV1_COMMUNITY_NAME) <<= m_communityName;
    }

    void Snmpv1::updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
    {
        try
        {
            si.getMember(DOC_SNMPV1_COMMUNITY_NAME) >>= m_communityName;
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(DOC_SNMPV1_COMMUNITY_NAME);
        }
    }

    void Snmpv1::updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si)
    {
    }

    Snmpv1Ptr Snmpv1::tryToCast(DocumentPtr doc)
    {
        Snmpv1Ptr ptr(nullptr);

        if((doc != nullptr) && (doc->getType() == SNMPV1_TYPE))
        {
            ptr = std::dynamic_pointer_cast<Snmpv1>(doc);
        }

        return ptr;
    }

    void Snmpv1::validate() const
    {
        if(m_communityName.empty()) throw SecwInvalidDocumentFormatException(DOC_SNMPV1_COMMUNITY_NAME);
    }

} // namespace secw

