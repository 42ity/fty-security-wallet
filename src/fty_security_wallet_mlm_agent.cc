/*  =========================================================================
    fty_security_wallet_mlmagent - Security Wallet malamute agent

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
 * Agents methods
 */

#include "fty_security_wallet_classes.h"

#include "secw_exception.h"
#include "secw_security_wallet_server.h"
#include "secw_security_wallet.h"
#include "secw_helpers.h"

#include <sys/types.h>
#include <gnu/libc-version.h>

//gettid() is available since glibc 2.30
#if ((__GLIBC__ < 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ < 30))
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

#include <iomanip>
#include <sstream>
#include <cxxtools/jsonserializer.h>

using namespace secw;
using namespace std::placeholders;

SecurityWalletMlmAgent::SecurityWalletMlmAgent(zsock_t *pipe)
    :   mlm::MlmAgent(pipe),
        m_storageconfigurationPath(DEFAULT_STORAGE_CONFIGURATION_PATH),
        m_storageDatabasePath(DEFAULT_STORAGE_DATABASE_PATH),
        m_endpoint(DEFAULT_ENDPOINT)
{}

bool SecurityWalletMlmAgent::handlePipe(zmsg_t *message)
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
        ZstrGuard endpoint(zmsg_popstr(message));
        m_endpoint = std::string(endpoint.get());
        
        ZstrGuard name(zmsg_popstr (message));
        
        m_secwServer = std::make_shared<SecurityWalletServer>
                            (   m_storageconfigurationPath,
                                m_storageDatabasePath,
                                std::bind(&SecurityWalletMlmAgent::publishOnBus, this, _1)
                            );
        
        log_debug("Wallet found in %s", m_storageDatabasePath.c_str());
        
        
        if (name)
        {
            connect(m_endpoint.c_str() ,name);
        }
    }
    else if (streq (actor_command, "STORAGE_CONFIGURATION_PATH"))
    {
        ZstrGuard storage_access_path(zmsg_popstr(message));
        m_storageconfigurationPath = std::string(storage_access_path.get());
    }
    else if (streq (actor_command, "STORAGE_DATABASE_PATH"))
    {
        ZstrGuard storage_database_path(zmsg_popstr(message));
        m_storageDatabasePath = std::string(storage_database_path.get());
    }
    else
    {
        log_error("Unknown pipe command '%s'", actor_command.get());
    }

    return rv;
}

bool SecurityWalletMlmAgent::handleMailbox(zmsg_t *message)
{
    std::string correlationId;
      
    //try to address the request
    try
    {
        Subject subject(mlm_client_subject(client()));
        Sender uniqueSender(mlm_client_sender(client()));
            
        //ignore none "REQUEST" message
        if (subject != "REQUEST")
        {
            log_warning ("Received mailbox message with subject '%s' from '%s', ignoring", subject.c_str(),uniqueSender.c_str());
            return true;
        }
        
        //Get number of frame all the frame
        size_t numberOfFrame = zmsg_size(message);
        
        log_debug("Received mailbox message with subject '%s' from '%s' with %i frames", subject.c_str(), uniqueSender.c_str(), numberOfFrame);

        
        /*  Message is valid if the header contain at least the following frame:
         * 0. Correlation id
         * 1. data
         */
        
        //TODO define a maximum to avoid DOS
        
        ZstrGuard ptrCorrelationId( zmsg_popstr(message) );
        
        std::vector<std::string> payload;
    
        //we unstack all the other starting by the 3rd one.
        for(size_t index = 1; index < numberOfFrame; index++)
        {
            ZstrGuard param( zmsg_popstr(message) );
            payload.push_back( std::string(param.get()) );
        }
        
        //Ensure the presence of data from the request
        if(ptrCorrelationId != nullptr)
        {
            correlationId = std::string(ptrCorrelationId.get());
        }
        
        if(correlationId.empty())
        {
            //no correlation id, it's a bad frame we ignore it
            throw std::runtime_error("Correlation id frame is empty");
        }
        
        //extract the sender from unique sender id: <Sender>.[thread id in hexa]
        Sender sender = uniqueSender.substr(0, (uniqueSender.size()-(sizeof(pid_t)*2)-1));
        
        //Execute the request
        std::vector<std::string> results = m_secwServer->handleRequest(sender, payload);

        //send the result if it's not empty
        if(!results.empty())
        {
            zmsg_t *reply = zmsg_new();
        
            zmsg_addstr (reply, correlationId.c_str());
            
            for(const std::string & result : results)
            {
                zmsg_addstr (reply, result.c_str());
            }

            int rv = mlm_client_sendto (client(), mlm_client_sender (client()),
                                "REPLY", NULL, 1000, &reply);
            if (rv != 0)
            {
                log_error ("s_handle_mailbox: failed to send reply to %s ",
                        mlm_client_sender (client()));
            }
        }
        
    }
    catch (std::exception &e)
    {
        log_error("Unexpected error: %s", e.what());
    }
    catch (...) //show must go one => Log and ignore the unknown error
    {
        log_error("Unexpected error: unknown");
    }

    return true;
}

