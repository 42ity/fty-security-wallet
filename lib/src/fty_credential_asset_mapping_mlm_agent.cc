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

#include <fty_common_mlm_basic_mailbox_server.h>
#include <map>
#include "cam_credential_asset_mapping_server.h"

void fty_credential_asset_mapping_mlm_agent(zsock_t* pipe, void* args)
{
    using Arguments = std::map<std::string, std::string>;

    const Arguments& arguments = *static_cast<Arguments*>(args);

    // create the server
    cam::CredentialAssetMappingServer server(arguments.at("STORAGE_MAPPING_PATH"));

    // launch the agent
    mlm::MlmBasicMailboxServer agent(pipe, server, arguments.at("AGENT_NAME"), arguments.at("ENDPOINT"));
    agent.mainloop();
}
