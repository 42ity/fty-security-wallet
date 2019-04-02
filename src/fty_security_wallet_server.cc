/*  =========================================================================
    fty_security_wallet_server - Security Wallet Server

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
    fty_security_wallet_server - secw broker agent
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

SecurityWalletServer::SecurityWalletServer(zsock_t *pipe)
    : FtyActor(pipe)
{

}

bool SecurityWalletServer::handlePipe(zmsg_t *message)
{
    bool rv = true;
    ZstrGuard actor_command(zmsg_popstr(message));

    // $TERM actor command implementation is required by zactor_t interface.
    if (streq(actor_command, "$TERM")) {
        rv = false;
    }
    else if (streq (actor_command, "CONNECT")) {
        ZstrGuard endpoint(zmsg_popstr(message));
        ZstrGuard name(zmsg_popstr (message));
        if (endpoint && name) {
            connect(endpoint,name);
        }
    }
    else if (streq (actor_command, "STORAGE_PATH")) {
        ZstrGuard storage_path(zmsg_popstr(message));
        int rv=m_storage.load(storage_path.get());
        log_debug("%d portfolio found in %s",rv,storage_path.get());
    }
    else {
        log_error("Unknown pipe command '%s'.", actor_command.get());
    }

    return rv;
}

zmsg_t * SecurityWalletServer::generateErrorMsg( ZstrGuard &correlation_id,  const char* err_msg)
{
    zmsg_t *error = zmsg_new ();
    if (correlation_id.get())
        zmsg_addstr (error, correlation_id);
    zmsg_addstr (error, "ERROR");
    zmsg_addstr (error, err_msg);
    return error;
}

zmsg_t * SecurityWalletServer::generateErrorMsg(   const char* err_msg)
{
    zmsg_t *error = zmsg_new ();
    zmsg_addstr (error, "ERROR");
    zmsg_addstr (error, err_msg);
    return error;
}

zmsg_t * SecurityWalletServer::handleGetDummyRequest(ZstrGuard &correlation_id, ZstrGuard &portfolio_name, bool isSecret)
{
    
    std::string _portfolio_name="default";
    if(portfolio_name)
        _portfolio_name=portfolio_name;
    zmsg_t *reply_msg=zmsg_new ();
    if (correlation_id.get())
        zmsg_addstr (reply_msg, correlation_id);
    std::string reply="[";
    SecurityWalletPotfolio portfolio=m_storage.getPortfolio(_portfolio_name);
    //TODO => use ref
    std::vector<Document> documents = portfolio.m_documents;
    for (int n=0; n<documents.size();n++){
        if(isSecret)
        {
            reply+=documents[n].toJSON_withSecret();
        }
        else
        {
            reply+=documents[n].toJSON_withoutSecret();
        }
        
    }
    reply+="]";
    zmsg_addstr (reply_msg, reply.c_str());
    return reply_msg;
}

bool SecurityWalletServer::handleMailbox(zmsg_t *message)
{
    log_debug("Received mailbox message '%s' from '%s'.", mlm_client_subject(m_client), mlm_client_sender(m_client));
    // we don't care the subject .. may be enforced if needed later. 
    try {
        zmsg_t *reply = NULL;
        ZstrGuard command (zmsg_popstr (message));
        ZstrGuard correlation_id (zmsg_popstr (message));
        if (!command){
            log_warning ("Received empty command ");
            reply = generateErrorMsg( "No command");
        }
        else if (streq (command, "GET_DUMMY_WITH_SECRET")){
            ZstrGuard portfolio_name (zmsg_popstr (message));
            log_debug("DO %s %s",command.get(), portfolio_name.get());
            reply = handleGetDummyRequest(correlation_id, portfolio_name,true);
        }
        else if (streq (command, "GET_DUMMY_WITHOUT_SECRET")){
            ZstrGuard portfolio_name (zmsg_popstr (message));
            log_debug("DO %s %s",command.get(), portfolio_name.get());
            reply = handleGetDummyRequest(correlation_id, portfolio_name,false);
        }
        else if (streq (command, "CREATE") ||
                streq (command, "DELETE") ||
                streq (command, "UPDATE_WITHOUT_SECRET") ||
                streq (command, "UPDATE_WITH_SECRET") ||
                streq (command, "GET_WITHOUT_SECRET") ||
                streq (command, "GET_WITH_SECRET")) {
            reply = generateErrorMsg(correlation_id, "Not yet implemented");
        }
        else if (streq (command, "ERROR")) {
            // Don't reply to ERROR messages
            log_warning ("Received ERROR command from '%s', ignoring",
                mlm_client_sender (m_client));
        }
        else 
        {
            log_warning ("Received invalid command '%s'", command.get());
            reply = generateErrorMsg(correlation_id, "invalid command");
        }

        if (reply) {
            int rv =
            mlm_client_sendto (m_client, mlm_client_sender (m_client),
                                "REPLY", NULL, 1000, &reply);
            if (rv != 0)
                log_error ("s_handle_mailbox: failed to send reply to %s ",
                        mlm_client_sender (m_client));
        }
    }
    catch (std::exception &e) {
        log_error("%s", e.what());
    }

    return true;
}

void fty_security_wallet_server(zsock_t *pipe, void *args)
{
    SecurityWalletServer server(pipe);
    server.mainloop();
}




//  --------------------------------------------------------------------------
//  Self test of this class

// If your selftest reads SCMed fixture data, please keep it in
// src/selftest-ro; if your test creates filesystem objects, please
// do so under src/selftest-rw.
// The following pattern is suggested for C selftest code:
//    char *filename = NULL;
//    filename = zsys_sprintf ("%s/%s", SELFTEST_DIR_RO, "mytemplate.file");
//    assert (filename);
//    ... use the "filename" for I/O ...
//    zstr_free (&filename);
// This way the same "filename" variable can be reused for many subtests.
#define SELFTEST_DIR_RO "src/selftest-ro"
#define SELFTEST_DIR_RW "src/selftest-rw"

void
fty_security_wallet_server_test (bool verbose)
{
    printf (" * fty_security_wallet_server: ");
    assert (SELFTEST_DIR_RO);
    assert (SELFTEST_DIR_RW);

    static const char* endpoint = "inproc://fty-security-walletg-test";

    zactor_t *broker = zactor_new (mlm_server, (void*) "Malamute");
    zstr_sendx (broker, "BIND", endpoint, NULL);
    if (verbose)
        zstr_send (broker, "VERBOSE");
    
    zactor_t *server = zactor_new (fty_security_wallet_server, (void *)endpoint);
    //set configuration parameters
    zstr_sendx (server, "STORAGE_PATH", SELFTEST_DIR_RW, NULL);
    zstr_sendx (server, "CONNECT", endpoint, SECURITY_WALLET_AGENT, NULL);

    mlm_client_t *client = mlm_client_new ();
    mlm_client_connect (client, endpoint, 1000, "secw-client");
    
    //test 1 => Invalid REQUEST command
    {
        log_debug ("fty_security_wallet_server:Test #1");
        // Invalid REQUEST command
        zmsg_t *request = zmsg_new();
        zmsg_addstr (request, "NON_EXISTENT_COMMAND");
        ZuuidGuard  zuuid(zuuid_new ());
        zmsg_addstr (request, zuuid_str_canonical (zuuid));
        mlm_client_sendto (client, SECURITY_WALLET_AGENT, "REQUEST", NULL, 1000, &request);

        ZmsgGuard recv(mlm_client_recv (client));
        assert (zmsg_size (recv) == 3);
        ZstrGuard str(zmsg_popstr (recv));
        assert (streq (str, zuuid_str_canonical (zuuid)));
        str = zmsg_popstr (recv);
        assert (streq (str, "ERROR"));
        str = zmsg_popstr (recv);
        assert (streq (str, "invalid command"));

    }

    mlm_client_destroy (&client);
    zstr_sendm (server, "$TERM");
    sleep(1);

    
    zactor_destroy (&server);
    zactor_destroy (&broker);

    printf ("OK\n");
}
