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
#include <chrono>
#include "fty_security_wallet_classes.h"

#include <thread>


#include "fty_common_socket_sync_client.h"
#include "fty_common_mlm_stream_client.h"

namespace secw
{
  ProducerAccessor::ProducerAccessor( fty::SocketSyncClient & requestClient)
  {
      m_clientAccessor = std::make_shared<ClientAccessor>(requestClient);
  }
  
  ProducerAccessor::ProducerAccessor( fty::SocketSyncClient & requestClient, mlm::MlmStreamClient & subscriberClient)
  {
      m_clientAccessor = std::make_shared<ClientAccessor>(requestClient, subscriberClient);
  }

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

  std::set<UsageId> ProducerAccessor::getProducerUsages(const std::string & portfolioName) const
  {
    std::set<UsageId> usages;

    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_PRODUCER_USAGES,{portfolioName});

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
    const UsageId & usageId) const
  {
    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_LIST_WITHOUT_SECRET, {portfolio,usageId});
    
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

  std::vector<DocumentPtr> ProducerAccessor::getListDocumentsWithoutPrivateData(
    const std::string & portfolio,
    const std::vector<Id> & ids ) const
  {
    std::vector<DocumentPtr> docs;

    for(const Id & id : ids)
    {
      try
      {
        docs.push_back(getDocumentWithoutPrivateData(portfolio, id));
      }
      catch(const SecwException &e)
      {
        //filter exceptions => ID not found and no access to this id
        if( (e.getErrorCode() != DOCUMENT_DO_NOT_EXIST) && (e.getErrorCode() != ILLEGAL_ACCESS) )
        {
          throw; //throw
        }
      }
    }

    return docs;
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

  DocumentPtr ProducerAccessor::getDocumentWithoutPrivateDataByName(
    const std::string & portfolio,
    const std::string & name) const
  {
    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_WITHOUT_SECRET_BY_NAME, {portfolio,name});
    
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

  Id ProducerAccessor::insertNewDocument(
    const std::string & portfolio,
    const DocumentPtr & doc) const
  {
    cxxtools::SerializationInfo si;
    si <<= doc;
    std::string jsonDoc = serialize(si);

    //create
    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::CREATE, {portfolio,jsonDoc});

    return frames.at(0);
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

  void ProducerAccessor::setCallbackOnUpdate(UpdatedCallback updatedCallback)
  {
    m_clientAccessor->setCallbackOnUpdate(updatedCallback);
  }

  void ProducerAccessor::setCallbackOnCreate(CreatedCallback createdCallback)
  {
    m_clientAccessor->setCallbackOnCreate(createdCallback);
  }

  void ProducerAccessor::setCallbackOnDelete(DeletedCallback deletedCallback)
  {
    m_clientAccessor->setCallbackOnDelete(deletedCallback);
  }

  void ProducerAccessor::setCallbackOnStart(StartedCallback startedCallback)
  {
    m_clientAccessor->setCallbackOnStart(startedCallback);
  }

} //namespace secw

//  --------------------------------------------------------------------------
//  Test of this class => This is used by fty_security_wallet_server_test
//  --------------------------------------------------------------------------

#define TEST_TIMEOUT 5

using namespace std::placeholders;

//callback for test
secw::DocumentPtr g_newDoc;
secw::DocumentPtr g_oldDoc;
std::string g_portfolio;
std::string g_action;

void callbackCreate(const std::string& portfolio, secw::DocumentPtr newDoc, std::mutex * mut, std::condition_variable * condvar)
{
  log_debug ("callback CREATED");
  std::unique_lock<std::mutex> lock(*mut);
  g_action = "CREATED";
  g_portfolio = portfolio;
  g_newDoc = newDoc->clone();
  condvar->notify_all();
}

void callbackUpdated(const std::string& portfolio, secw::DocumentPtr oldDoc, secw::DocumentPtr newDoc, std::mutex * mut, std::condition_variable * condvar)
{
  log_debug ("callback UPDATED");
  std::unique_lock<std::mutex> lock(*mut);
  g_action = "UPDATED";
  g_portfolio = portfolio;
  g_newDoc = newDoc->clone();
  g_oldDoc = oldDoc->clone();
  condvar->notify_all();
}