void SecurityWalletMlmAgent::publishOnBus(const std::string & payload)
{
    //std::cerr << "Publish on Bus <" << messageType << ">:" << payload << std::endl;
    
    mlm_client_t * client = mlm_client_new();

    if(client == NULL)
    {
      mlm_client_destroy(&client);
      throw SecwMalamuteClientIsNullException();
    }

    //create a unique sender id: SECURITY_WALLET_AGENT.[thread id in hexa]
    pid_t threadId = gettid();

    std::stringstream ss;
    ss << SECURITY_WALLET_AGENT  << "." << std::setfill('0') << std::setw(sizeof(pid_t)*2) << std::hex << threadId;

    std::string uniqueId = ss.str();

    int rc = mlm_client_connect (client, m_endpoint.c_str(), 1000, uniqueId.c_str());

    if (rc != 0)
    {
      mlm_client_destroy(&client);
      throw SecwMalamuteConnectionFailedException();
    }

    rc = mlm_client_set_producer (client, SECW_NOTIFICATIONS);
    if (rc != 0)
    {
        mlm_client_destroy (&client);
        throw SecwMalamuteInterruptedException();
    }

    zmsg_t *notification = zmsg_new ();
    zmsg_addstr (notification, payload.c_str ());

    rc = mlm_client_send (client, "NOTIFICATION", &notification);

    if (rc != 0)
    {
      zmsg_destroy(&notification);
      mlm_client_destroy(&client);
      throw SecwMalamuteInterruptedException();
    }

    mlm_client_destroy (&client);
    
    //std::cerr << "Publish on Bus Done!" << std::endl;
}

/* external interface */

