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

/*
@header
    secw_producer_accessor - Accessor to return documents from the agent
@discuss
@end
*/
#include "fty_security_wallet_classes.h"
namespace secw
{
  ProducerAccessor::ProducerAccessor(	const ClientId & clientId,
                                      uint32_t timeout,
                                      const std::string & endPoint):
    m_clientAccessor(std::make_shared<ClientAccessor>(clientId, timeout, endPoint))
  {}

  std::vector<std::string> ProducerAccessor::getPortfolioList() const
  {
    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_PORTFOLIO_LIST,{});

    if(frames.size() < 1)
    {
      throw SecwProtocolErrorException("Empty answer from server");
    }

    cxxtools::SerializationInfo si = deserialize(frames.at(0));

    std::vector<std::string> portfolioNames;

    si >>= portfolioNames;

    return portfolioNames;
  }

  std::set<UsageId> ProducerAccessor::getProducerUsages() const
  {
    std::set<UsageId> usages;

    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_PRODUCER_USAGES,{});

    if(frames.size() < 1)
    {
      throw SecwProtocolErrorException("Empty answer from server");
    }

    cxxtools::SerializationInfo si = deserialize(frames.at(0));

    si >>= usages;

    return usages;
  }

  std::vector<DocumentPtr> ProducerAccessor::getListDocumentsWithoutPrivateData(
    const std::string & portfolio,
    const DocumentType & type) const
  {
    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_LIST_WITHOUT_SECRET, {portfolio,type});
    
    //the first frame should contain the data
    if(frames.size() < 1)
    {
      throw SecwProtocolErrorException("Empty answer from server");
    }

    cxxtools::SerializationInfo si = deserialize(frames.at(0));
    
    std::vector<DocumentPtr> documents;
    
    si >>= documents;
    
    return documents;
  }

  DocumentPtr ProducerAccessor::getDocumentWithoutPrivateData(
    const std::string & portfolio,
    const Id & id) const
  {
    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_WITHOUT_SECRET, {portfolio,id});
    
    //the first frame should contain the data
    if(frames.size() < 1)
    {
      throw SecwProtocolErrorException("Empty answer from server");
    }

    cxxtools::SerializationInfo si = deserialize(frames.at(0));
    
    DocumentPtr document;
    
    si >>= document;
    
    return document;
  }

  void ProducerAccessor::insertNewDocument(
    const std::string & portfolio,
    const DocumentPtr & doc) const
  {
    cxxtools::SerializationInfo si;
    si <<= doc;
    std::string jsonDoc = serialize(si);

    //create
    m_clientAccessor->sendCommand(SecurityWalletServer::CREATE, {portfolio,jsonDoc});
  }  

  void ProducerAccessor::updateDocument(
    const std::string & portfolio,
    const DocumentPtr & doc) const
  {
    cxxtools::SerializationInfo si;
    si <<= doc;
    std::string jsonDoc = serialize(si);
    //update
    m_clientAccessor->sendCommand(SecurityWalletServer::UPDATE, {portfolio,jsonDoc});

  }  

  void ProducerAccessor::deleteDocument(
    const std::string & portfolio,
    const Id & id) const
  {
    m_clientAccessor->sendCommand(SecurityWalletServer::DELETE, {portfolio,id});
  }

} //namespace secw

//  --------------------------------------------------------------------------
//  Test of this class => This is used by fty_security_wallet_server_test
//  --------------------------------------------------------------------------

#define SELFTEST_CLIENT_ID "secw-client-test"

