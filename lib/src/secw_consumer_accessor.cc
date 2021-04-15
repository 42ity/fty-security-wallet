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

/*
@header
  secw_consumer_accessor - Accessor to return documents from the agent
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

#include "secw_client_accessor.h"
#include "secw_helpers.h"

#include <fty_common_socket_sync_client.h>
#include <fty_common_mlm_stream_client.h>

namespace secw
{
  ConsumerAccessor::ConsumerAccessor( fty::SocketSyncClient & requestClient)
  {
      m_clientAccessor = std::make_shared<ClientAccessor>(requestClient);
  }

  ConsumerAccessor::ConsumerAccessor( fty::SocketSyncClient & requestClient, mlm::MlmStreamClient & subscriberClient)
  {
      m_clientAccessor = std::make_shared<ClientAccessor>(requestClient, subscriberClient);
  }

  std::vector<std::string> ConsumerAccessor::getPortfolioList() const
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

  std::set<UsageId> ConsumerAccessor::getConsumerUsages(const std::string & portfolioName) const
  {
    std::set<UsageId> usages;

    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_CONSUMER_USAGES,{portfolioName});

    if(frames.size() < 1)
    {
      throw SecwProtocolErrorException("Empty answer from server");
    }

    cxxtools::SerializationInfo si = deserialize(frames.at(0));

    si >>= usages;

    return usages;
  }

  std::vector<DocumentPtr> ConsumerAccessor::getListDocumentsWithPrivateData(
    const std::string & portfolio,
    const UsageId & usageId) const
  {
    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_LIST_WITH_SECRET, {portfolio,usageId});

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

  std::vector<DocumentPtr> ConsumerAccessor::getListDocumentsWithPrivateData(
      const std::string & portfolio,
      const std::vector<Id> & ids) const
  {
    std::vector<DocumentPtr> docs;

    for(const Id & id : ids)
    {
      try
      {
        docs.push_back(getDocumentWithPrivateData(portfolio, id));
      }
      catch(const SecwException &e)
      {
        //filter exceptions => ID not found and noit access to this id
        if( (e.getErrorCode() != DOCUMENT_DO_NOT_EXIST) && (e.getErrorCode() != ILLEGAL_ACCESS) )
        {
          throw; //throw
        }
      }
    }

    return docs;
  }

  DocumentPtr ConsumerAccessor::getDocumentWithPrivateData(
    const std::string & portfolio,
    const Id & id) const
  {
    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_WITH_SECRET, {portfolio,id});

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

  DocumentPtr ConsumerAccessor::getDocumentWithPrivateDataByName(
    const std::string & portfolio,
    const std::string & name) const
  {
    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_WITH_SECRET_BY_NAME, {portfolio,name});

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

  void ConsumerAccessor::setCallbackOnUpdate(UpdatedCallback updatedCallback)
  {
    m_clientAccessor->setCallbackOnUpdate(updatedCallback);
  }

  void ConsumerAccessor::setCallbackOnCreate(CreatedCallback createdCallback)
  {
    m_clientAccessor->setCallbackOnCreate(createdCallback);
  }

  void ConsumerAccessor::setCallbackOnDelete(DeletedCallback deletedCallback)
  {
    m_clientAccessor->setCallbackOnDelete(deletedCallback);
  }

  void ConsumerAccessor::setCallbackOnStart(StartedCallback startedCallback)
  {
    m_clientAccessor->setCallbackOnStart(startedCallback);
  }

} //namespace secw

//  --------------------------------------------------------------------------
//  Test of this class => This is used by fty_security_wallet_server_test
//  --------------------------------------------------------------------------



std::vector<std::pair<std::string,bool>> secw_consumer_accessor_test(fty::SocketSyncClient & syncClient, mlm::MlmStreamClient & streamClient)
{
  std::vector<std::pair<std::string,bool>> testsResults;

  using namespace secw;

  printf(" ** secw_consumer_accessor_test: \n");

//test 1.1 => getPortfolioList
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #1.1 getPortfolioList\n");
    ConsumerAccessor consumerAccessor(syncClient, streamClient);
    try
    {
      std::vector<std::string> portfolio = consumerAccessor.getPortfolioList();

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
    std::string portfolioName("XXXXX");

    ConsumerAccessor consumerAccessor(syncClient, streamClient);

    try
    {
      consumerAccessor.getDocumentWithPrivateData(portfolioName, "XXXXX-XXXXXXXXX");

      throw std::runtime_error("Document is returned");
    }
    catch(const SecwUnknownPortfolioException & e)
    {
      if(e.getPortfolioName() == portfolioName )
      {
        printf(" *<=  Test #1.2 > OK\n");
        testsResults.emplace_back (" Test #1.2 SecwUnknownPortfolioException",true);
      }
      else
      {
        printf(" *<=  Test #1.2 > Failed\n");
        printf("Error: missmatch of portfolio name %s, %s\n\n",e.getPortfolioName().c_str(),portfolioName.c_str());
        testsResults.emplace_back (" Test #1.2 SecwUnknownPortfolioException",false);
      }

    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #1.2 > Failed\n");
      printf("Error: %s\n\n",e.what());
      testsResults.emplace_back (" Test #1.2 SecwUnknownPortfolioException",false);
    }
  }

//test 2.1 => getConsumerUsages
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #2.1 getConsumerUsages\n");
    ConsumerAccessor consumerAccessor(syncClient, streamClient);
    try
    {
      std::set<std::string> usages = consumerAccessor.getConsumerUsages();

      if(usages.size() != 1)
      {
        throw std::runtime_error("Wrong number of customer use returned");
      }

      if(usages.count("discovery_monitoring") == 0)
      {
        throw std::runtime_error("Usage 'discovery_monitoring' is not in the list of consumer usages");
      }

      printf(" *<=  Test #2.1 > OK\n");

      testsResults.emplace_back (" Test #2.1 getConsumerUsages",true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #2.1 > Failed\n");
      printf("Error: %s\n\n",e.what());

      testsResults.emplace_back (" Test #2.1 getConsumerUsages",false);
    }

  }

//test 3.1 => getListDocumentsWithPrivateData
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #3.1 getListDocumentsWithPrivateData\n");
    ConsumerAccessor consumerAccessor(syncClient, streamClient);
    try
    {
      std::vector<DocumentPtr> doc = consumerAccessor.getListDocumentsWithPrivateData("default");

      if(doc.size() != 1)
      {
        throw std::runtime_error("Not the good number of documents: expected 1, received "+std::to_string(doc.size()));
      }

      if(!doc[0]->isContainingPrivateData())
      {
        throw std::runtime_error("Document is not containing private data");
      }

      printf(" *<=  Test #3.1 > OK\n");
      testsResults.emplace_back (" Test #3.1 getListDocumentsWithPrivateData",true);

    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #3.1 > Failed\n");
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #3.1 getListDocumentsWithPrivateData",false);
    }
  }

//test 3.2 => getListDocumentsWithPrivateData usage="discovery_monitoring"
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #3.2 getListDocumentsWithPrivateData usage=discovery_monitoring");
    ConsumerAccessor consumerAccessor(syncClient, streamClient);
    try
    {
      std::vector<DocumentPtr> doc = consumerAccessor.getListDocumentsWithPrivateData("default", "discovery_monitoring");

      if(doc.size() != 1)
      {
        throw std::runtime_error("Not the good number of documents: expected 1, received "+std::to_string(doc.size()));
      }

      if(!doc[0]->isContainingPrivateData())
      {
        throw std::runtime_error("Document is not containing private data");
      }

      printf(" *<=  Test #3.2 > OK");
      testsResults.emplace_back (" Test #3.2 getListDocumentsWithPrivateData usage=discovery_monitoring",true);

    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #3.2 > Failed");
      printf("Error: %s\n\n",e.what());
      testsResults.emplace_back (" Test #3.2 getListDocumentsWithPrivateData usage=discovery_monitoring",false);
    }
  }

//test 3.3  => getListDocumentsWithPrivateData SecwIllegalAccess
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #3.3 getListDocumentsWithPrivateData => SecwIllegalAccess");
    ConsumerAccessor consumerAccessor(syncClient, streamClient);
    try
    {
      consumerAccessor.getListDocumentsWithPrivateData("default", "mass_device_management");

      throw std::runtime_error("Documents are returned");
    }
    catch(const SecwIllegalAccess &)
    {
      printf(" *<=  Test #3.3 > OK");
      testsResults.emplace_back (" Test #3.3 getListDocumentsWithPrivateData => SecwIllegalAccess",true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #3.3 > Failed");
      printf("Error: %s\n\n",e.what());
      testsResults.emplace_back (" Test #3.3 getListDocumentsWithPrivateData => SecwIllegalAccess",false);
    }
  }

//test 4.1 => getDocumentWithPrivateData
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #4.1 getDocumentWithPrivateData\n");
    ConsumerAccessor consumerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr doc = consumerAccessor.getDocumentWithPrivateData("default", "id_readable");

      if(!doc->isContainingPrivateData())
      {
        throw std::runtime_error("Document is not containing private data");
      }

      printf(" *<=  Test #4.1 > OK\n");
      testsResults.emplace_back (" Test #4.1 getDocumentWithPrivateData",true);

    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #4.1 > Failed\n");
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #4.1 getDocumentWithPrivateData",false);
    }
  }

//test 4.2 => getDocumentWithPrivateData => SecwDocumentDoNotExistException
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #4.2 getDocumentWithPrivateData => SecwDocumentDoNotExistException\n");
    ConsumerAccessor consumerAccessor(syncClient, streamClient);
    try
    {
      consumerAccessor.getDocumentWithPrivateData("default", "XXXXX-XXXXXXXXX");

      throw std::runtime_error("Document is return");
    }
    catch(const SecwDocumentDoNotExistException &)
    {
      printf(" *<=  Test #4.2 > OK\n");
      testsResults.emplace_back (" Test #4.2 getDocumentWithPrivateData => SecwDocumentDoNotExistException",true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #4.2 > Failed\n");
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #4.2 getDocumentWithPrivateData => SecwDocumentDoNotExistException",false);
    }
  }

//test 4.3 => getDocumentWithPrivateData
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #4.3 getDocumentWithPrivateDataByName\n");
    ConsumerAccessor consumerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr doc = consumerAccessor.getDocumentWithPrivateDataByName("default", "myFirstDoc");

      if(!doc->isContainingPrivateData())
      {
        throw std::runtime_error("Document is not containing private data");
      }

      printf(" *<=  Test #4.3 > OK\n");
      testsResults.emplace_back (" Test #4.3 getDocumentWithPrivateDataByName",true);

    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #4.3 > Failed\n");
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #4.3 getDocumentWithPrivateDataByName",false);
    }
  }

//test 4.4 => getDocumentWithPrivateDataByName => SecwNameDoesNotExistException
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #4.4 getDocumentWithPrivateDataByName => SecwNameDoesNotExistException\n");
    ConsumerAccessor consumerAccessor(syncClient, streamClient);
    try
    {
      consumerAccessor.getDocumentWithPrivateDataByName("default", "XXXXX-XXXXXXXXX");

      throw std::runtime_error("Document is return");
    }
    catch(const SecwNameDoesNotExistException &)
    {
      printf(" *<=  Test #4.4 > OK\n");
      testsResults.emplace_back (" Test #4.4 getDocumentWithPrivateDataByName => SecwNameDoesNotExistException",true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #4.4 > Failed\n");
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #4.4 getDocumentWithPrivateDataByName => SecwNameDoesNotExistException",false);
    }
  }

  return testsResults;

}
