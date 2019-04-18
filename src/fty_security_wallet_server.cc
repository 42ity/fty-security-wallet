/*  =========================================================================
    fty_security_wallet_server - Security Wallet Server

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
    fty_security_wallet_server - secw broker agent
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

#include "secw_exception.h"
#include "secw_security_wallet.h"

#include <sstream>
#include <cxxtools/jsonserializer.h>

using namespace secw;

std::shared_ptr<SecurityWallet> SecurityWalletServer::m_activeWallet = std::shared_ptr<SecurityWallet>(nullptr);

SecurityWalletServer::SecurityWalletServer(zsock_t *pipe)
    : mlm::MlmAgent(pipe)
{
    //initiate the commands handlers
    m_supportedCommands[GET_PORTFOLIO_LIST] = handleGetListPortfolio;

    m_supportedCommands[GET_LIST_WITH_SECRET] = handleGetListDocumentsWithSecret;
    m_supportedCommands[GET_LIST_WITHOUT_SECRET] = handleGetListDocumentsWithoutSecret;

    m_supportedCommands[GET_EDITABLE_TAGS] = handleGetListEditableTags;
    m_supportedCommands[GET_READABLE_TAGS] = handleGetListReadableTags;

    m_supportedCommands[GET_WITHOUT_SECRET] = handleGetDocumentWithoutSecret;
    m_supportedCommands[GET_WITH_SECRET] = handleGetDocumentWithSecret;
    
    m_supportedCommands[CREATE] = handleNotImplementedCmd;
    m_supportedCommands[DELETE] = handleNotImplementedCmd;
    m_supportedCommands[UPDATE] = handleNotImplementedCmd;
}

/* Commands implementation section*/
//TODO remove
std::string SecurityWalletServer::handleNotImplementedCmd(const Sender & /*sender*/, const std::vector<std::string> & /*params*/)
{
    /*
     * No parameters for this command.
     * 
     */
    
    throw SecwException("Command is not implemented yet!!");
}

std::string SecurityWalletServer::handleGetListPortfolio(const Sender & /*sender*/, const std::vector<std::string> & /*params*/)
{
    /*
     * No parameters for this command.
     * 
     */
    
    cxxtools::SerializationInfo si;
    si <<= m_activeWallet->getPortfolioNames();
    
    return toJsonFromSerializationInfo(si);
}


std::string SecurityWalletServer::handleGetDocumentWithSecret(const Sender & sender, const std::vector<std::string> & params)
{
    /*
     * Parameters for this command:
     * 
     * 0. name of the portfolio
     * 1. document id
     */
    
    if(params.size() != 2)
    {
        throw SecwBadCommandArgumentException("Command need at 2 arguments");
    }

    const std::string & portfolioName = params[0];
    const Id & id = params[1];
    
    DocumentPtr doc  = m_activeWallet->getPortfolio(portfolioName).getDocument(id);
    
    //check if we have the access
    if( ! Document::hasCommonTag(doc->getTags(), m_activeWallet->getAccessibleTags(sender, READ_ACCESS)))
    {
        throw SecwIllegalAccess("Access to private date of "+id+" is not available for "+sender);
    }
    
    cxxtools::SerializationInfo si;
    
    doc->fillSerializationInfoWithSecret(si);
    
    return toJsonFromSerializationInfo(si);
}
std::string SecurityWalletServer::handleGetDocumentWithoutSecret(const Sender & /*sender*/, const std::vector<std::string> & params)
{
    /*
     * Parameters for this command:
     * 
     * 0. name of the portfolio
     * 1. document id
     */
    if(params.size() != 2)
    {
        throw SecwBadCommandArgumentException("Command need at 2 arguments");
    }
    
    const std::string & portfolioName = params[0];
    const Id & id = params[1];
    
    DocumentPtr doc  = m_activeWallet->getPortfolio(portfolioName).getDocument(id);
    
    cxxtools::SerializationInfo si;
    
    doc->fillSerializationInfoWithoutSecret(si);
    
    return toJsonFromSerializationInfo(si);
}
        
std::string SecurityWalletServer::handleGetListReadableTags(const Sender & sender, const std::vector<std::string> & /*params*/)
{
    /*
     * No parameters for this command.
     * 
     */
    
    cxxtools::SerializationInfo si;
    si <<= m_activeWallet->getAccessibleTags(sender, READ_ACCESS);
    
    return toJsonFromSerializationInfo(si);
}