std::vector<std::pair<std::string,bool>> secw_producer_accessor_test()
{
  std::vector<std::pair<std::string,bool>> testsResults;

  static const char* endpoint = "inproc://fty-security-walletg-test";
  
  using namespace secw;

  printf(" ** secw_producer_accessor_test: \n");

//test 1.1 => getPortfolioList
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #1.1 getPortfolioList\n");
    ProducerAccessor producerAccessor(SELFTEST_CLIENT_ID, 1000, endpoint);
    try
    {
      std::vector<std::string> portfolio = producerAccessor.getPortfolioList();

      if(portfolio.at(0) != "default")
      {
        throw std::runtime_error("Portfolio default is not in the list of portfolio");
      }
      
      printf(" *<=  Test #1.1 > OK\n");
      
      testsResults.emplace_back (" Test #1.1 getPortfolioList",true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #1.1 > Failed\n");
      printf("Error: %s\n\n",e.what());
      
      testsResults.emplace_back (" Test #1.1 getPortfolioList",false);
    }

  }

//test 1.2  => SecwUnknownPortfolioException
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #1.2 SecwUnknownPortfolioException\n");
    ProducerAccessor producerAccessor(SELFTEST_CLIENT_ID, 1000, endpoint);
    try
    {
      producerAccessor.getDocumentWithoutPrivateData("XXXXX", "XXXXX-XXXXXXXXX");

      throw std::runtime_error("Document is return");
    }
    catch(const SecwUnknownPortfolioException &)
    {
      printf(" *<=  Test #1.2 > OK\n");
      testsResults.emplace_back (" Test #1.2 SecwUnknownPortfolioException",true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #1.2 > Failed\n");
      printf("Error: %s\n\n",e.what());
      testsResults.emplace_back (" Test #1.2 SecwUnknownPortfolioException",false);
    }
  }

//test 2.1 => getProducerUsages
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #2.1 getProducerUsages\n");
    ProducerAccessor producerAccessor(SELFTEST_CLIENT_ID, 1000, endpoint);
    try
    {
      std::set<std::string> usages = producerAccessor.getProducerUsages();

      if(usages.size() != 2)
      {
        throw std::runtime_error("Wrong number of customer use returned");
      }

      if(usages.count("discovery_monitoring") == 0)
      {
        throw std::runtime_error("Usage 'discovery_monitoring' is not in the list of producer usages");
      }

      if(usages.count("mass_device_management") == 0)
      {
        throw std::runtime_error("Usage 'mass_device_management' is not in the list of producer usages");
      }
      
      printf(" *<=  Test #2.1 > OK\n");
      
      testsResults.emplace_back (" Test #2.1 getProducerUsages",true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #2.1 > Failed\n");
      printf("Error: %s\n\n",e.what());
      
      testsResults.emplace_back (" Test #2.1 getProducerUsages",false);
    }

  }

//test 3.1 => getListDocumentsWithoutPrivateData
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #3.1 getListDocumentsWithoutPrivateData\n");
    ProducerAccessor producerAccessor(SELFTEST_CLIENT_ID, 1000, endpoint);
    try
    {
      std::vector<DocumentPtr> doc = producerAccessor.getListDocumentsWithoutPrivateData("default");

      if(doc.size() != 2)
      {
        throw std::runtime_error("Not the good number of documents: expected 2, received "+std::to_string(doc.size()));
      }
      
      if(doc[0]->isContainingPrivateData())
      {
        throw std::runtime_error("Document is containing private data");
      }
      
      printf(" *<=  Test #3.1 > OK\n");
      testsResults.emplace_back (" Test #3.1 getListDocumentsWithoutPrivateData",true);

    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #3.1 > Failed\n");
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #3.1 getListDocumentsWithoutPrivateData",false);
    }
  }

//test 3.2 => getListDocumentsWithoutPrivateData usage="discovery_monitoring"
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #3.2 getListDocumentsWithoutPrivateData usage=discovery_monitoring");
    ProducerAccessor producerAccessor(SELFTEST_CLIENT_ID, 1000, endpoint);
    try
    {
      std::vector<DocumentPtr> doc = producerAccessor.getListDocumentsWithoutPrivateData("default", "discovery_monitoring");

      if(doc.size() != 1)
      {
        throw std::runtime_error("Not the good number of documents: expected 1, received "+std::to_string(doc.size()));
      }
      
      if(doc[0]->isContainingPrivateData())
      {
        throw std::runtime_error("Document is containing private data");
      }
      
      printf(" *<=  Test #3.2 > OK");
      testsResults.emplace_back (" Test #3.2 getListDocumentsWithoutPrivateData usage=discovery_monitoring",true);

    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #3.2 > Failed");
      printf("Error: %s\n\n",e.what());
      testsResults.emplace_back (" Test #3.2 getListDocumentsWithoutPrivateData usage=discovery_monitoring",false);
    }
  }

//test 4.1 => getDocumentWithoutPrivateData
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #4.1 getDocumentWithoutPrivateData\n");
    ProducerAccessor producerAccessor(SELFTEST_CLIENT_ID, 1000, endpoint);
    try
    {
      DocumentPtr doc = producerAccessor.getDocumentWithoutPrivateData("default", "id_readable");

      if(doc->isContainingPrivateData())
      {
        throw std::runtime_error("Document is containing private data");
      }
      
      printf(" *<=  Test #4.1 > OK\n");
      testsResults.emplace_back (" Test #4.1 getDocumentWithoutPrivateData",true);

    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #4.1 > Failed\n");
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #4.1 getDocumentWithoutPrivateData",false);
    }
  }

//test 4.2 => getDocumentWithoutPrivateData => SecwDocumentDoNotExistException
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #4.2 getDocumentWithoutPrivateData => SecwDocumentDoNotExistException\n");
    ProducerAccessor producerAccessor(SELFTEST_CLIENT_ID, 1000, endpoint);
    try
    {
      producerAccessor.getDocumentWithoutPrivateData("default", "XXXXX-XXXXXXXXX");

      throw std::runtime_error("Document is return");
    }
    catch(const SecwDocumentDoNotExistException &)
    {
      printf(" *<=  Test #4.2 > OK\n");
      testsResults.emplace_back (" Test #4.2 getDocumentWithoutPrivateData => SecwDocumentDoNotExistException",true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #4.2 > Failed\n");
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #4.2 getDocumentWithoutPrivateData => SecwDocumentDoNotExistException",false);
    }
  }

  return testsResults;
  
}

