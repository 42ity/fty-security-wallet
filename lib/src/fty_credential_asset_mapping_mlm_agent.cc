/*  =========================================================================
    fty_credential_asset_mapping_mlm_agent - Credential Asset Mapping malamute agent

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
    fty_credential_asset_mapping_mlm_agent - Credential Asset Mapping malamute agent
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

void fty_credential_asset_mapping_mlm_agent (zsock_t *pipe, void *args)
{
    using Arguments = std::map<std::string, std::string>;

    const Arguments & arguments = *static_cast<Arguments*>(args);

    //create the server
    cam::CredentialAssetMappingServer server(arguments.at("STORAGE_MAPPING_PATH"));

    //launch the agent
    mlm::MlmBasicMailboxServer agent(  pipe,
                                       server,
                                       arguments.at("AGENT_NAME"),
                                       arguments.at("ENDPOINT")
                                    );
    agent.mainloop();
}


//  --------------------------------------------------------------------------
//  Self test of this class

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
fty_credential_asset_mapping_mlm_agent_test (bool verbose)
{
    using Arguments = std::map<std::string, std::string>;

    printf ("\n ** fty_credential_asset_mapping_mlm_agent: \n");
    assert (SELFTEST_DIR_RO);
    assert (SELFTEST_DIR_RW);

    static const char* endpoint = "inproc://fty-credential-asset-mapping-test";

    //Copy the mapping file
    {
        std::ifstream source(SELFTEST_DIR_RO"/mapping.json", std::ios::binary);
        std::ofstream dest(SELFTEST_DIR_RW"/mapping.json", std::ios::binary | std::ofstream::trunc );
        dest << source.rdbuf();

        dest.close();
        source.close();
    }

    zactor_t *broker = zactor_new (mlm_server, (void*) "Malamute");
    zstr_sendx (broker, "BIND", endpoint, NULL);
    if (verbose)
        zstr_send (broker, "VERBOSE");

    //set configuration parameters
    Arguments paramsCam;

    paramsCam["STORAGE_MAPPING_PATH"] =  SELFTEST_DIR_RW"/mapping.json";
    paramsCam["AGENT_NAME"] = MAPPING_AGENT;
    paramsCam["ENDPOINT"] = endpoint;

    //start broker agent
    zactor_t *server = zactor_new (fty_credential_asset_mapping_mlm_agent,static_cast<void*>(&paramsCam));

    {
        //create the 1 Client
        mlm::MlmSyncClient syncClient("cam-server-test", MAPPING_AGENT, 1000, endpoint);

        //Tests from the lib
        std::vector<std::pair<std::string,bool>> testLibResults = cam_accessor_test(syncClient);

        printf("\n-----------------------------------------------------------------------\n");

        uint32_t testsPassed = 0;
        uint32_t testsFailed = 0;

        printf ("\n ** fty_credential_asset_mapping_mlm_agent: \n");

        printf("\tTests from the lib: \n");
        for(const auto & result : testLibResults)
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

            /*std::ifstream database(SELFTEST_DIR_RW"/mapping.json", std::ios::binary);
            std::cerr << database.rdbuf() << std::endl;

            database.close();*/
        }
        else
        {
            printf(ANSI_COLOR_RED"\n!!!!!!!! %i/%i tests did not pass !!!!!!!! \n" ANSI_COLOR_RESET "\n",testsFailed,(testsPassed+testsFailed));

            printf("Content of the mapping at the end of tests: \n");
            printf("\n\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

            std::ifstream database(SELFTEST_DIR_RW"/mapping.json", std::ios::binary);
            std::cerr << database.rdbuf() << std::endl;

            database.close();
            printf("\n\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

            assert(false);
        }


        zstr_sendm (server, "$TERM");
        sleep(1);

    }

    zactor_destroy (&server);
    zactor_destroy (&broker);
}