void callbackDeleted(const std::string& portfolio, secw::DocumentPtr oldDoc, std::mutex * mut, std::condition_variable * condvar)
{ 
  log_debug ("callback DELETED");
  std::unique_lock<std::mutex> lock(*mut);
  g_action = "DELETED";
  g_portfolio = portfolio;
  g_oldDoc = oldDoc->clone();
  condvar->notify_all();
}

/*void callbackStarted() Cannot be tested here
{
  std::unique_lock<std::mutex> lock(mut);
  g_action = "STARTED";
  g_condvar.notify_all();
}*/


std::vector<std::pair<std::string,bool>> secw_producer_accessor_test(fty::SocketSyncClient & syncClient, mlm::MlmStreamClient & streamClient)
{
  std::vector<std::pair<std::string,bool>> testsResults;
  
  std::mutex g_lock;
  std::condition_variable g_condvar;
  
  using namespace secw;

  printf(" ** secw_producer_accessor_test: \n");

  std::string testNumber;
  std::string testName;


//test 1.1 => getPortfolioList
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #1.1 getPortfolioList\n");
    ProducerAccessor producerAccessor(syncClient, streamClient);
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
    std::string portfolioName("XXXXX");
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      producerAccessor.getDocumentWithoutPrivateData(portfolioName, "XXXXX-XXXXXXXXX");

      throw std::runtime_error("Document is returned");
    }
    catch(const SecwUnknownPortfolioException &e)
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

//test 2.1 => getProducerUsages
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #2.1 getProducerUsages\n");
    ProducerAccessor producerAccessor(syncClient, streamClient);
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
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      std::vector<DocumentPtr> doc = producerAccessor.getListDocumentsWithoutPrivateData("default");

      if(doc.size() != 4)
      {
        throw std::runtime_error("Not the good number of documents: expected 4, received "+std::to_string(doc.size()));
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
    ProducerAccessor producerAccessor(syncClient, streamClient);
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

//test 3.3 => getListDocumentsWithoutPrivateData with list of id
  testNumber = "3.3";
  testName = "getListDocumentsWithoutPrivateData with list of id";
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      std::vector<Id> ids = {"id_readable", "id_notReadable"};
      std::vector<DocumentPtr> docs = producerAccessor.getListDocumentsWithoutPrivateData("default", ids);

      if(docs.size() != 2)
      {
        throw std::runtime_error("Bad number of documents returned");
      }

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 4.1 => getDocumentWithoutPrivateData
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #4.1 getDocumentWithoutPrivateData\n");
    ProducerAccessor producerAccessor(syncClient, streamClient);
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
    ProducerAccessor producerAccessor(syncClient, streamClient);
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
  
 //test 4.3 => getDocumentWithoutPrivateData
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #4.3 getDocumentWithoutPrivateDataByName\n");
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr doc = producerAccessor.getDocumentWithoutPrivateDataByName("default", "myFirstDoc");

      if(doc->isContainingPrivateData())
      {
        throw std::runtime_error("Document is containing private data");
      }
      
      printf(" *<=  Test #4.3 > OK\n");
      testsResults.emplace_back (" Test #4.3 getDocumentWithoutPrivateDataByName",true);

    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #4.3 > Failed\n");
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #4.3 getDocumentWithoutPrivateDataByName",false);
    }
  }

//test 4.4 => getDocumentWithoutPrivateDataByName => SecwNameDoesNotExistException
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #4.4 getDocumentWithoutPrivateDataByName => SecwNameDoesNotExistException\n");
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      producerAccessor.getDocumentWithoutPrivateDataByName("default", "XXXXX-XXXXXXXXX");

      throw std::runtime_error("Document is return");
    }
    catch(const SecwNameDoesNotExistException &)
    {
      printf(" *<=  Test #4.4 > OK\n");
      testsResults.emplace_back (" Test #4.4 getDocumentWithoutPrivateDataByName => SecwNameDoesNotExistException",true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #4.4 > Failed\n");
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #4.4 getDocumentWithoutPrivateDataByName => SecwNameDoesNotExistException",false);
    }
  }

  Id id = "";

