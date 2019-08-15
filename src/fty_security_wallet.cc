/*  =========================================================================
    fty_security_wallet - Security Wallet Binary

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
    fty_security_wallet - Security Wallet Binary
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

#include "fty_common_mlm_zconfig.h"

void usage(){
    puts (SECURITY_WALLET_AGENT " [options] ...");
    puts ("  -v|--verbose        verbose test output");
    puts ("  -h|--help           this information");
    puts ("  -c|--config         path to config file");
}

int main (int argc, char *argv [])
{
    try
    {
    
        ftylog_setInstance(SECURITY_WALLET_AGENT,"");
        int argn;
        char *config_file = NULL;
        bool verbose = false;
        // Parse command line
        for (argn = 1; argn < argc; argn++) {
            char *param = NULL;
            if (argn < argc - 1) param = argv [argn+1];

            if (streq (argv [argn], "--help")
            ||  streq (argv [argn], "-h")) {
                usage();
                return 0;
            }
            else if (streq (argv [argn], "--verbose") || streq (argv [argn], "-v")) {
                verbose = true;
            }
            else if (streq (argv [argn], "--config") || streq (argv [argn], "-c")) {
                if (param) config_file = param;
                ++argn;
            }
        }
        
        // Parse config file
        std::string secw_actor_name(SECURITY_WALLET_AGENT);
        std::string endpoint(DEFAULT_ENDPOINT);
        std::string storage_database_path(DEFAULT_STORAGE_DATABASE_PATH);
        std::string storage_access_path(DEFAULT_STORAGE_CONFIGURATION_PATH);

        std::string mapping_actor_name(MAPPING_AGENT);
        std::string storage_mapping_path(DEFAULT_STORAGE_MAPPING_PATH);

        //char *log_config = NULL;
        if(config_file)
        {
            log_debug (SECURITY_WALLET_AGENT ": loading configuration file from '%s' ...", config_file);
            mlm::ZConfig config(config_file);

            verbose |= (config.getEntry("secw_server/verbose", "false") == "true");

            endpoint = config.getEntry("secw-malamute/endpoint", DEFAULT_ENDPOINT);
            secw_actor_name = config.getEntry( "secw-malamute/address", SECURITY_WALLET_AGENT);
            storage_database_path = config.getEntry( "secw-storage/database", DEFAULT_STORAGE_DATABASE_PATH);
            storage_access_path = config.getEntry( "secw-storage/configuration", DEFAULT_STORAGE_CONFIGURATION_PATH);

            mapping_actor_name = config.getEntry("mapping-malamute/address", MAPPING_AGENT);
            storage_mapping_path = config.getEntry( "mapping-storage/database", MAPPING_AGENT);
        }

        log_debug (SECURITY_WALLET_AGENT ": storage_access_path '%s'", storage_access_path.c_str());
        log_debug (SECURITY_WALLET_AGENT ": storage_database_path '%s'", storage_database_path.c_str());
        log_debug (SECURITY_WALLET_AGENT ": storage_mapping_path '%s'.", storage_mapping_path.c_str());

        if (verbose)
        {
            ftylog_setVeboseMode(ftylog_getInstance());
            log_trace("Verbose mode OK");
        }

        log_info(SECURITY_WALLET_AGENT " starting");

        //start broker agent
        zactor_t *secw_server = zactor_new (fty_security_wallet_mlm_agent, (void *)endpoint.c_str());
        //set configuration parameters
        zstr_sendx (secw_server, "STORAGE_CONFIGURATION_PATH", storage_access_path.c_str(), NULL);
        zstr_sendx (secw_server, "STORAGE_DATABASE_PATH", storage_database_path.c_str(), NULL);
        zstr_sendx (secw_server, "CONNECT", endpoint.c_str(), secw_actor_name.c_str(), NULL);

        //start broker agent
        zactor_t *cam_server = zactor_new (fty_credential_asset_mapping_server, (void *)endpoint.c_str());
        //set configuration parameters
        zstr_sendx (cam_server, "STORAGE_MAPPING_PATH", storage_mapping_path.c_str(), NULL);
        zstr_sendx (cam_server, "CONNECT",  endpoint.c_str(), mapping_actor_name.c_str(), NULL);

        while (true)
        {
            char *str = zstr_recv (secw_server);
            if (str)
            {
                puts (str);
                zstr_free (&str);
            }
            else
            {
                //stop everything
                break;
            }

            char *camStr = zstr_recv (cam_server);
            if (camStr)
            {
                puts (camStr);
                zstr_free (&camStr);
            }
            else
            {
                //stop everything
                break;
            }

        }

        log_info ("Secw Interrupted ...");
        zactor_destroy(&secw_server);

        log_info ("Cam Interrupted ...");
        zactor_destroy(&cam_server);

        return 0;
    }
    catch(std::exception & e)
    {
        log_error (SECURITY_WALLET_AGENT ": Error '%s'", e.what());
        exit(EXIT_FAILURE);
    }
    catch(...)
    {
        log_error (SECURITY_WALLET_AGENT ": Error unknown");
        exit(EXIT_FAILURE);
    }

}
