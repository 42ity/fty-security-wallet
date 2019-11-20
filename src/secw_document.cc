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
#include "secw_external_certificate.h"
#include "secw_internal_certificate.h"

#include "secw_helpers.h"

#include <cxxtools/jsonserializer.h>
#include <cxxtools/jsondeserializer.h>

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
    { USER_AND_PASSWORD_TYPE, []() { return DocumentPtr(new UserAndPassword()); }},
    { EXTERNAL_CERTIFICATE_TYPE, []() { return DocumentPtr(new ExternalCertificate()); }},
    { INTERNAL_CERTIFICATE_TYPE, []() { return DocumentPtr(new InternalCertificate()); }}
};

//Public
    const std::string & Document::getName() const
    { 
        return m_name;
    }

    void Document::setName(const std::string & name)
    {
        m_name = name;
    }

    void Document::addTag(const Tag & tag)
    {
        m_tags.insert(tag);
    }

    void Document::removeTag(const Tag & tag)
    {
        if(m_tags.count(tag) > 0)
        {
            m_tags.erase(tag);
        }
    }

    std::set<Tag> Document::getTags() const
    { 
        return m_tags;
    }

    void Document::addUsage(const UsageId & id)
    {
        m_usages.insert(id);
    }

    void Document::removeUsage(const UsageId & id)
    {
        if(m_usages.count(id) > 0)
        {
            m_usages.erase(id);
        }
    }

    std::set<UsageId> Document::getUsageIds() const
    { 
        return m_usages;
    }

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

    void Document::fillSerializationInfoSRR(cxxtools::SerializationInfo& si, const std::string & encryptionKey) const
    {
        fillSerializationInfoHeaderDoc(si);
        fillSerializationInfoPublicDoc(si.addMember(DOC_PUBLIC_ENTRY));
        cxxtools::SerializationInfo & privateSi = si.addMember(DOC_PRIVATE_ENTRY);

        privateSi.addMember("format") <<= "ENC";
        cxxtools::SerializationInfo subSi;

        fillSerializationInfoPrivateDoc(subSi);

        std::string dataToEncrypt = serialize(subSi);

        privateSi.addMember("data") <<= encrypt(dataToEncrypt, encryptionKey);
    }

    DocumentPtr Document::createFromSRR(const cxxtools::SerializationInfo& si, const std::string & encryptionKey)
    {
        DocumentPtr doc;
        try
        {
            Id id;
            DocumentType type;

            si.getMember(DOC_TYPE_ENTRY) >>= type;
            si.getMember(DOC_ID_ENTRY) >>= id;

            const cxxtools::SerializationInfo & publicEntry = si.getMember(DOC_PUBLIC_ENTRY);
            const cxxtools::SerializationInfo & privateSection = si.getMember(DOC_PRIVATE_ENTRY);

            doc = Document::m_documentFactoryFuntions.at(type)();
            doc->m_id = id;
            
            //log_debug("Create document '%s' matching with '%s'", doc->getType().c_str(), type.c_str());

            doc->updateHeaderFromSerializationInfo(si);
            doc->updatePublicDocFromSerializationInfo(publicEntry);

            std::string format;
            privateSection.getMember("format") >>= format;

            if(format == "plaintext")
            {
                const cxxtools::SerializationInfo & privateEntry = privateSection.getMember("data");
                doc->updatePrivateDocFromSerializationInfo(privateEntry);
            }
            else if (format == "ENC")
            {
                std::string encryptedData;
                privateSection.getMember("data") >>= encryptedData;

                cxxtools::SerializationInfo privateEntry = deserialize(decrypt(encryptedData, encryptionKey));

                doc->updatePrivateDocFromSerializationInfo(privateEntry);
            }
            else
            {
                throw SecwException("Bad data format");
            }  

            doc->m_containPrivateData = true;

        }
        catch(const SecwException & e)
        {
            throw;
        }
        catch(const std::exception& e)
        {
            throw SecwException(e.what());
        }

        return doc;
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

    void Document::updateHeaderFromSerializationInfo(const cxxtools::SerializationInfo& si)
    {
        try
        {
            //We don't read the id because will portfolio insertion process is gonna do it
            //We don't read the type because it is define by the object type
            si.getMember(DOC_NAME_ENTRY) >>= m_name;
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(DOC_NAME_ENTRY);
        }

        try
        {
            si.getMember(DOC_TAGS_ENTRY) >>= m_tags;
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(DOC_TAGS_ENTRY);
        }

        try
        {
            si.getMember(DOC_USAGES_ENTRY) >>= m_usages;
        }
        catch(const std::exception& e)
        {
            throw SecwInvalidDocumentFormatException(DOC_USAGES_ENTRY);
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
            
            bool documentExist = (doc != nullptr);

            si.getMember(DOC_TYPE_ENTRY) >>= type;
            si.getMember(DOC_ID_ENTRY) >>= id;

            const cxxtools::SerializationInfo & publicEntry = si.getMember(DOC_PUBLIC_ENTRY);
            const cxxtools::SerializationInfo * privateEntry = si.findMember(DOC_PRIVATE_ENTRY);
            
            //if we override and existing document, check that we have the same type and same id 
            if(documentExist)
            {
               if(doc->getType() != type)
               {
                   throw SecwInvalidDocumentFormatException(DOC_TYPE_ENTRY);
               }
               
               if(doc->getId() != id)
               {
                   throw SecwInvalidDocumentFormatException(DOC_ID_ENTRY);
               }
            }
            else
            {
                //document do not exist, so create one
                doc = Document::m_documentFactoryFuntions.at(type)();
                doc->m_id = id;
            }
            
            //log_debug("Create document '%s' matching with '%s'", doc->getType().c_str(), type.c_str());

            doc->updateHeaderFromSerializationInfo(si);
            doc->updatePublicDocFromSerializationInfo(publicEntry);
            
            if(privateEntry != nullptr)
            {
                doc->updatePrivateDocFromSerializationInfo(*privateEntry);
                
                if(!documentExist)
                {
                    doc->m_containPrivateData = true;
                }
            }
            else
            {
                if(!documentExist)
                {
                    doc->m_containPrivateData = false;
                }
            } 

        }
        catch(const SecwException & e)
        {
            throw;
        }
        catch(const std::exception& e)
        {
            throw SecwException(e.what());
        }

    }

    void operator<<= (std::string& str, const Document & doc)
    {
        cxxtools::SerializationInfo si;
        si <<= doc;

        try
        {
            std::stringstream output;

            cxxtools::JsonSerializer serializer(output);
            serializer.beautify(true);
            serializer.serialize(si);

            str = output.str();
        }
        catch(const std::exception& e)
        {
            throw SecwException("Error while creating json "+std::string(e.what()));
        }
    }

    void operator<<= (std::string& str, const DocumentPtr & doc)
    {
        str <<= *doc;
    }

    void operator>>= (const std::string& str, DocumentPtr & doc)
    {
        cxxtools::SerializationInfo si;

        try
        {
            std::stringstream input;
            input << str;
            cxxtools::JsonDeserializer deserializer(input);
            deserializer.deserialize(si);
        }
        catch(const std::exception& e)
        {
            throw SecwProtocolErrorException("Error in json: "+std::string(e.what()));
        }

        si >>= doc;
    }

    void operator<<= (std::string& str, const std::vector<DocumentPtr> & docs)
    {
        cxxtools::SerializationInfo si;
        si <<= docs;

        try
        {
            std::stringstream output;

            cxxtools::JsonSerializer serializer(output);
            serializer.beautify(true);
            serializer.serialize(si);

            str = output.str();
        }
        catch(const std::exception& e)
        {
            throw SecwException("Error while creating json "+std::string(e.what()));
        }
    }

    /*std::ostream& operator<< (std::ostream& os, const DocumentPtr & doc)
    {
        os << *(doc);
        return os;
    }*/

    std::ostream& operator<< (std::ostream& os, const Document & doc)
    {
        std::string data;
        data <<= doc;
        os << data << std::endl;

        return os;
    }

    std::ostream& operator<< (std::ostream& os, const std::vector<DocumentPtr> & docs)
    {
        std::string data;
        data <<= docs;
        os << data << std::endl;

        return os;
    }

} // namespace secw

