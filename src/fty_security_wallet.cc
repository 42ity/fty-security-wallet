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

void usage(){
    puts (SECURITY_WALLET_AGENT " [options] ...");
    puts ("  -v|--verbose        verbose test output");
    puts ("  -h|--help           this information");
    puts ("  -c|--config         path to config file");
}

int main (int argc, char *argv [])
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

    if (verbose)
    {
        ftylog_setVeboseMode(ftylog_getInstance());
        log_trace("Verbose mode OK");
    }

    // Parse config file
    ZstrGuard actor_name(strdup(SECURITY_WALLET_AGENT));
    ZstrGuard endpoint( strdup(DEFAULT_ENDPOINT));
    ZstrGuard storage_path( strdup(DEFAULT_STORGAE_PATH));
    char *log_config = NULL;
    if(config_file) {
        log_debug (SECURITY_WALLET_AGENT ": loading configuration file from '%s' ...", config_file);
        ZconfigGuard config(zconfig_load (config_file));
        if (!config) {
            log_error ("Failed to load config file %s: %m", config_file);
            exit (EXIT_FAILURE);
        }
        // VERBOSE
        if (streq (zconfig_get (config, "server/verbose", "false"), "true")) {
            verbose = true;
        }
        endpoint = strdup (zconfig_get (config, "malamute/endpoint", DEFAULT_ENDPOINT));
        actor_name = strdup (zconfig_get (config, "malamute/address", DEFAULT_ENDPOINT));
        storage_path = strdup (zconfig_get (config, "storage/path", DEFAULT_STORGAE_PATH));
    }
    log_info(SECURITY_WALLET_AGENT " starting");

    //start broker agent
    zactor_t *server = zactor_new (fty_security_wallet_server, (void *)endpoint);
    //set configuration parameters
    zstr_sendx (server, "STORAGE_PATH", storage_path.get(), NULL);
    zstr_sendx (server, "CONNECT", endpoint.get(), actor_name.get(), NULL);
    
    while (true) {
        char *str = zstr_recv (server);
        if (str) {
            puts (str);
            zstr_free (&str);
        }
        else {
            log_info ("Interrupted ...");
            break;
        }
    }

    zactor_destroy(&server);

    return 0;

}
