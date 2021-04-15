/*  =========================================================================
    fty_security_wallet_mlm_agent - Security Wallet malamute agent

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
 * Agents methods
 */

#include "fty_security_wallet_classes.h"

#include "secw_exception.h"
#include "secw_security_wallet_server.h"
#include "secw_helpers.h"

#include <fty_common_mlm_stream_client.h>
#include <fty_common_mlm_basic_mailbox_server.h>

#include <sstream>
#include <cxxtools/jsonserializer.h>

void fty_security_wallet_mlm_agent(zsock_t *pipe, void *args)
{
    using Arguments = std::map<std::string, std::string>;

    const Arguments & arguments = *static_cast<Arguments*>(args);

    //create a stream publisher for notification
    mlm::MlmStreamClient notificationStream(SECURITY_WALLET_AGENT, SECW_NOTIFICATIONS, 1000, arguments.at("ENDPOINT"));

    //create the server
    secw::SecurityWalletServer server(  arguments.at("STORAGE_CONFIGURATION_PATH"),
                                        arguments.at("STORAGE_DATABASE_PATH"),
                                        notificationStream,
                                        arguments.at("ENDPOINT_SRR"),
                                        arguments.at("AGENT_NAME_SRR"));

    //launch the agent
    mlm::MlmBasicMailboxServer agent(  pipe,
                                       server,
                                       arguments.at("AGENT_NAME"),
                                       arguments.at("ENDPOINT")
                                    );
    agent.mainloop();

    std::cerr << "Leave the agent" << std::endl;
}


//  --------------------------------------------------------------------------
//  Self test


#define SELFTEST_DIR_RO "selftest-ro"
#define SELFTEST_DIR_RW "selftest-rw"

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
    /*printf ("\n\n ** fty_security_wallet_mlm_agent: \n\n");
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


        zstr_sendm (server, "$TERM");
        sleep(1);

        zactor_destroy (&server);
    }

    zactor_destroy (&broker);*/
}
