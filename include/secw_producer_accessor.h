/*  =========================================================================
    secw_producer_accessor - Accessor to return documents from the agent

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

#ifndef SECW_PRODUCER_ACCESSOR_H_INCLUDED
#define SECW_PRODUCER_ACCESSOR_H_INCLUDED

#include "secw_document.h"

namespace secw
{
  using ClientId = std::string;
  
  using CreatedCallback = std::function<void(const std::string&, DocumentPtr)> ;
  using UpdatedCallback = std::function<void(const std::string&, DocumentPtr, DocumentPtr)> ;
  using DeletedCallback = std::function<void(const std::string&, DocumentPtr)> ;
  using StartedCallback = std::function<void()>;

  class ClientAccessor;
  
  /**
   * @brief Give Producer access: 
   * A producer has a list of usageId define by the server configuration.
   * A producer can retrieve any documents from the server.
   * A producer can add or remove in usageIds of the document only if those usageids are in the list define for him.
   * A producer can write any part of a document part of the documents.
   * 
   * A customer can only read the public part of document.
   * A customer can add a new document in the server.
   * 
   * @exception For exceptions list, see secw_exception.h
   * 
   */
  class ProducerAccessor
  {
  public:
  /**
   * @brief Construct a new Producer Accessor
   * 
   * @param clientId 
   * @param timeout 
   * @param endPoint 
   */
    explicit ProducerAccessor(const ClientId & clientId,
                uint32_t timeout,
                const std::string & endPoint);
    
    /**
     * @brief Get the List of Portfolio name
     * 
     * @return std::vector<std::string> 
     */
    std::vector<std::string> getPortfolioList() const;

    /**
     * @brief Get the List of usages that the producer can use
     * 
     * @return std::set<UsageId> 
     */
    std::set<UsageId> getProducerUsages(const std::string & portfolioName = "default") const;
    
    /**
     * @brief Get the List Documents Without Private Data
     * 
     * @param portfolio name
     * @param usageId (optional)
     * @return std::vector<DocumentPtr> 
     */
    std::vector<DocumentPtr> getListDocumentsWithoutPrivateData(
      const std::string & portfolio,
      const UsageId & usageId = "") const;

    /**
     * @brief Get the List Documents Without Private Data from a list of id.
     * 
     * If a document cannot be retrived (bad id), this document will not be on the list.
     * 
     * @param portfolio name
     * @param list of id requested
     * @return std::vector<DocumentPtr> contain the documents which have been retrieved.
     */
    std::vector<DocumentPtr> getListDocumentsWithoutPrivateData(
      const std::string & portfolio,
      const std::vector<Id> & ids ) const;
    
    /**
     * @brief Get a Document Without Private Data object
     * 
     * @param portfolio name
     * @param id of the document
     * @return DocumentPtr on the document.
     */
    DocumentPtr getDocumentWithoutPrivateData(
      const std::string & portfolio,
      const Id & id) const;

    /**
     * @brief Get a Document Without Private Data object
     * 
     * @param portfolio name
     * @param name of the document
     * @return DocumentPtr on the document.
     */
    DocumentPtr getDocumentWithoutPrivateDataByName(
      const std::string & portfolio,
      const std::string & name) const;

    /**
     * @brief Insert a new document into the server database
     * 
     * @param portfolio name
     * @param document
     * @return Id of the new document
     */
    Id insertNewDocument(
      const std::string & portfolio,
      const DocumentPtr & doc) const;

    /**
     * @brief Update a document into the server database. The document must exist into the database.
     * 
     * @param portfolio name
     * @param document
     */
    void updateDocument(
      const std::string & portfolio,
      const DocumentPtr & doc) const;

    /**
     * @brief Update a document from the server database. The document must exist into the database.
     * 
     * @param portfolio name
     * @param id of the document to be removed
     */
    void deleteDocument(
      const std::string & portfolio,
      const Id & id) const;

    void setCallbackOnUpdate(UpdatedCallback updatedCallback = nullptr);
    void setCallbackOnCreate(CreatedCallback createdCallback= nullptr);
    void setCallbackOnDelete(DeletedCallback deletedCallback= nullptr);
    void setCallbackOnStart(StartedCallback startedCallback= nullptr);
  
  private:
    std::shared_ptr<ClientAccessor> m_clientAccessor;
  };
  
} //namespace secw

//  @interface
std::vector<std::pair<std::string,bool>> secw_producer_accessor_test();

#endif
