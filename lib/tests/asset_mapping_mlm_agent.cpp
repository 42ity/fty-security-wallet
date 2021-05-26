#include <catch2/catch.hpp>
#include <map>
#include <fstream>
#include <fty_common_mlm.h>
#include "fty_security_wallet.h"
#include "cam_accessor.h"

TEST_CASE("credential asset mapping mlm agent test")
{
    using Arguments = std::map<std::string, std::string>;

    static const char* endpoint = "inproc://fty-credential-asset-mapping-test";

    // Copy the mapping file
    {
        std::ifstream source("tests/selftest-ro/mapping.json", std::ios::binary);
        std::ofstream dest("mapping.json", std::ios::binary | std::ofstream::trunc);
        dest << source.rdbuf();

        dest.close();
        source.close();
    }

    zactor_t* broker = zactor_new(mlm_server, const_cast<char*>("Malamute"));
    zstr_sendx(broker, "BIND", endpoint, NULL);
    zstr_send(broker, "VERBOSE");

    // set configuration parameters
    Arguments paramsCam;

    paramsCam["STORAGE_MAPPING_PATH"] = "mapping.json";
    paramsCam["AGENT_NAME"]           = MAPPING_AGENT;
    paramsCam["ENDPOINT"]             = endpoint;

    // start broker agent
    zactor_t* server = zactor_new(fty_credential_asset_mapping_mlm_agent, static_cast<void*>(&paramsCam));

    {
        // create the 1 Client
        mlm::MlmSyncClient syncClient("cam-server-test", MAPPING_AGENT, 1000, endpoint);

        // Tests from the lib
        camAccessorTest(syncClient);

        zstr_sendm(server, "$TERM");
        sleep(1);
    }

    zactor_destroy(&server);
    zactor_destroy(&broker);
}
