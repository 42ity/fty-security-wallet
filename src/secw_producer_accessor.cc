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
    /*ClientAccessor::ClientAccessor(const ClientId & clientId):
        m_clientId(clientId),
        m_client(mlm_client_new())
    {
        mlm_client_connect(m_client, END_POINT, 1000, m_clientId.c_str());
    }

    ClientAccessor:: ~ClientAccessor()
    {
        mlm_client_destroy(&m_client);
    }

    std::vector<std::string> ClientAccessor::getPortfolioList() const
    {
        std::vector<std::string> frames = sendCommand(SecurityWalletServer::GET_PORTFOLIO_LIST,{});

        if(frames.size() < 1)
        {
            throw SecwProtocolErrorException("Empty answer from server");
        }

        cxxtools::SerializationInfo si = deserialize(frames.at(0));

        std::vector<std::string> portfolioNames;

        si >>= portfolioNames;

        return portfolioNames;
    }

    std::vector<Tag> ClientAccessor::getPrivateReadableTagList() const
    {
        std::vector<std::string> frames = sendCommand(SecurityWalletServer::GET_READABLE_TAGS,{});

        if(frames.size() < 1)
        {
            throw SecwProtocolErrorException("Empty answer from server");
        }

        cxxtools::SerializationInfo si = deserialize(frames.at(0));

        std::vector<std::string> tags;

        si >>= tags;

        return tags;
    }
    
    std::vector<Tag> ClientAccessor::getEditableTagList() const
    {
        std::vector<std::string> frames = sendCommand(SecurityWalletServer::GET_EDITABLE_TAGS,{});

        if(frames.size() < 1)
        {
            throw SecwProtocolErrorException("Empty answer from server");
        }

        cxxtools::SerializationInfo si = deserialize(frames.at(0));

        std::vector<std::string> tags;

        si >>= tags;

        return tags;
    }

    std::vector<DocumentPtr> ClientAccessor::getListDocumentsWithPrivateData(
        const std::string & portfolio,
        const Tag & tag,
        const DocumentType & type) const
    {
        std::vector<std::string> frames = sendCommand(SecurityWalletServer::GET_LIST_WITH_SECRET, {portfolio,tag,type});
        
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

    std::vector<DocumentPtr> ClientAccessor::getDocumentsWithoutPrivateData(
        const std::string & portfolio,
        const Tag & tag,
        const DocumentType & type) const
    {
        std::vector<std::string> frames = sendCommand(SecurityWalletServer::GET_LIST_WITHOUT_SECRET, {portfolio,tag,type});
        
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

    DocumentPtr ClientAccessor::getDocumentWithPrivateData(
        const std::string & portfolio,
        const Id & id) const
    {
        std::vector<std::string> frames = sendCommand(SecurityWalletServer::GET_WITH_SECRET, {portfolio,id});
        
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
    
    DocumentPtr ClientAccessor::getDocumentWithoutPrivateData(
        const std::string & portfolio,
        const Id & id) const
    {
        std::vector<std::string> frames = sendCommand(SecurityWalletServer::GET_WITHOUT_SECRET, {portfolio,id});
        
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

    void ClientAccessor::setDocument(
        const std::string & portfolio,
        const DocumentPtr & doc) const
    {
        cxxtools::SerializationInfo si;
        si <<= doc;
        std::string jsonDoc = serialize(si);
        
        if(doc->getId().empty())
        {
            //create
            sendCommand(SecurityWalletServer::CREATE, {portfolio,jsonDoc});
        }
        else
        {
            //update
            sendCommand(SecurityWalletServer::UPDATE, {portfolio,jsonDoc});
        }
    }

    void ClientAccessor::deleteDocument(
        const std::string & portfolio,
        const DocumentPtr & doc) const
    {
        sendCommand(SecurityWalletServer::DELETE, {portfolio,doc->getId()});
    }

//Private
    std::vector<std::string> ClientAccessor::sendCommand(const std::string & command, const std::vector<std::string> & frames) const
    {
        //Prepare the request:
        zmsg_t *request = zmsg_new();
        ZuuidGuard  zuuid(zuuid_new ());
        zmsg_addstr (request, zuuid_str_canonical (zuuid));

        //add the command
        zmsg_addstr (request, command.c_str());

        //add all the extra frames
        for(const std::string & frame : frames )
        {
            zmsg_addstr (request, frame.c_str());
        }

        //send the message
        mlm_client_sendto (m_client, SECURITY_WALLET_AGENT, "REQUEST", NULL, 1000, &request);

        //Get the reply
        ZmsgGuard recv(mlm_client_recv (m_client));

        //Get number of frame all the frame
        size_t numberOfFrame = zmsg_size(recv);

        if(numberOfFrame < 2)
        {
            throw SecwProtocolErrorException("Wrong number of frame");
        }

        //Check the message
        ZstrGuard str(zmsg_popstr (recv));
        if(!streq (str, zuuid_str_canonical (zuuid)))
        {
            throw SecwProtocolErrorException("Mismatch correlation id");
        };

        std::vector<std::string> receivedFrames;
    
        //we unstack all the other frame starting by the 2rd one.
        for(size_t index = 1; index < numberOfFrame; index++)
        {
            ZstrGuard frame( zmsg_popstr(recv) );
            receivedFrames.push_back( std::string(frame.get()) );
        }

        //check if the first frame we get is an error
        if(receivedFrames[0] == "ERROR")
        {
            //It's an error and we will throw directly the exceptions
            if(receivedFrames.size() == 2)
            {
                SecwException::throwSecwException(receivedFrames.at(1));
            }
            else
            {
                throw SecwProtocolErrorException("Missing data for error");
            }
        }

        return receivedFrames;
    }

    cxxtools::SerializationInfo ClientAccessor::deserialize(const std::string & json)
    {
        cxxtools::SerializationInfo si;

        try
        {
            std::stringstream input;
            input << json;
            cxxtools::JsonDeserializer deserializer(input);
            deserializer.deserialize(si);
        }
        catch(const std::exception& e)
        {
            throw SecwProtocolErrorException("Error in the json from server: "+std::string(e.what()));
        }

        return si;

    }
    
    std::string ClientAccessor::serialize(const cxxtools::SerializationInfo & si)
    {
        std::string returnData("");

        try
        {
            std::stringstream output;
            cxxtools::JsonSerializer serializer(output);
            serializer.serialize(si);
            
            returnData = output.str();
        }
        catch(const std::exception& e)
        {
            throw SecwException("Error while creating json "+std::string(e.what()));
        }

        return returnData;
    }
        
    
} //namespace secw

//  --------------------------------------------------------------------------
//  Test of this class => This is used by fty_security_wallet_server_test

#define SELFTEST_CLIENT_ID "secw-client-test"

std::vector<std::pair<std::string,bool>> secw_client_accessor_test ()
{
    std::vector<std::pair<std::string,bool>> testsResults;
    
    using namespace secw;

    log_debug (" ** secw_client_accessor_tests: ");

//test 1 => getPortfolioList
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #1 getPortfolioList");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            std::vector<std::string> portfolio = clientAccessor.getPortfolioList();

            if(portfolio.at(0) != "default")
            {
                throw std::runtime_error("Portfolio default is not in the list of portfolio");
            }
            
            log_debug(" *<= Test #1 > OK");
            
            testsResults.emplace_back ("Test #1 getPortfolioList",true);
        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #1 > Failed");
            log_debug("Error: %s\n\n",e.what());
            
            testsResults.emplace_back ("Test #1 getPortfolioList",false);
        }

    }
    
//test 2 => getPrivateReadableTagList
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #2 getPrivateReadableTagList ");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            std::vector<std::string> tags = clientAccessor.getPrivateReadableTagList();

            if(tags.at(0) != "test_read")
            {
                throw std::runtime_error("Tag <test_read> is not in the list of tags");
            }
            
            log_debug(" *<= Test #2 > OK");
            testsResults.emplace_back ("Test #2 getPrivateReadableTagList",true);

        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #2 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #2 getPrivateReadableTagList",false);
        }
    }
    
//test 3 => getEditableTagList
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #3 getEditableTagList");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            std::vector<std::string> tags = clientAccessor.getEditableTagList();

            if(tags.at(0) != "test_update")
            {
                throw std::runtime_error("Tag <test_update> is not in the list of tags");
            }
            
            log_debug(" *<= Test #3 > OK");
            testsResults.emplace_back ("Test #3 getEditableTagList",true);

        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #3 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #3 getEditableTagList",false);
        }
    }
    
//test 4.1 => getListDocumentsWithPrivateData
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #4.1 getListDocumentsWithPrivateData");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            std::vector<DocumentPtr> doc = clientAccessor.getListDocumentsWithPrivateData("default");

            if(doc.size() != 1)
            {
                throw std::runtime_error("Not the good number of documents: expected 1, received "+std::to_string(doc.size()));
            }
            
            if(!doc[0]->isContainingPrivateData())
            {
                throw std::runtime_error("Document is not containing private data");
            }
            
            log_debug(" *<= Test #4.1 > OK");
            testsResults.emplace_back ("Test #4.1 getListDocumentsWithPrivateData",true);

        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #4.1 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #4.1 getListDocumentsWithPrivateData",false);
        }
    }

//test 4.2 => getListDocumentsWithPrivateData tag=test_read
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #4.2 getListDocumentsWithPrivateData tag=test_read");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            std::vector<DocumentPtr> doc = clientAccessor.getListDocumentsWithPrivateData("default", "test_read");

            if(doc.size() != 1)
            {
                throw std::runtime_error("Not the good number of documents: expected 1, received "+std::to_string(doc.size()));
            }
            
            if(!doc[0]->isContainingPrivateData())
            {
                throw std::runtime_error("Document is not containing private data");
            }
            
            log_debug(" *<= Test #4.2 > OK");
            testsResults.emplace_back ("Test #4.2 getListDocumentsWithPrivateData tag=test_read",true);

        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #4.2 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #4.2 getListDocumentsWithPrivateData tag=test_read",false);
        }
    }
    
//test 4.3  => getListDocumentsWithPrivateData SecwIllegalAccess
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #4.3 getListDocumentsWithPrivateData => SecwIllegalAccess");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            clientAccessor.getListDocumentsWithPrivateData("default", "test_update");

            throw std::runtime_error("Documents are returned");
        }
        catch(const SecwIllegalAccess &)
        {
            log_debug(" *<= Test #4.3 > OK");
            testsResults.emplace_back ("Test #4.3 getListDocumentsWithPrivateData => SecwIllegalAccess",true);
        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #4.3 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #4.3 getListDocumentsWithPrivateData => SecwIllegalAccess",false);
        }
    }
    
//test 5.1 => getDocumentsWithoutPrivateData
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #5.1 getDocumentsWithoutPrivateData");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            std::vector<DocumentPtr> doc = clientAccessor.getDocumentsWithoutPrivateData("default");

            if(doc.size() != 2)
            {
                throw std::runtime_error("Not the good number of documents: expected 2, received "+std::to_string(doc.size()));
            }
            
            if(doc[0]->isContainingPrivateData())
            {
                throw std::runtime_error("Document is containing private data");
            }
            
            log_debug(" *<= Test #5.1 > OK");
            testsResults.emplace_back ("Test #5.1 getDocumentsWithoutPrivateData",true);

        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #5.1 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #5.1 getDocumentsWithoutPrivateData",false);
        }
    }
    
//test 5.2 => getDocumentsWithoutPrivateData => SecwUnknownTagException
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #5.2 getDocumentsWithoutPrivateData => SecwUnknownTagException");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            clientAccessor.getDocumentsWithoutPrivateData("default", "test_not_exist_tag");

            throw std::runtime_error("Documents are returned");
        }
        catch(const SecwUnknownTagException &)
        {
            log_debug(" *<= Test #5.2 > OK");
            testsResults.emplace_back ("Test #5.2 getDocumentsWithoutPrivateData => SecwUnknownTagException",true);
        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #5.2 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #5.2 getDocumentsWithoutPrivateData => SecwUnknownTagException",false);
        }
    }
    
//test 6 => getDocumentWithPrivateData
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #6 getDocumentWithPrivateData");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            DocumentPtr doc = clientAccessor.getDocumentWithPrivateData("default", "id_readable");

            if(!doc->isContainingPrivateData())
            {
                throw std::runtime_error("Document is not containing private data");
            }
            
            log_debug(" *<= Test #6 > OK");
            testsResults.emplace_back ("Test #6 getDocumentWithPrivateData",true);

        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #6 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #6 getDocumentWithPrivateData",false);
        }
    }
    
//test 7 => getDocumentWithoutPrivateData
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #7 getDocumentWithoutPrivateData");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            DocumentPtr doc = clientAccessor.getDocumentWithoutPrivateData("default", "id_readable");

            if(doc->isContainingPrivateData())
            {
                throw std::runtime_error("Document is containing private data");
            }
            
            log_debug(" *<= Test #7 > OK");
            testsResults.emplace_back ("Test #7 getDocumentWithoutPrivateData",true);

        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #7 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #7 getDocumentWithoutPrivateData",false);
        }
    }
    
//test 8 => SecwDocumentDoNotExistException 1/2
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #8 SecwDocumentDoNotExistException 1/2");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            clientAccessor.getDocumentWithoutPrivateData("default", "XXXXX-XXXXXXXXX");

            throw std::runtime_error("Document is return");
        }
        catch(const SecwDocumentDoNotExistException &)
        {
            log_debug(" *<= Test #8 > OK");
            testsResults.emplace_back ("Test #8 SecwDocumentDoNotExistException 1/2",true);
        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #8 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #8 SecwDocumentDoNotExistException 1/2",false);
        }
    }
    
//test 9 => SecwDocumentDoNotExistException 2/2
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #9 SecwDocumentDoNotExistException 2/2");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            clientAccessor.getDocumentWithPrivateData("default", "XXXXX-XXXXXXXXX");

            throw std::runtime_error("Document is return");
        }
        catch(const SecwDocumentDoNotExistException &)
        {
            log_debug(" *<= Test #9 > OK");
            testsResults.emplace_back ("Test #9 SecwDocumentDoNotExistException 2/2",true);
        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #9 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #9 SecwDocumentDoNotExistException 2/2",false);
        }
    }
    
//test 10  => SecwUnknownPortfolioException
    printf("\n-----------------------------------------------------------------------\n");
    {
        log_debug(" *=> Test #10 SecwUnknownPortfolioException");
        ClientAccessor clientAccessor(SELFTEST_CLIENT_ID);
        try
        {
            clientAccessor.getDocumentWithPrivateData("XXXXX", "XXXXX-XXXXXXXXX");

            throw std::runtime_error("Document is return");
        }
        catch(const SecwUnknownPortfolioException &)
        {
            log_debug(" *<= Test #10 > OK");
            testsResults.emplace_back ("Test #10 SecwUnknownPortfolioException",true);
        }
        catch(const std::exception& e)
        {
            log_debug(" *<= Test #10 > Failed");
            log_debug("Error: %s\n\n",e.what());
            testsResults.emplace_back ("Test #10 SecwUnknownPortfolioException",false);
        }
    }

    return testsResults;*/
    
}

