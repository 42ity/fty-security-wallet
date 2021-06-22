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

#include "fty_security_wallet.h"
#include "secw_exception.h"
#include "secw_helpers.h"
#include "secw_security_wallet_server.h"
#include <cxxtools/jsonserializer.h>
#include <fty_common_mlm_basic_mailbox_server.h>
#include <fty_common_mlm_stream_client.h>
#include <sstream>

void fty_security_wallet_mlm_agent(zsock_t* pipe, void* args)
{
    using Arguments = std::map<std::string, std::string>;

    const Arguments& arguments = *static_cast<Arguments*>(args);

    // create a stream publisher for notification
    mlm::MlmStreamClient notificationStream(SECURITY_WALLET_AGENT, SECW_NOTIFICATIONS, 1000, arguments.at("ENDPOINT"));

    // create the server
    secw::SecurityWalletServer server(arguments.at("STORAGE_CONFIGURATION_PATH"), arguments.at("STORAGE_DATABASE_PATH"),
        notificationStream, arguments.at("ENDPOINT_SRR"), arguments.at("AGENT_NAME_SRR"));

    // launch the agent
    mlm::MlmBasicMailboxServer agent(pipe, server, arguments.at("AGENT_NAME"), arguments.at("ENDPOINT"));
    agent.mainloop();

    std::cerr << "Leave the agent" << std::endl;
}