std::string SecurityWalletServer::handleGetListEditableTags(const Sender & sender, const std::vector<std::string> & /*params*/)
{
    /*
     * No parameters for this command.
     * 
     */

    cxxtools::SerializationInfo si;
    si <<= m_activeWallet->getAccessibleTags(sender, UPDATE_ACCESS);
    
    return toJsonFromSerializationInfo(si);
}


std::string SecurityWalletServer::handleGetListDocumentsWithSecret(const Sender & sender, const std::vector<std::string> & params)
{
    /*
     * Parameters for this command:
     * 
     * 0. name of the portfolio
     * 1. tag of documents (optional)
     * 2. type of documents (optional)
     */
    
    if(params.size() < 1)
    {
        throw SecwBadCommandArgumentException("Command need at least argument");
    }

    const std::string & portfolioName = params[0];

    std::string debugInfo = "DO GetListDocumentsWithSecret on portfolio <"+portfolioName+">";
    
    Tag tag = ""; //by default all the tag
    DocumentType type = ""; //by default all the type

    if(params.size() >= 2)
    {
        tag = params[1];
    }
 
    if(params.size() >= 3)
    {
        type = params[2];
    }

    if(!tag.empty())
    {
        debugInfo += " with the tag '"+tag+"'";
    }
    else
    {
        debugInfo+= " without specific tag";
    }

    if(!type.empty())
    {
        debugInfo+= " with the type '"+type+"'";
    }
    else
    {
        debugInfo += " without specific type";
    }

    log_debug("%s",debugInfo.c_str());

    //prepare request
    std::vector<DocumentType> types;
    std::vector<Tag> tags;

    //check if the type is supported
    if(!type.empty())
    {
        if(!Document::isSupportedType(type)) throw SecwUnknownPortfolioException("Document type "+type+" is not supported");
        types = {type};
    }
    else
    {
        types = Document::getSupportedTypes();
    }

    //Check if we are allow to request
    if(tag.empty())
    {
        tags = m_activeWallet->getAccessibleTags(sender, READ_ACCESS);
    }
    else
    {
        if(!m_activeWallet->checkTagAccess(tag, sender, READ_ACCESS))
        {
            throw SecwIllegalAccess("Forbidden to retrieve private data from the tag "+tag);
        }

        tags = {tag};

    }
    
    return serializeListDocumentsPrivate(portfolioName, types, tags);
}

std::string SecurityWalletServer::handleGetListDocumentsWithoutSecret(const Sender & /*sender*/, const std::vector<std::string> & params)
{
    /*
     * Parameters for this command:
     * 
     * 0. name of the portfolio
     * 1. tag of documents (optional) => default all
     * 2. type of documents (optional) => default all 
     */
    
    if(params.size() < 1)
    {
        throw SecwBadCommandArgumentException("Command need at least argument");
    }

    const std::string & portfolioName = params[0];
    Tag tag = ""; //by default all the tag
    DocumentType type = ""; //by default all the type

    std::string debugInfo = "DO GetListDummyDocumentsWithoutSecret on portfolio <"+portfolioName+">";
    
    if(params.size() >= 2)
    {
        tag = params[1];
        debugInfo += " with the tag '"+tag+"'";
    }
    else
    {
        debugInfo+= " without specific tag";
    }
    
    
    if(params.size() >= 3)
    {
        //type has been specified
        type = params[2];
        debugInfo+= " with the type '"+type+"'";
    }
    else
    {
        debugInfo += " without specific type";
    }

    log_debug("%s",debugInfo.c_str());

    //prepare request
    std::vector<DocumentType> types;
    std::vector<Tag> tags;

    if(type.empty())
    {
        types = Document::getSupportedTypes();
    }
    else
    {
        if(!Document::isSupportedType(type)) throw SecwUnknownPortfolioException("Document type "+type+" is not supported");
        types = {type};
    }
    
    if(!tag.empty())
    {
        if(! Document::hasCommonTag({tag},m_activeWallet->getAvailableTags()))
        {
            throw SecwUnknownTagException("Tag "+tag+" is unknown");
        }

        tags = {tag};
    }
    
    return serializeListDocumentsPublic(portfolioName, types, tags);
}

/* Agents methods*/

static std::string g_storageAccessPath = DEFAULT_STORAGE_ACCESS_PATH;
static std::string g_storageDatabasePath = DEFAULT_STORAGE_DATABASE_PATH;

