/*  =========================================================================
    fty_security_wallet_socket_agent - Security Wallet malamute agent

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

#include "fty_security_wallet_classes.h"

#include "secw_exception.h"
#include "secw_security_wallet_server.h"
#include "secw_security_wallet.h"
#include "secw_helpers.h"

#include "fty_common_mlm_stream_client.h"
#include "fty_common_socket_basic_mailbox_server.h"
#include "fty_common_socket_sync_client.h"

#include <sstream>
#include <fstream>
#include <thread>

#include <cxxtools/jsonserializer.h>

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

void
fty_security_wallet_socket_agent_test (bool verbose)
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
    
    //New section => we need to destroy clients before broker
    {
        //setup parameters for the agent
        std::map<std::string, std::string> paramsSecw;

        paramsSecw["STORAGE_CONFIGURATION_PATH"] = SELFTEST_DIR_RW"/configuration.json";
        paramsSecw["STORAGE_DATABASE_PATH"] = SELFTEST_DIR_RW"/data.json";
        paramsSecw["AGENT_NAME"] = SECURITY_WALLET_AGENT;
        paramsSecw["ENDPOINT"] = endpoint;
        
        //create a stream publisher for notification
        mlm::MlmStreamClient notificationStream(SECURITY_WALLET_AGENT, SECW_NOTIFICATIONS, 1000, paramsSecw.at("ENDPOINT"));

        //create the server
        secw::SecurityWalletServer serverSecw(  paramsSecw.at("STORAGE_CONFIGURATION_PATH"),
                                        paramsSecw.at("STORAGE_DATABASE_PATH"),
                                        notificationStream);
        
        fty::SocketBasicServer agentSecw( serverSecw, "secw-test.socket");
        std::thread agentSecwThread(&fty::SocketBasicServer::run, &agentSecw);
  
        //create the 2 Clients
        fty::SocketSyncClient syncClient("secw-test.socket");
        mlm::MlmStreamClient streamClient("secw-server-test", SECW_NOTIFICATIONS, 1000, endpoint);

        //Tests from the lib
        std::vector<std::pair<std::string,bool>> testLibConsumerResults = secw_consumer_accessor_test(syncClient, streamClient);
        std::vector<std::pair<std::string,bool>> testLibProducerResults = secw_producer_accessor_test(syncClient, streamClient);
        
        printf("\n-----------------------------------------------------------------------\n");

        uint32_t testsPassed = 0;
        uint32_t testsFailed = 0;

        printf ("\n\n ** fty_security_wallet_mlm_agent: \n\n");

        printf("\tTests from the lib: Consumer part\n");
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


        agentSecw.requestStop();
        agentSecwThread.join();
    }

    zactor_destroy (&broker);
}