//test 5.1 => insertNewDocument SNMPV3
  testNumber = "5.1";
  testName = "insertNewDocument SNMPV3";
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);

    //register the callback on create
    producerAccessor.setCallbackOnCreate(std::bind(callbackCreate, _1, _2, &g_lock, &g_condvar));

    try
    {
      Snmpv3Ptr snmpv3Doc = std::make_shared<Snmpv3>("Test insert snmpv3",
                Snmpv3SecurityLevel::AUTH_PRIV,
                "test security name",
                Snmpv3AuthProtocol::MD5,
                "test auth password",
                Snmpv3PrivProtocol::AES,
                "test priv password");

      snmpv3Doc->addUsage("discovery_monitoring");

      //lock to wait for the callback => the callback will notify this thread if called properly
      std::unique_lock<std::mutex> lock(g_lock);
      g_action = "";
      g_portfolio = "";
      g_newDoc = nullptr;

      id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<Document>(snmpv3Doc));

      //wait for the callback to finish
      if (g_condvar.wait_for(lock, std::chrono::seconds(TEST_TIMEOUT)) == std::cv_status::timeout)
      {
          throw std::runtime_error ("Timed out when waiting for callback to finish");

      }

      //check that the doc is inserted and the callback called
      if(g_action != "CREATED") throw std::runtime_error("Wrong action in the callback");
      if(g_portfolio != "default") throw std::runtime_error("Wrong portfolio in the created callback");
      if(g_newDoc == nullptr) throw std::runtime_error("No new data in the created callback");
      if(g_newDoc->getId() != id) throw std::runtime_error("Wrong id in the new document in the created callback");
      if(g_newDoc->getName() != "Test insert snmpv3") throw std::runtime_error("Wrong name in the new document in the created callback");

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 5.2 => insertNewDocument -> retrieve data
  testNumber = "5.2";
  testName = "insertNewDocument SNMPV3 -> retrieve data";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      Snmpv3Ptr snmpv3 = Snmpv3::tryToCast(insertedDoc);

      if(snmpv3 == nullptr) throw std::runtime_error("No document retrieved");

      if(snmpv3->getUsageIds().count("discovery_monitoring") == 0) throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
      if(snmpv3->getUsageIds().size() != 1) throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

      if(snmpv3->getName() != "Test insert snmpv3") throw std::runtime_error("Bad document retrieved: name do not match");
      
      if(snmpv3->getSecurityName() != "test security name") throw std::runtime_error("Bad document retrieved: security name do not match");
      if(snmpv3->getSecurityLevel() != Snmpv3SecurityLevel::AUTH_PRIV) throw std::runtime_error("Bad document retrieved: security level do not match");
      if(snmpv3->getAuthProtocol() != Snmpv3AuthProtocol::MD5) throw std::runtime_error("Bad document retrieved: auth protocol do not match");
      if(snmpv3->getPrivProtocol() != Snmpv3PrivProtocol::AES) throw std::runtime_error("Bad document retrieved: priv protocol do not match");

      //doc is valid

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 5.3 => updateDocument SNMPV3
  testNumber = "5.3";
  testName = "updateDocument SNMPV3";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);

    //register the callback on update
    producerAccessor.setCallbackOnUpdate(std::bind(callbackUpdated, _1, _2, _3, &g_lock, &g_condvar));


    try
    {
      DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      Snmpv3Ptr snmpv3 = Snmpv3::tryToCast(insertedDoc);

      if(snmpv3 == nullptr) throw std::runtime_error("No document retrieved");

      //update security name and priv password
      snmpv3->setName("Test update snmpv3");
      snmpv3->setSecurityName("test update security snmpv3");
      snmpv3->setPrivPassword("new password");

      //lock to wait for the callback => the callback will notify this thread if called properly
      std::unique_lock<std::mutex> lock(g_lock);
      g_action = "";
      g_portfolio = "";
      g_oldDoc = nullptr;
      g_newDoc = nullptr;

      //update
      producerAccessor.updateDocument("default", std::dynamic_pointer_cast<Document>(snmpv3));

      //wait for the callback to finish
      if (g_condvar.wait_for(lock, std::chrono::seconds(TEST_TIMEOUT)) == std::cv_status::timeout)
      {
          throw std::runtime_error ("Timed out when waiting for callback to finish");
      }

      //check that the doc is inserted and the callback called
      if(g_action != "UPDATED") throw std::runtime_error("Wrong action in the callback");
      if(g_portfolio != "default") throw std::runtime_error("Wrong portfolio in the updated callback");
      if(g_oldDoc == nullptr) throw std::runtime_error("No old data in the updated callback");
      if(g_oldDoc->getId() != id) throw std::runtime_error("Wrong id in the old document in the updated callback");
      if(g_oldDoc->getName() != "Test insert snmpv3") throw std::runtime_error("Wrong name in the old document in the updated callback");
      if(g_newDoc == nullptr) throw std::runtime_error("No new data in the updated callback");
      if(g_newDoc->getId() != id) throw std::runtime_error("Wrong id in the new document in the updated callback");
      if(g_newDoc->getName() != "Test update snmpv3") throw std::runtime_error("Wrong name in the new document in the updated callback");

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 5.4 => updateDocument SNMPV3 -> retrieve data
  testNumber = "5.4";
  testName = "updateDocument SNMPV3 -> retrieve data";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      Snmpv3Ptr snmpv3 = Snmpv3::tryToCast(insertedDoc);

      if(snmpv3 == nullptr) throw std::runtime_error("No document retrieved");

      if(snmpv3->getUsageIds().count("discovery_monitoring") == 0) throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
      if(snmpv3->getUsageIds().size() != 1) throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

      if(snmpv3->getName() != "Test update snmpv3") throw std::runtime_error("Bad document retrieved: name do not match: "+snmpv3->getName());
      
      if(snmpv3->getSecurityName() != "test update security snmpv3") throw std::runtime_error("Bad document retrieved: security name do not match");
      if(snmpv3->getSecurityLevel() != Snmpv3SecurityLevel::AUTH_PRIV) throw std::runtime_error("Bad document retrieved: security level do not match");
      if(snmpv3->getAuthProtocol() != Snmpv3AuthProtocol::MD5) throw std::runtime_error("Bad document retrieved: auth protocol do not match");
      if(snmpv3->getPrivProtocol() != Snmpv3PrivProtocol::AES) throw std::runtime_error("Bad document retrieved: priv protocol do not match");

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }


