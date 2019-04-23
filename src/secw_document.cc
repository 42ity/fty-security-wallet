/*  =========================================================================
    secw_document - Document parsers

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
    secw_document - Document parsers
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

#include "secw_snmpv3.h"
#include "secw_snmpv1.h"
#include "secw_user_and_password.h"

namespace secw
{
/*-----------------------------------------------------------------------------*/
/*   Document                                                                  */
/*-----------------------------------------------------------------------------*/
//Here are the supported documents - Need to be update if you want to add some
std::map<DocumentType, FctDocumentFactory> Document::m_documentFactoryFuntions =
{
    { SNMPV3_TYPE, []() { return DocumentPtr(new Snmpv3()); }},
    { SNMPV1_TYPE, []() { return DocumentPtr(new Snmpv1()); }},
    { USER_AND_PASSWORD_TYPE, []() { return DocumentPtr(new UserAndPassword()); }}
};

//Public
    const std::string & Document::getName() const { return m_name; }
    std::vector<Tag> Document::getTags() const { return m_tags; }
    std::set<UsageId> Document::getUsageIds() const { return m_usages; }
    const DocumentType & Document::getType() const { return m_type; }
    const Id & Document::getId() const { return m_id; }

    bool Document::isContainingPrivateData() const {return m_containPrivateData; }

    bool Document::isSupportedType(const DocumentType & type)
    {
        return (m_documentFactoryFuntions.count(type) > 0);
    }

    std::vector<DocumentType> Document::getSupportedTypes()
    {
        std::vector<DocumentType> types;

        for( const auto & item : m_documentFactoryFuntions )
        {
            types.push_back(item.first);
        }
        
        return types;
    }

    void Document::fillSerializationInfoWithoutSecret(cxxtools::SerializationInfo& si) const
    {
        fillSerializationInfoHeaderDoc(si);
        fillSerializationInfoPublicDoc(si.addMember(DOC_PUBLIC_ENTRY));
    }

    void Document::fillSerializationInfoWithSecret(cxxtools::SerializationInfo& si) const
    {
        fillSerializationInfoHeaderDoc(si);
        fillSerializationInfoPublicDoc(si.addMember(DOC_PUBLIC_ENTRY));
        fillSerializationInfoPrivateDoc(si.addMember(DOC_PRIVATE_ENTRY));
    }

//Private 
    void Document::fillSerializationInfoHeaderDoc(cxxtools::SerializationInfo& si) const
    {
        si.addMember(DOC_ID_ENTRY) <<= getId();
        si.addMember(DOC_NAME_ENTRY) <<= getName();
        si.addMember(DOC_TYPE_ENTRY) <<= getType();
        si.addMember(DOC_TAGS_ENTRY) <<= getTags();
        si.addMember(DOC_USAGES_ENTRY) <<= getUsageIds();
    }

    void Document::UpdateHeaderFromSerializationInfo(const cxxtools::SerializationInfo& si)
    {
        try
        {
            //We don't read the id because will portfolio insertion process is gonna do it
            //We don't read the type because it is define by the object type
            si.getMember(DOC_NAME_ENTRY) >>= m_name;
            si.getMember(DOC_TAGS_ENTRY) >>= m_tags;
            si.getMember(DOC_USAGES_ENTRY) >>= m_usages;
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(e.what());
        }
    }

    void operator<<= (cxxtools::SerializationInfo& si, const Document & doc)
    {
        doc.fillSerializationInfoWithSecret(si);
    }

    void operator<<= (cxxtools::SerializationInfo& si, const DocumentPtr & doc)
    {
        doc->fillSerializationInfoWithSecret(si);
    }

    void operator>>= (const cxxtools::SerializationInfo& si, DocumentPtr & doc)
    {
        try
        {
            Id id = "";
            DocumentType type = "";

            si.getMember(DOC_TYPE_ENTRY) >>= type;
            si.getMember(DOC_ID_ENTRY) >>= id;

            const cxxtools::SerializationInfo & publicEntry = si.getMember(DOC_PUBLIC_ENTRY);
            const cxxtools::SerializationInfo * privateEntry = si.findMember(DOC_PRIVATE_ENTRY);
            
            doc = Document::m_documentFactoryFuntions.at(type)();
            
            //log_debug("Create document '%s' matching with '%s'", doc->getType().c_str(), type.c_str());

            doc->UpdateHeaderFromSerializationInfo(si);
            doc->UpdatePublicDocFromSerializationInfo(publicEntry);
            
            if(privateEntry != nullptr)
            {
                doc->UpdatePrivateDocFromSerializationInfo(*privateEntry);
                doc->m_containPrivateData = true;
            }
            else
            {
                doc->m_containPrivateData = false;
            }
            
            doc->m_id = id;

        }
        catch(const SecwException & e)
        {
            throw;
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(e.what());
        }

    }

} // namespace secw

