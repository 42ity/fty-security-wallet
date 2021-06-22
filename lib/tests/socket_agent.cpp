#include <catch2/catch.hpp>
#include <czmq.h>
#include <fstream>
#include <fty_common_mlm.h>
#include <fty_common_socket.h>
#include <fty_security_wallet.h>
#include <map>
#include <mlm_server.h>
#include <src/secw_security_wallet_server.h>
#include "consumer_accessor.h"
#include "producer_accessor.h"

TEST_CASE("Security wallet socket agent test")
{
    INFO("\n\n ** fty_security_wallet_mlm_agent: \n\n");

    static const char* endpoint = "inproc://fty-security-walletg-test";

    // Copy the database file
    {
        std::ifstream source("tests/selftest-ro/data.json", std::ios::binary);
        std::ofstream dest("data.json", std::ios::binary | std::ofstream::trunc);
        dest << source.rdbuf();

        dest.close();
        source.close();
    }

    // Copy the configuration file
    {
        std::ifstream source("tests/selftest-ro/configuration.json", std::ios::binary);
        std::ofstream dest("configuration.json", std::ios::binary | std::ofstream::trunc);
        dest << source.rdbuf();

        dest.close();
        source.close();
    }

    // create the broker
    zactor_t* broker = zactor_new(mlm_server, const_cast<char*>("Malamute"));
    zstr_sendx(broker, "BIND", endpoint, NULL);
    zstr_send(broker, "VERBOSE");

    // New section => we need to destroy clients before broker
    {
        // setup parameters for the agent
        std::map<std::string, std::string> paramsSecw;

        paramsSecw["STORAGE_CONFIGURATION_PATH"] = "configuration.json";
        paramsSecw["STORAGE_DATABASE_PATH"]      = "data.json";
        paramsSecw["AGENT_NAME"]                 = SECURITY_WALLET_AGENT;
        paramsSecw["ENDPOINT"]                   = endpoint;

        // create a stream publisher for notification
        mlm::MlmStreamClient notificationStream(
            SECURITY_WALLET_AGENT, SECW_NOTIFICATIONS, 1000, paramsSecw.at("ENDPOINT"));

        // create the server
        secw::SecurityWalletServer serverSecw(
            paramsSecw.at("STORAGE_CONFIGURATION_PATH"), paramsSecw.at("STORAGE_DATABASE_PATH"), notificationStream);

        fty::SocketBasicServer agentSecw(serverSecw, "secw-test.socket");
        std::thread            agentSecwThread(&fty::SocketBasicServer::run, &agentSecw);

        // create the 2 Clients
        fty::SocketSyncClient syncClient("secw-test.socket");
        mlm::MlmStreamClient  streamClient("secw-server-test", SECW_NOTIFICATIONS, 1000, endpoint);

        // Tests from the lib
        secwConsumerAccessorTest(syncClient, streamClient);
        secwProducerAccessorTest(syncClient, streamClient);

        agentSecw.requestStop();
        agentSecwThread.join();
    }

    zactor_destroy(&broker);
}