bool SecurityWalletServer::handlePipe(zmsg_t *message)
{
    bool rv = true;
    ZstrGuard actor_command(zmsg_popstr(message));

    // $TERM actor command implementation is required by zactor_t interface.
    if (streq(actor_command, "$TERM"))
    {
        rv = false;
    }
    else if (streq (actor_command, "CONNECT"))
    {
        m_activeWallet = std::make_shared<SecurityWallet>(g_storageAccessPath, g_storageDatabasePath);
        
        log_debug("Wallet found in %s",g_storageDatabasePath.c_str());

        ZstrGuard endpoint(zmsg_popstr(message));
        ZstrGuard name(zmsg_popstr (message));
        if (endpoint && name) {
            connect(endpoint,name);
        }
    }
    else if (streq (actor_command, "STORAGE_ACCESS_PATH"))
    {
        ZstrGuard storage_access_path(zmsg_popstr(message));
        g_storageAccessPath = std::string(storage_access_path.get());
    }
    else if (streq (actor_command, "STORAGE_DATABASE_PATH"))
    {
        ZstrGuard storage_database_path(zmsg_popstr(message));
        g_storageDatabasePath = std::string(storage_database_path.get());
    }
    else
    {
        log_error("Unknown pipe command '%s'", actor_command.get());
    }

    return rv;
}

bool SecurityWalletServer::handleMailbox(zmsg_t *message)
{
    std::string correlationId;
      
    //try to address the request
    try
    {
        Subject subject(mlm_client_subject(client()));
        Sender sender(mlm_client_sender(client()));
            
        //ignore none "REQUEST" message
        if (subject != "REQUEST")
        {
            log_warning ("Received mailbox message with subject '%s' from '%s', ignoring", subject.c_str(),sender.c_str());
            return true;
        }
        
        //Get number of frame all the frame
        size_t numberOfFrame = zmsg_size(message);
        
        log_debug("Received mailbox message with subject '%s' from '%s' with %i frames", subject.c_str(), sender.c_str(), numberOfFrame);

        
        /*  Message is valid if the header contain at least the following frame:
         * 0. Correlation id
         * 1. Command
         */
        
        //TODO define a maximum to avoid DOS
        
        ZstrGuard ptrCorrelationId( zmsg_popstr(message) );
        ZstrGuard ptrCommand( zmsg_popstr(message) );
        
        std::vector<std::string> params;
    
        //we unstack all the other starting by the 3rd one.
        for(size_t index = 2; index < numberOfFrame; index++)
        {
            ZstrGuard param( zmsg_popstr(message) );
            params.push_back( std::string(param.get()) );
        }
        
        //Ensure the presence of data from the request
        if(ptrCorrelationId != nullptr)
        {
            correlationId = std::string(ptrCorrelationId.get());
        }
        
        if(correlationId.empty())
        {
            throw SecwProtocolErrorException("Correlation id frame is empty");
        }
        
        Command command;
        
        if(ptrCommand != nullptr)
        {
            command = Command(ptrCommand.get());
        }
              
        if (command.empty())
        {
            throw SecwProtocolErrorException("Command frame is empty");
        }
        
        // Don't reply to ERROR messages
        if (command == "ERROR")
        {
            log_warning ("Received <%s> message from '%s', ignoring", command.c_str(),
                mlm_client_sender (client()) );
            
            return true;
        }
        
        //check if the command exist in the system
        if( m_supportedCommands.count(command) == 0)
        {
            throw SecwUnsupportedCommandException(command + " not supported");
        }

        FctCommandHandler cmdHandler = m_supportedCommands[command];
        
        //Execute the command
        std::string result = cmdHandler(sender, params);

        //send the result
        zmsg_t *reply = zmsg_new();
        
        zmsg_addstr (reply, correlationId.c_str());
        zmsg_addstr (reply, result.c_str());

        int rv = mlm_client_sendto (client(), mlm_client_sender (client()),
                            "REPLY", NULL, 1000, &reply);
        if (rv != 0)
        {
            log_error ("s_handle_mailbox: failed to send reply to %s ",
                    mlm_client_sender (client()));
        }

    }
    catch(SecwException &e)
    {
        log_warning("%s", e.what());
        
        //send the error
        zmsg_t *reply = generateErrorMsg(correlationId, e.toJson());
        
        int rv = mlm_client_sendto (client(), mlm_client_sender (client()),
                                "REPLY", NULL, 1000, &reply);
        if (rv != 0)
        {
            log_error ("secw_handle_mailbox: failed to send reply to %s ",
                        mlm_client_sender (client()));
        }
    }
    catch (std::exception &e)
    {
        log_error("Unexpected error: %s", e.what());
        
        //send the error
        zmsg_t *reply = generateErrorMsg(correlationId, "");
        
        int rv = mlm_client_sendto (client(), mlm_client_sender (client()),
                                "REPLY", NULL, 1000, &reply);
        if (rv != 0)
        {
            log_error ("secw_handle_mailbox: failed to send reply to %s ",
                        mlm_client_sender (client()));
        }
    }
    catch (...) //show must go one => Log and ignore the unknown error
    {
        log_error("Unexpected error: unknown");
        
        //send the error
        zmsg_t *reply = generateErrorMsg(correlationId, "");
        
        int rv = mlm_client_sendto (client(), mlm_client_sender (client()),
                                "REPLY", NULL, 1000, &reply);
        if (rv != 0)
        {
            log_error ("secw_handle_mailbox: failed to send reply to %s ",
                        mlm_client_sender (client()));
        }
    }

    return true;
}

