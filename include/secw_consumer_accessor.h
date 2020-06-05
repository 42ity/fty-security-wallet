/*  =========================================================================
  secw_consumer_accessor - Accessor to return documents from the agent

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

#ifndef SECW_CONSUMER_ACCESSOR_H_INCLUDED
#define SECW_CONSUMER_ACCESSOR_H_INCLUDED

#include <functional>

#include "fty_common_client.h"

namespace mlm
{
    class MlmStreamClient;
}

namespace fty
{
    class SocketSyncClient;
}

namespace secw
{
  using ClientId = std::string;

    /**
     * @brief Callback for create notification
     *
     * @param portfolio name
     * @param created document
     */
  using CreatedCallback = std::function<void(const std::string&, DocumentPtr)> ;
    /**
     * @brief Callback for update notification
     *
     * @param portfolio name
     * @param old document
     * @param new document
     */
  using UpdatedCallback = std::function<void(const std::string&, DocumentPtr, DocumentPtr)> ;
    /**
     * @brief Callback for delete notification
     *
     * @param portfolio name
     * @param deleted document
     */
  using DeletedCallback = std::function<void(const std::string&, DocumentPtr)> ;
    /**
     * @brief Callback for startup notification
     */
  using StartedCallback = std::function<void()>;

  class ClientAccessor;

  /**
   * @brief Give consumer access:
   * A consumer has a list of usageId define by the server configuration.
   * A consumer can read to documents which have a usageId in the list of the customer usageId.
   * A consumer can read to the private part of the documents.
   *
   * A customer cannot do any modification on the document in the server.
   * A customer cannot do creation of document in the server.
   *
   * @exception For exceptions list, see secw_exception.h
   *
   */
  class ConsumerAccessor
  {
  public:

    explicit ConsumerAccessor( fty::SocketSyncClient & requestClient);
    explicit ConsumerAccessor( fty::SocketSyncClient & requestClient, mlm::MlmStreamClient & subscriberClient);

    /**
     * @brief Get the List of portfolio name
     *
     * @return std::vector<std::string>
     */
    std::vector<std::string> getPortfolioList() const;

    /**
     * @brief Get the List of usages that the consumer can access
     * @param portfolio name (default value "default")
     *
     * @return std::set<UsageId>
     */
    std::set<UsageId> getConsumerUsages(const std::string & portfolioName = "default") const;

    /**
     * @brief Get the List Documents With Private Data
     *
     * @param portfolio name
     * @param usageId (optional)
     * @return std::vector<DocumentPtr>
     */
    std::vector<DocumentPtr> getListDocumentsWithPrivateData(
      const std::string & portfolio,
      const UsageId & usageId = "") const;

    /**
     * @brief Get the List Documents With Private Data from a list of id.
     *
     * If a document cannot be retrived (bad id or none access right), this document will not be on the list.
     *
     * @param portfolio name
     * @param list of id requested
     * @return std::vector<DocumentPtr> contain the documents which have been retrieved.
     */
    std::vector<DocumentPtr> getListDocumentsWithPrivateData(
      const std::string & portfolio,
      const std::vector<Id> & ids) const;

    /**
     * @brief Get a Document With Private Data object
     *
     * @param portfolio name
     * @param id of the document
     * @return DocumentPtr on the document.
     */
    DocumentPtr getDocumentWithPrivateData(
      const std::string & portfolio,
      const Id & id) const;


    /**
     * @brief Get a Document With Private Data object
     *
     * @param portfolio name
     * @param name of the document
     * @return DocumentPtr on the document.
     */
    DocumentPtr getDocumentWithPrivateDataByName(
      const std::string & portfolio,
      const std::string & name) const;

    /**
     * @brief Set callback for update notification
     *
     * @param callback
     */
    void setCallbackOnUpdate(UpdatedCallback updatedCallback = nullptr);
    /**
     * @brief Set callback for create notification
     *
     * @param callback
     */
    void setCallbackOnCreate(CreatedCallback createdCallback= nullptr);
    /**
     * @brief Set callback for delete notification
     *
     * @param callback
     */
    void setCallbackOnDelete(DeletedCallback deletedCallback= nullptr);
    /**
     * @brief Set callback for startup notification
     *
     * @param callback
     */
    void setCallbackOnStart(StartedCallback startedCallback= nullptr);


  private:
    std::shared_ptr<ClientAccessor> m_clientAccessor;

  };

} //namespace secw

//  @interface
std::vector<std::pair<std::string,bool>> secw_consumer_accessor_test(fty::SocketSyncClient & syncClient, mlm::MlmStreamClient & streamClient);

#endif