void fty_security_wallet_mlm_agent(zsock_t *pipe, void *args)
{
    SecurityWalletMlmAgent agent(pipe);
    agent.mainloop();
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

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#include <fstream>

void
fty_security_wallet_mlm_agent_test (bool verbose)
{
    std::vector<std::pair<std::string,bool>> testsServerResults;
    printf ("\n ** fty_security_wallet_mlm_agent: \n");
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

    //Copy the configuration file
    {
        std::ifstream source(SELFTEST_DIR_RO"/configuration.json", std::ios::binary);
        std::ofstream dest(SELFTEST_DIR_RW"/configuration.json", std::ios::binary | std::ofstream::trunc );
        dest << source.rdbuf();

        dest.close();
        source.close();
    }

    zactor_t *broker = zactor_new (mlm_server, (void*) "Malamute");
    zstr_sendx (broker, "BIND", endpoint, NULL);
    if (verbose)
        zstr_send (broker, "VERBOSE");
    
    zactor_t *server = zactor_new (fty_security_wallet_mlm_agent, (void *)endpoint);
    //set configuration parameters
    zstr_sendx (server, "STORAGE_CONFIGURATION_PATH", SELFTEST_DIR_RW"/configuration.json", NULL);
    zstr_sendx (server, "STORAGE_DATABASE_PATH", SELFTEST_DIR_RW"/data.json", NULL);
    zstr_sendx (server, "CONNECT", endpoint, SECURITY_WALLET_AGENT, NULL);

    mlm_client_t *client = mlm_client_new();
    mlm_client_connect (client, endpoint, 1000, "secw-server-test");

    //test 1 => Invalid REQUEST command
    printf("\n-----------------------------------------------------------------------\n");
    try
    {
        log_debug ("*=> Test #1 Invalid REQUEST command");
            
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

        testsServerResults.emplace_back("Test #1 Invalid REQUEST command",true);

    }
    catch(const std::exception& e)
    {
        log_debug(" *<= Test #1 > Failed");
        log_debug("Error: %s\n\n",e.what());

        testsServerResults.emplace_back("Test #1 Invalid REQUEST command",false);
    }
    
    //end of the server tests
    mlm_client_destroy(&client);

    //Tests from the lib
    std::vector<std::pair<std::string,bool>> testLibConsumerResults = secw_consumer_accessor_test();
    std::vector<std::pair<std::string,bool>> testLibProducerResults = secw_producer_accessor_test();

    printf("\n-----------------------------------------------------------------------\n");
    
    uint32_t testsPassed = 0;
    uint32_t testsFailed = 0;
    
    //Print all the result
    printf("\n\tTests from the server:\n");
    for(const auto & result : testsServerResults)
    {
        if(result.second)
        {
            printf(ANSI_COLOR_GREEN"\tOK " ANSI_COLOR_RESET "\t%s\n",result.first.c_str());
            testsPassed++;
        }
        else
        {
            printf(ANSI_COLOR_RED"\tNOK" ANSI_COLOR_RESET "\t%s\n",result.first.c_str());
            testsFailed++;
        }
    }
    
    printf("\n\tTests from the lib: Consumer part\n");
    for(const auto & result : testLibConsumerResults)
    {
        if(result.second)
        {
            printf(ANSI_COLOR_GREEN"\tOK " ANSI_COLOR_RESET "\t%s\n",result.first.c_str());
            testsPassed++;
        }
        else
        {
            printf(ANSI_COLOR_RED"\tNOK" ANSI_COLOR_RESET "\t%s\n",result.first.c_str());
            testsFailed++;
        }
    }

    printf("\n\tTests from the lib: Producer part\n");
    for(const auto & result : testLibProducerResults)
    {
        if(result.second)
        {
            printf(ANSI_COLOR_GREEN"\tOK " ANSI_COLOR_RESET "\t%s\n",result.first.c_str());
            testsPassed++;
        }
        else
        {
            printf(ANSI_COLOR_RED"\tNOK" ANSI_COLOR_RESET "\t%s\n",result.first.c_str());
            testsFailed++;
        }
    }

    printf("\n-----------------------------------------------------------------------\n");
    
    if(testsFailed == 0)
    {
        printf(ANSI_COLOR_GREEN"\n %i tests passed, everything is ok\n" ANSI_COLOR_RESET "\n",testsPassed);
    }
    else
    {
        printf(ANSI_COLOR_RED"\n!!!!!!!! %i/%i tests did not pass !!!!!!!! \n" ANSI_COLOR_RESET "\n",testsFailed,(testsPassed+testsFailed));

        printf("Content of the database at the end of tests: \n");
        printf("\n\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

        std::ifstream database(SELFTEST_DIR_RW"/data.json", std::ios::binary);
        std::cerr << database.rdbuf() << std::endl;

        database.close();
        printf("\n\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

        assert(false);
    }


    zstr_sendm (server, "$TERM");
    sleep(1);

    zactor_destroy (&server);
    zactor_destroy (&broker);
}