void fty_security_wallet_server(zsock_t *pipe, void *args)
{
    SecurityWalletServer server(pipe);
    server.mainloop();
}

/* Helpers section */

zmsg_t * SecurityWalletServer::generateErrorMsg( const std::string & correlationId,  const std::string & errPayload)
{
    zmsg_t *error = zmsg_new();
    zmsg_addstr (error, correlationId.c_str());
    zmsg_addstr (error, "ERROR");
    zmsg_addstr (error, errPayload.c_str());
    return error;
}

std::string SecurityWalletServer::toJsonFromSerializationInfo(const cxxtools::SerializationInfo& si)
{
    std::stringstream output;
    cxxtools::JsonSerializer serializer(output);
    serializer.beautify(true);
    serializer.serialize(si);
    
    return output.str();
}

std::string SecurityWalletServer::serializeListDocumentsPublic(const std::string & portfolioName, const std::vector<DocumentType> & types, const std::vector<Tag> & tags)
{
    Portfolio & portfolio = m_activeWallet->getPortfolio(portfolioName);

    //get the documents
    cxxtools::SerializationInfo si;
    
    for (const auto pDoc : portfolio.getListDocuments(types, tags) )
    {
        pDoc->fillSerializationInfoWithoutSecret(si.addMember(""));
  
    }

    si.setCategory(cxxtools::SerializationInfo::Array);

    return toJsonFromSerializationInfo(si);

}

std::string SecurityWalletServer::serializeListDocumentsPrivate(const std::string & portfolioName, const std::vector<DocumentType> & types, const std::vector<Tag> & tags)
{
    Portfolio & portfolio = m_activeWallet->getPortfolio(portfolioName);

    //get the documents
    cxxtools::SerializationInfo si;
    
    for (const auto pDoc : portfolio.getListDocuments(types, tags) )
    {
        pDoc->fillSerializationInfoWithSecret(si.addMember(""));  
    }

    si.setCategory(cxxtools::SerializationInfo::Array);

    return toJsonFromSerializationInfo(si);
}


//  --------------------------------------------------------------------------
//  Self test of this class

// If your selftest reads SCMed fixture data, please keep it in
// src/selftest-ro; if your test creates filesystem objects, please
// do so under src/selftest-rw.
// The following pattern is suggested for C selftest code:
//    char *filename = NULL;
//    filename = zsys_sprintf ("%s/%s", SELFTEST_DIR_RO, "mytemplate.file");
//    assert (filename);
//    ... use the "filename" for I/O ...
//    zstr_free (&filename);
// This way the same "filename" variable can be reused for many subtests.
#define SELFTEST_DIR_RO "src/selftest-ro"
#define SELFTEST_DIR_RW "src/selftest-rw"

#include <fstream>

