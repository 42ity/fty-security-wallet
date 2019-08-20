/*  =========================================================================
    fty_security_wallet_mlm_agent - Security Wallet malamute agent

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

#include "fty_common_mlm_stream_client.h"

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

SecurityWalletMlmAgent::SecurityWalletMlmAgent(zsock_t *pipe,
                                        const std::string & endpoint,
                                        const std::string & storageconfigurationPath,
                                        const std::string & storageDatabasePath,
                                        fty::StreamPublisher & notificationStream)
    :   mlm::MlmAgent(pipe),
        m_endpoint(endpoint),
        m_secwServer(new SecurityWalletServer
                                (   storageconfigurationPath,
                                    storageDatabasePath,
                                    notificationStream
                                )
                    )
{
    connect(m_endpoint.c_str(), SECURITY_WALLET_AGENT);
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

/* external interface */

void fty_security_wallet_mlm_agent(zsock_t *pipe, void *args)
{
    const secw::Arguments & arguments = *static_cast<secw::Arguments*>(args);
    
    mlm::MlmStreamClient notificationStream(SECURITY_WALLET_AGENT, SECW_NOTIFICATIONS, 1000, arguments.at("ENDPOINT"));
        
    secw::SecurityWalletMlmAgent agent(   pipe, 
                                    arguments.at("ENDPOINT"),
                                    arguments.at("STORAGE_CONFIGURATION_PATH"),
                                    arguments.at("STORAGE_DATABASE_PATH"),
                                    notificationStream
                                );
    agent.mainloop();
}




//  --------------------------------------------------------------------------
//  Self test


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
    printf ("\n\n ** fty_security_wallet_mlm_agent: \n\n");
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

    //create the broker 
    zactor_t *broker = zactor_new (mlm_server, (void*) "Malamute");
    zstr_sendx (broker, "BIND", endpoint, NULL);
    if (verbose)
        zstr_send (broker, "VERBOSE");
    
    //setup parameters for the agent
    std::map<std::string, std::string> paramsSecw;
    
    paramsSecw["STORAGE_CONFIGURATION_PATH"] = SELFTEST_DIR_RW"/configuration.json";
    paramsSecw["STORAGE_DATABASE_PATH"] = SELFTEST_DIR_RW"/data.json";
    paramsSecw["AGENT_NAME"] = SECURITY_WALLET_AGENT;
    paramsSecw["ENDPOINT"] = endpoint;
    
    zactor_t *server = zactor_new (fty_security_wallet_mlm_agent, static_cast<void*>(&paramsSecw));
  
    //create the 2 Clients
    mlm::MlmSyncClient syncClient("secw-server-test", SECURITY_WALLET_AGENT, 1000, endpoint);
    mlm::MlmStreamClient streamClient("secw-server-test", SECW_NOTIFICATIONS, 1000, endpoint);

    //Tests from the lib
    std::vector<std::pair<std::string,bool>> testLibConsumerResults = secw_consumer_accessor_test(syncClient, streamClient);
    std::vector<std::pair<std::string,bool>> testLibProducerResults = secw_producer_accessor_test(syncClient, streamClient);

    printf("\n-----------------------------------------------------------------------\n");
    
    uint32_t testsPassed = 0;
    uint32_t testsFailed = 0;
    
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