//test 5.5 => updateDocument SNMPV3 -> bad format
  testNumber = "5.5";
  testName = "updateDocument SNMPV3 -> bad format";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);

    //register the callback on update
    producerAccessor.setCallbackOnUpdate(std::bind(callbackUpdated, _1, _2, _3, &g_lock, &g_condvar));


    try
    {
     DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      Snmpv3Ptr snmpv3 = Snmpv3::tryToCast(insertedDoc);

      if(snmpv3 == nullptr) throw std::runtime_error("No document retrieved");

      //update with wrong data
      snmpv3->setSecurityName("");

      g_action = "";
      g_portfolio = "";
      g_newDoc = nullptr;

      //update
      producerAccessor.updateDocument("default", std::dynamic_pointer_cast<Document>(snmpv3));

      if(g_action != "") throw std::runtime_error("Non-empty action");
      if(g_portfolio != "") throw std::runtime_error("Non-empty portfolio");
      if(g_newDoc != nullptr) throw std::runtime_error("Non-empty data");

      throw std::runtime_error("Document has been updated");
    }
    catch(const SecwInvalidDocumentFormatException &)
    {
      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 5.6 => deleteDocument SNMPV3
  testNumber = "5.6";
  testName = "deleteDocument SNMPV3";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);

    //register the callback on create
    producerAccessor.setCallbackOnDelete(std::bind(callbackDeleted, _1, _2, &g_lock, &g_condvar));

    try
    {
      //lock to wait for the callback => the callback will notify this thread if called properly
      std::unique_lock<std::mutex> lock(g_lock);
      g_action = "";
      g_portfolio = "";
      g_oldDoc = nullptr;

      producerAccessor.deleteDocument("default", id);

      //wait for the callback to finish
      if (g_condvar.wait_for(lock, std::chrono::seconds(TEST_TIMEOUT)) == std::cv_status::timeout)
      {
          throw std::runtime_error ("Timed out when waiting for callback to finish");
      }

      //check that the doc is inserted and the callback called
      if(g_action != "DELETED") throw std::runtime_error("Wrong action in the callback");
      if(g_portfolio != "default") throw std::runtime_error("Wrong portfolio in the deleted callback");
      if(g_oldDoc == nullptr) throw std::runtime_error("No old data in the deleted callback");
      if(g_oldDoc->getId() != id) throw std::runtime_error("Wrong id in the old document in the deleted callback");
      if(g_oldDoc->getName() != "Test update snmpv3") throw std::runtime_error("Wrong name in the old document in the deleted callback");

      //check that the document is removed
      std::vector<Id> ids = {id};
      if(producerAccessor.getListDocumentsWithoutPrivateData("default",ids).size() != 0)
      {
        throw std::runtime_error("Document is not removed");
      }

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 6.1 => insertNewDocument User and Password
  testNumber = "6.1";
  testName = "insertNewDocument User and Password";
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      UserAndPasswordPtr doc = std::make_shared<UserAndPassword>("Test insert username","username", "password");

      doc->addUsage("discovery_monitoring");

      id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<Document>(doc));

      //check that the doc is inserted

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 6.2 => insertNewDocument -> retrieve data
  testNumber = "6.2";
  testName = "insertNewDocument User and Password -> retrieve data";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      UserAndPasswordPtr doc = UserAndPassword::tryToCast(insertedDoc);

      if(doc == nullptr) throw std::runtime_error("No document retrieved");

      if(doc->getUsageIds().count("discovery_monitoring") == 0) throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
      if(doc->getUsageIds().size() != 1) throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

      if(doc->getName() != "Test insert username") throw std::runtime_error("Bad document retrieved: name do not match");
      
      if(doc->getUsername() != "username") throw std::runtime_error("Bad document retrieved: username do not match");

      //doc is valid

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 6.3 => insertNewDocument User and Password
  testNumber = "6.3";
  testName = "insertNewDocument User and Password -> name already exist";
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      UserAndPasswordPtr doc = std::make_shared<UserAndPassword>("Test insert username","username", "password");

      doc->addUsage("discovery_monitoring");

      id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<Document>(doc));

      throw std::runtime_error("Document has been added");
    }
    catch(const SecwNameAlreadyExistsException & e)
    {
      if(e.getName() == "Test insert username")
      {
        printf(" *<=  Test #%s > OK\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
      }
      else
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: expected name in exception 'Test insert username' received '%s'\n",e.getName().c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 6.4 => updateDocument User and Password
  testNumber = "6.4";
  testName = "updateDocument User and Password";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      UserAndPasswordPtr doc = UserAndPassword::tryToCast(insertedDoc);

      if(doc == nullptr) throw std::runtime_error("No document retrieved");

      //update security name and priv password
      doc->setName("Test update username");
      doc->setUsername("new_username");
      doc->setPassword("new_password");

      //update
      producerAccessor.updateDocument("default", std::dynamic_pointer_cast<Document>(doc));

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 6.6 => updateDocument User and Password -> retrieve data
  testNumber = "6.6";
  testName = "updateDocument User and Password -> retrieve data";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      UserAndPasswordPtr doc = UserAndPassword::tryToCast(insertedDoc);

      if(doc == nullptr) throw std::runtime_error("No document retrieved");

      if(doc->getUsageIds().count("discovery_monitoring") == 0) throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
      if(doc->getUsageIds().size() != 1) throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

      
      if(doc->getName() != "Test update username") throw std::runtime_error("Bad document retrieved: name do not match");
      if(doc->getUsername() != "new_username") throw std::runtime_error("Bad document retrieved: username do not match");

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 6.7 => updateDocument User and Password -> retrieve data getDocumentWithoutPrivateDataByName
  testNumber = "6.7";
  testName = "updateDocument User and Password -> retrieve data getDocumentWithoutPrivateDataByName";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateDataByName("default", "Test update username");

      UserAndPasswordPtr doc = UserAndPassword::tryToCast(insertedDoc);

      if(doc == nullptr) throw std::runtime_error("No document retrieved");

      if(doc->getUsageIds().count("discovery_monitoring") == 0) throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
      if(doc->getUsageIds().size() != 1) throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

      
      if(doc->getId() != id ) throw std::runtime_error("Bad document retrieved: id do not match");
      if(doc->getUsername() != "new_username") throw std::runtime_error("Bad document retrieved: username do not match");

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 6.8 => updateDocument User and Password -> getDocumentWithoutPrivateDataByName => SecwNameDoesNotExistException
  testNumber = "6.8";
  testName = "updateDocument User and Password -> getDocumentWithoutPrivateDataByName => SecwNameDoesNotExistException";
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      producerAccessor.getDocumentWithoutPrivateDataByName("default", "Test insert username");

      throw std::runtime_error("Document is return");
    }
    catch(const SecwNameDoesNotExistException &)
    {
      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 6.9 => updateDocument User and Password
  testNumber = "6.9";
  testName = "updateDocument User and Password -> name already exist";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      UserAndPasswordPtr doc = UserAndPassword::tryToCast(insertedDoc);

      if(doc == nullptr) throw std::runtime_error("No document retrieved");

      //update name
      doc->setName("myFirstDoc");

      //update
      producerAccessor.updateDocument("default", std::dynamic_pointer_cast<Document>(doc));

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      throw std::runtime_error("Document has been updated");
    }
    catch(const SecwNameAlreadyExistsException & e)
    {
      if(e.getName() == "myFirstDoc")
      {
        printf(" *<=  Test #%s > OK\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
      }
      else
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: expected name in exception 'myFirstDoc' received '%s'\n",e.getName().c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 6.10 => updateDocument User and Password -> bad format
  testNumber = "6.10";
  testName = "updateDocument User and Password -> bad format";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
     DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      UserAndPasswordPtr doc = UserAndPassword::tryToCast(insertedDoc);

      if(doc == nullptr) throw std::runtime_error("No document retrieved");

      //update with wrong data
      doc->setUsername("");

      //update
      producerAccessor.updateDocument("default", std::dynamic_pointer_cast<Document>(doc));

      throw std::runtime_error("Document has been updated");
    }
    catch(const SecwInvalidDocumentFormatException &)
    {
      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 6.11 => deleteDocument User and Password
  testNumber = "6.11";
  testName = "deleteDocument User and Password";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      producerAccessor.deleteDocument("default", id);

      //check that the document is removed
      std::vector<Id> ids = {id};
      if(producerAccessor.getListDocumentsWithoutPrivateData("default", ids).size() != 0)
      {
        throw std::runtime_error("Document is not removed");
      }

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }


//test 7.1 => insertNewDocument Snmpv1
  testNumber = "7.1";
  testName = "insertNewDocument Snmpv1";
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      Snmpv1Ptr doc = std::make_shared<Snmpv1>("Test insert snmpv1","community");

      doc->addUsage("discovery_monitoring");

      id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<Document>(doc));

      //check that the doc is inserted

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 7.2 => insertNewDocument Snmpv1 -> retrieve data
  testNumber = "7.2";
  testName = "insertNewDocument Snmpv1 -> retrieve data";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      Snmpv1Ptr doc = Snmpv1::tryToCast(insertedDoc);

      if(doc == nullptr) throw std::runtime_error("No document retrieved");

      if(doc->getUsageIds().count("discovery_monitoring") == 0) throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
      if(doc->getUsageIds().size() != 1) throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

      if(doc->getName() != "Test insert snmpv1") throw std::runtime_error("Bad document retrieved: name do not match");
      
      if(doc->getCommunityName() != "community") throw std::runtime_error("Bad document retrieved: community do not match");

      //doc is valid

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 7.3 => updateDocument Snmpv1
  testNumber = "7.3";
  testName = "updateDocument Snmpv1";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      Snmpv1Ptr doc = Snmpv1::tryToCast(insertedDoc);

      if(doc == nullptr) throw std::runtime_error("No document retrieved");

      //update
      doc->setName("Test update snmpv1");
      doc->setCommunityName("new_community");

      //update
      producerAccessor.updateDocument("default", std::dynamic_pointer_cast<Document>(doc));

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 7.4 => updateDocument Snmpv1 -> retrieve data
  testNumber = "7.4";
  testName = "updateDocument Snmpv1 -> retrieve data";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      Snmpv1Ptr doc = Snmpv1::tryToCast(insertedDoc);

      if(doc == nullptr) throw std::runtime_error("No document retrieved");

      if(doc->getUsageIds().count("discovery_monitoring") == 0) throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
      if(doc->getUsageIds().size() != 1) throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

      
      if(doc->getName() != "Test update snmpv1") throw std::runtime_error("Bad document retrieved: name do not match");
      if(doc->getCommunityName() != "new_community") throw std::runtime_error("Bad document retrieved: community do not match");

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }


