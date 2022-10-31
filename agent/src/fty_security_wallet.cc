/*  =========================================================================
    fty_security_wallet - Security Wallet Binary

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
    fty_security_wallet - Security Wallet Binary
@discuss
@end
*/

#if defined(HAVE_LIBSYSTEMD)
#pragma message "HAVE_LIBSYSTEMD defined"
#include <systemd/sd-daemon.h>
#endif

#include "fty_security_wallet.h"
#include <fty_common_mlm.h>
#include <fty_common_socket.h>
#include <fty_log.h>
#include <thread>
#include "src/secw_security_wallet_server.h"

static void usage()
{
    puts(SECURITY_WALLET_AGENT " [options] ...");
    puts("  -v|--verbose        verbose output");
    puts("  -h|--help           this information");
    puts("  -c|--config <path>  load a configuration file");
}

int main(int argc, char* argv[])
{
    using Arguments = std::map<std::string, std::string>;

    zactor_t* server_cam = NULL;
    bool processSuccess = false;

    try {
        ftylog_setInstance(SECURITY_WALLET_AGENT, FTY_COMMON_LOGGING_DEFAULT_CFG);

        char* config_file = NULL;
        bool  verbose     = false;

        // Parse command line
        for (int argn = 1; argn < argc; argn++) {
            char* param = (argn < argc - 1) ? argv[argn + 1] : NULL;

            if (streq(argv[argn], "--help") || streq(argv[argn], "-h")) {
                usage();
                return EXIT_SUCCESS;
            }
            else if (streq(argv[argn], "--verbose") || streq(argv[argn], "-v")) {
                verbose = true;
            }
            else if (streq(argv[argn], "--config") || streq(argv[argn], "-c")) {
                if (!param) {
                    fprintf(stderr, "%s: path argument is missing\n", argv[argn]);
                    return EXIT_FAILURE;
                }
                config_file = param;
                ++argn;
            }
            else {
                fprintf(stderr, "%s: unkonwn argument\n", argv[argn]);
                return EXIT_FAILURE;
            }
        }

        // Defaults
        std::string endpoint(DEFAULT_ENDPOINT);

        std::string secw_actor_name(SECURITY_WALLET_AGENT);
        std::string socketPath(DEFAULT_SOCKET);
        std::string storage_database_path(DEFAULT_STORAGE_DATABASE_PATH);
        std::string storage_access_path(DEFAULT_STORAGE_CONFIGURATION_PATH);

        std::string mapping_actor_name(MAPPING_AGENT);
        std::string mapping_actor_name_srr(MAPPING_AGENT_SRR);
        std::string storage_mapping_path(DEFAULT_STORAGE_MAPPING_PATH);

        // Parse config file
        if (config_file) {
            log_debug(SECURITY_WALLET_AGENT ": loading configuration file from '%s' ...", config_file);
            mlm::ZConfig config(config_file);

            verbose |= (config.getEntry("server/verbose", "false") == "true");

            endpoint = config.getEntry("secw-malamute/endpoint", DEFAULT_ENDPOINT);

            secw_actor_name       = config.getEntry("secw-malamute/address", SECURITY_WALLET_AGENT);
            socketPath            = config.getEntry("secw-socket/socket", DEFAULT_SOCKET);
            storage_database_path = config.getEntry("secw-storage/database", DEFAULT_STORAGE_DATABASE_PATH);
            storage_access_path   = config.getEntry("secw-storage/configuration", DEFAULT_STORAGE_CONFIGURATION_PATH);

            mapping_actor_name     = config.getEntry("mapping-malamute/address", MAPPING_AGENT);
            mapping_actor_name_srr = config.getEntry("mapping-malamute/address_srr", MAPPING_AGENT_SRR);
            storage_mapping_path   = config.getEntry("mapping-storage/database", MAPPING_AGENT);
        }

        log_debug(SECURITY_WALLET_AGENT ":"
            "\n\tendpoint: '%s'"
            "\n\tsecw_actor_name: '%s'"
            "\n\tsocketPath: '%s'"
            "\n\tstorage_access_path: '%s'"
            "\n\tstorage_database_path: '%s'"
            "\n\tmapping_actor_name: '%s'"
            "\n\tmapping_actor_name_srr: '%s'"
            "\n\tstorage_mapping_path: '%s'"
            , endpoint.c_str()
            , secw_actor_name.c_str()
            , socketPath.c_str()
            , storage_access_path.c_str()
            , storage_database_path.c_str()
            , mapping_actor_name.c_str()
            , mapping_actor_name_srr.c_str()
            , storage_mapping_path.c_str()
        );

        if (verbose) {
            ftylog_setVerboseMode(ftylog_getInstance());
            log_trace(SECURITY_WALLET_AGENT ": Verbose mode");
        }

        log_info(SECURITY_WALLET_AGENT " starting...");

        // params for SECW
        Arguments paramsSecw;
        paramsSecw["STORAGE_CONFIGURATION_PATH"] = storage_access_path;
        paramsSecw["STORAGE_DATABASE_PATH"]      = storage_database_path;
        paramsSecw["AGENT_NAME"]                 = secw_actor_name;
        paramsSecw["ENDPOINT"]                   = endpoint;
        paramsSecw["AGENT_NAME_SRR"]             = secw_actor_name;
        paramsSecw["ENDPOINT_SRR"]               = endpoint;

        // create a stream publisher for SECW notification
        mlm::MlmStreamClient notificationStream(
            SECURITY_WALLET_AGENT, SECW_NOTIFICATIONS, 1000, paramsSecw.at("ENDPOINT"));

        // create/start the SECW server
        secw::SecurityWalletServer serverSecw(
            paramsSecw.at("STORAGE_CONFIGURATION_PATH"),
            paramsSecw.at("STORAGE_DATABASE_PATH"),
            notificationStream,
            paramsSecw.at("ENDPOINT_SRR"),
            paramsSecw.at("AGENT_NAME_SRR")
        );

        fty::SocketBasicServer agentSecw(serverSecw, socketPath);

        std::thread agentSecwThread(&fty::SocketBasicServer::run, &agentSecw);

        // params for CAM (Credentials Asset Mapping)
        Arguments paramsCam;
        paramsCam["STORAGE_MAPPING_PATH"] = storage_mapping_path;
        paramsCam["AGENT_NAME"]           = mapping_actor_name;
        paramsCam["ENDPOINT"]             = endpoint;
        paramsCam["AGENT_NAME_SRR"]       = mapping_actor_name_srr;
        paramsCam["ENDPOINT_SRR"]         = endpoint;

        // start CAM actor
        server_cam = zactor_new(fty_credential_asset_mapping_mlm_agent, static_cast<void*>(&paramsCam));
        if (!server_cam) {
            throw std::runtime_error("CAM server creation failed");
        }

#if defined(HAVE_LIBSYSTEMD)
        // notify systemd that the socket is ready, so that depending units can start
        // TODO: somehow self-check that the agent/actor/threads are all usable
        // and ready to respond to queries - only then notify that we are ready
        log_debug(SECURITY_WALLET_AGENT ": notifying systemd that this unit is ready to serve");
        sd_notify(0, "READY=1");
#endif

        log_info(SECURITY_WALLET_AGENT " started");

        // main loop, accept any message back from server
        // copy from src/malamute.c under MPL license
        while (!zsys_interrupted) {
            char* msg = zstr_recv(server_cam);
            if (!msg) {
                break;
            }
            log_debug(SECURITY_WALLET_AGENT " recv msg '%s'", msg);
            zstr_free(&msg);
        }

        log_info(SECURITY_WALLET_AGENT " ending...");

#if defined(HAVE_LIBSYSTEMD)
        // notify systemd that the service is stopping, so that depending units can be stopped too
        log_debug(SECURITY_WALLET_AGENT ": notifying systemd that this unit is beginning its shutdown");
        sd_notify(0, "STOPPING=1");
        // TODO: somehow wait here to make sure all consumers have disconnected?
#endif

        // actually stop security wallet
        zactor_destroy(&server_cam);
        agentSecw.requestStop();
        agentSecwThread.join();

        log_info(SECURITY_WALLET_AGENT " ended");
        processSuccess = true;

    } catch (const std::exception& e) {
        log_error(SECURITY_WALLET_AGENT ": Error '%s'", e.what());
    } catch (...) {
        log_error(SECURITY_WALLET_AGENT ": Error unknown");
    }

    zactor_destroy(&server_cam);

    return processSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}