void
fty_security_wallet_server_test (bool verbose)
{
    std::vector<std::pair<std::string,bool>> testsServerResults;
    printf ("\n ** fty_security_wallet_server: \n");
    assert (SELFTEST_DIR_RO);
    assert (SELFTEST_DIR_RW);

    static const char* endpoint = "inproc://fty-security-walletg-test";

    //Copy the database file
    {
        std::ifstream source(SELFTEST_DIR_RO"/data.json", std::ios::binary);
        std::ofstream dest(SELFTEST_DIR_RW"/data.json", std::ios::binary | std::ofstream::trunc );
        dest << source.rdbuf();

        dest.close();
        source.close();
    }

    //Copy the access file
    {
        std::ifstream source(SELFTEST_DIR_RO"/access.json", std::ios::binary);
        std::ofstream dest(SELFTEST_DIR_RW"/access.json", std::ios::binary | std::ofstream::trunc );
        dest << source.rdbuf();

        dest.close();
        source.close();
    }

    zactor_t *broker = zactor_new (mlm_server, (void*) "Malamute");
    zstr_sendx (broker, "BIND", endpoint, NULL);
    if (verbose)
        zstr_send (broker, "VERBOSE");
    
    zactor_t *server = zactor_new (fty_security_wallet_server, (void *)endpoint);
    //set configuration parameters
    zstr_sendx (server, "STORAGE_ACCESS_PATH", SELFTEST_DIR_RO"/access.json", NULL);
    zstr_sendx (server, "STORAGE_DATABASE_PATH", SELFTEST_DIR_RW"/data.json", NULL);
    zstr_sendx (server, "CONNECT", endpoint, SECURITY_WALLET_AGENT, NULL);

    mlm_client_t *client = mlm_client_new();
    mlm_client_connect (client, endpoint, 1000, "secw-server-test");

    //test 1 => Invalid REQUEST command
    printf("\n-----------------------------------------------------------------------\n");
    try
    {
        log_debug ("*=> fty_security_wallet_server > Test #1 Invalid REQUEST command");
            
        zmsg_t *request = zmsg_new();
        ZuuidGuard  zuuid(zuuid_new ());
        zmsg_addstr (request, zuuid_str_canonical (zuuid));
        zmsg_addstr (request, "NON_EXISTENT_COMMAND");
        mlm_client_sendto (client, SECURITY_WALLET_AGENT, "REQUEST", NULL, 1000, &request);

        ZmsgGuard recv(mlm_client_recv (client));
        if(zmsg_size (recv) != 3)
        {
            throw std::runtime_error("Bad number of frames received");
        }
        ZstrGuard str(zmsg_popstr (recv));
        if(!streq (str, zuuid_str_canonical (zuuid)))
        {
            throw std::runtime_error("Bad correlation id received");
        }

        str = zmsg_popstr (recv);
        if(!streq (str, "ERROR"))
        {
            throw std::runtime_error("Bad message type received: "+std::string(str));
        }

        testsServerResults.emplace_back("fty_security_wallet_server > Test #1 Invalid REQUEST command",true);

    }
    catch(const std::exception& e)
    {
        log_debug(" *<= secw_client_accessor_test > Test #1 > Failed");
        log_debug("Error: %s\n\n",e.what());

        testsServerResults.emplace_back("fty_security_wallet_server > Test #1 Invalid REQUEST command",false);
    }
    
    //end of the server tests
    mlm_client_destroy(&client);

    //Tests from the lib
    std::vector<std::pair<std::string,bool>> testLibResults = secw_client_accessor_test();

    printf("\n-----------------------------------------------------------------------\n");
    
    uint32_t testsPassed = 0;
    uint32_t testsFailed = 0;
    
    //Print all the result
    printf("\n\tTests from the server:\n");
    for(const auto & result : testsServerResults)
    {
        if(result.second)
        {
            printf("\tOK \t%s\n",result.first.c_str());
            testsPassed++;
        }
        else
        {
            printf("\tNOK\t%s\n",result.first.c_str());
            testsFailed++;
        }
    }
    
    printf("\n\tTests from the lib:\n");
    for(const auto & result : testLibResults)
    {
        if(result.second)
        {
            printf("\tOK \t%s\n",result.first.c_str());
            testsPassed++;
        }
        else
        {
            printf("\tNOK\t%s\n",result.first.c_str());
            testsFailed++;
        }
    }

    printf("\n-----------------------------------------------------------------------\n");
    
    if(testsFailed == 0)
    {
        printf("\n %i tests passed, everything is ok\n\n",testsPassed);
    }
    else
    {
        printf("\n!!!!!!!! At %i/%i tests did not pass !!!!!!!! \n\n",testsFailed,(testsPassed+testsFailed));
        assert(false);
    }


    zstr_sendm (server, "$TERM");
    sleep(1);

    zactor_destroy (&server);
    zactor_destroy (&broker);
}
