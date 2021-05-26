/*  =========================================================================
  secw_document - Document parsers

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

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>

namespace cxxtools {
class SerializationInfo;
}

namespace secw {
class Portfolio;
class Document;

/// Some typedef to make the code more clear
using Id           = std::string;
using UsageId      = std::string;
using Type         = std::string;
using Tag          = std::string;
using DocumentType = std::string;
using DocumentPtr  = std::shared_ptr<Document>;

/// FctDocumentFactory is returning a shared ptr of Doccument after building it using default contructor
using FctDocumentFactory = std::function<DocumentPtr()>;

/// Some key definition for serialization
static constexpr const char* DOC_ID_ENTRY      = "secw_doc_id";
static constexpr const char* DOC_NAME_ENTRY    = "secw_doc_name";
static constexpr const char* DOC_TYPE_ENTRY    = "secw_doc_type";
static constexpr const char* DOC_TAGS_ENTRY    = "secw_doc_tags";
static constexpr const char* DOC_USAGES_ENTRY  = "secw_doc_usages";
static constexpr const char* DOC_PUBLIC_ENTRY  = "secw_doc_public";
static constexpr const char* DOC_PRIVATE_ENTRY = "secw_doc_private";

/// Document: Public interface
class Document
{

    friend class Portfolio; // give the friendship to be able to set an id to new document.

public:
    bool isContainingPrivateData() const;

    const std::string& getName() const;
    void               setName(const std::string& name);

    void          addTag(const Tag& tag);
    void          removeTag(const Tag& tag);
    std::set<Tag> getTags() const;

    void              addUsage(const UsageId& id);
    void              removeUsage(const UsageId& id);
    std::set<UsageId> getUsageIds() const;

    const DocumentType& getType() const;
    const Id&           getId() const;

    /// Clone any document - useful to apply modification before to update
    /// @return shared ptr on a document
    virtual DocumentPtr clone() const = 0;

    /// Destructor
    virtual ~Document()
    {
        /*log_debug("Cleaning document %s", m_name.c_str());*/
    }

    /// Give information if the object document contain private data or only the public data
    /// @return true if the document contain private data
    bool containPrivateData() const;

    /// Append the serialization of the document with header, public and private (secret) part.
    /// @param[in|out] cxxtools::SerializationInfo
    void fillSerializationInfoWithSecret(cxxtools::SerializationInfo& si) const;

    /// Append the serialization of the document with header and public.
    /// @param[in|out] cxxtools::SerializationInfo
    void fillSerializationInfoWithoutSecret(cxxtools::SerializationInfo& si) const;

    /// Append the serialization of the document for SRR.
    /// @param[in|out] cxxtools::SerializationInfo
    /// @param[in] enctyption key use to encrypt private part
    void fillSerializationInfoSRR(cxxtools::SerializationInfo& si, const std::string& encryptionKey) const;

    /// Compare the non secret part of 2 documents
    /// @param[in] Other DocumentPtr
    /// @return true if non secret information are the same
    bool isNonSecretEquals(const DocumentPtr& other) const;

    /// Compare the secret part of 2 documents
    /// @param[in] Other DocumentPtr
    /// @return true if secret information are the same
    bool isSecretEquals(const DocumentPtr& other) const;

    /// Return a document Ptr from a serilization from SRR
    /// @return DocumentPtr
    static DocumentPtr createFromSRR(const cxxtools::SerializationInfo& si, const std::string& encryptionKey);

    /// return the list of all supported types od documents
    /// @return list of types
    static std::vector<DocumentType> getSupportedTypes();

    /// Return if we supported the Type
    /// @param[in] Type for check
    /// @return true if we support the type
    static bool isSupportedType(const DocumentType& type);

    /// Tell if the data in the document are valid: for example password cannot be empty.
    /// @exceptions: In case it's not valid.
    virtual void validate() const = 0;

    /// friend stream operator use to load data from data base
    /// @param[in] serialization info
    /// @param[out] shared pointer on document
    friend void operator>>=(const cxxtools::SerializationInfo& si, DocumentPtr& doc);

protected:
    explicit Document(const DocumentType& type)
        : m_type(type)
    {
    }

    std::string       m_name = "";
    DocumentType      m_type = "";
    Id                m_id   = "";
    std::set<Tag>     m_tags;
    std::set<UsageId> m_usages;

    // This map is use to define the supported type and how to build them
    static std::map<DocumentType, FctDocumentFactory> m_documentFactoryFuntions;

    bool m_containPrivateData = true;

    virtual void fillSerializationInfoPrivateDoc(cxxtools::SerializationInfo& si) const = 0;
    virtual void fillSerializationInfoPublicDoc(cxxtools::SerializationInfo& si) const  = 0;

    virtual void updatePrivateDocFromSerializationInfo(const cxxtools::SerializationInfo& si) = 0;
    virtual void updatePublicDocFromSerializationInfo(const cxxtools::SerializationInfo& si)  = 0;

private:
    void fillSerializationInfoHeaderDoc(cxxtools::SerializationInfo& si) const;

    void updateHeaderFromSerializationInfo(const cxxtools::SerializationInfo& si);
};

// save as fillSerializationInfoWithSecret
void operator<<=(cxxtools::SerializationInfo& si, const Document& doc);
void operator<<=(cxxtools::SerializationInfo& si, const DocumentPtr& doc);
void operator>>=(const cxxtools::SerializationInfo& si, DocumentPtr& doc);

void operator<<=(std::string& str, const Document& doc);
void operator<<=(std::string& str, const DocumentPtr& doc);
void operator<<=(std::string& str, const std::vector<DocumentPtr>& docs);
void operator>>=(const std::string& str, DocumentPtr& doc);

// add a stream operator to display the document in debug for example
// std::ostream& operator<< (std::ostream& os, const DocumentPtr & doc);
std::ostream& operator<<(std::ostream& os, const Document& doc);
std::ostream& operator<<(std::ostream& os, const std::vector<DocumentPtr>& docs);

} // namespace secw
