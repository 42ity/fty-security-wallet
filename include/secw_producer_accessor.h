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

namespace secw
{
  using ClientId = std::string;

  class ClientAccessor;
  
  class ProducerAccessor
  {
  public:
    explicit ProducerAccessor(const ClientId & clientId,
                uint32_t timeout,
                const std::string & endPoint);
    
    std::vector<std::string> getPortfolioList() const;

    std::set<UsageId> getProducerUsages() const;
    
    std::vector<DocumentPtr> getListDocumentsWithoutPrivateData(
      const std::string & portfolio,
      const DocumentType & type = "") const;
    
    DocumentPtr getDocumentWithoutPrivateData(
      const std::string & portfolio,
      const Id & id) const;

    void insertNewDocument(
      const std::string & portfolio,
      const DocumentPtr & doc) const;

    void updateDocument(
      const std::string & portfolio,
      const DocumentPtr & doc) const;

    void deleteDocument(
      const std::string & portfolio,
      const Id & id) const;
  
  private:
    std::shared_ptr<ClientAccessor> m_clientAccessor;
  };
  
} //namespace secw

//  @interface
std::vector<std::pair<std::string,bool>> secw_producer_accessor_test();

#endif