//test 7.5 => updateDocument Snmpv1 -> bad format
  testNumber = "7.5";
  testName = "updateDocument Snmpv1 -> bad format";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
     DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

      Snmpv1Ptr doc = Snmpv1::tryToCast(insertedDoc);

      if(doc == nullptr) throw std::runtime_error("No document retrieved");

      //update with wrong data
      doc->setCommunityName("");

      //update
      producerAccessor.updateDocument("default", std::dynamic_pointer_cast<Document>(doc));

      throw std::runtime_error("Document has been updated");
    }
    catch(const SecwInvalidDocumentFormatException &)
    {
      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 7.6 => deleteDocument Snmpv1
  testNumber = "7.6";
  testName = "deleteDocument Snmpv1";
  printf("\n-----------------------------------------------------------------------\n");
  {
     printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    ProducerAccessor producerAccessor(syncClient, streamClient);
    try
    {
      producerAccessor.deleteDocument("default", id);

      //check that the document is removed
      std::vector<Id> ids = {id};
      if(producerAccessor.getListDocumentsWithoutPrivateData("default", ids).size() != 0)
      {
        throw std::runtime_error("Document is not removed");
      }

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

std::string document;
//test 8.1 => test serialization in string of doc
  testNumber = "8.1";
  testName = "serialization in string of doc";
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    try
    {
      Snmpv3Ptr snmpv3Doc = std::make_shared<Snmpv3>("Test insert snmpv3",
                Snmpv3SecurityLevel::AUTH_PRIV,
                "test security name",
                Snmpv3AuthProtocol::MD5,
                "test auth password",
                Snmpv3PrivProtocol::AES,
                "test priv password");
      
      snmpv3Doc->validate();

      document <<= snmpv3Doc;

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

//test 8.2 => test deserialization of doc from string
  testNumber = "8.2";
  testName = "deserialization of doc from string";
  printf("\n-----------------------------------------------------------------------\n");
  {
    printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
    try
    {
      DocumentPtr doc;

      document >>= doc;
      
      doc->validate();

      printf(" *<=  Test #%s > OK\n", testNumber.c_str());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
    }
    catch(const std::exception& e)
    {
      printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
      printf("Error: %s\n",e.what());
      testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
    }
  }

  return testsResults;
  
}

