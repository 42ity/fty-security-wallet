/*void fty_security_wallet_mlm_agent_test(bool verbose)
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

        zactor_t *server = zactor_new (fty_security_wallet_mlm_agent, static_cast<void*>(&paramsSecw));

        //create the 2 Clients
        mlm::MlmSyncClient syncClient("secw-server-test", SECURITY_WALLET_AGENT, 1000, endpoint);
        mlm::MlmStreamClient streamClient("secw-server-test", SECW_NOTIFICATIONS, 1000, endpoint);

        //Tests from the lib
        std::vector<std::pair<std::string,bool>> testLibConsumerResults = secw_consumer_accessor_test(syncClient,
    streamClient); std::vector<std::pair<std::string,bool>> testLibProducerResults =
    secw_producer_accessor_test(syncClient, streamClient);

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
            printf(ANSI_COLOR_RED"\n!!!!!!!! %i/%i tests did not pass !!!!!!!! \n" ANSI_COLOR_RESET
    "\n",testsFailed,(testsPassed+testsFailed));

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

    zactor_destroy (&broker);
}*/
