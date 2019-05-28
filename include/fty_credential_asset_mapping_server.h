/*  =========================================================================
    fty_credential_asset_mapping_server - Credential Asset Mapping Server

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

#ifndef FTY_CREDENTIAL_ASSET_MAPPING_SERVER_H_INCLUDED
#define FTY_CREDENTIAL_ASSET_MAPPING_SERVER_H_INCLUDED

#include "cam_credential_asset_mapping.h"

#include <czmq.h>
#include <malamute.h>
#include <fty_common_mlm.h>

#include <functional>

/**
 * \brief Agent CredentialAssetMappingServer main server actor
 */

namespace cam
{
    using Command   = std::string;
    using Sender    = std::string;
    using Subject   = std::string;
    
    using FctCommandHandler = std::function<std::string (const Sender &, const std::vector<std::string> &)>;

    class CredentialAssetMappingStorage;

    class CredentialAssetMappingServer final : public mlm::MlmAgent
    {

    public:
        explicit CredentialAssetMappingServer(zsock_t *pipe);

    private:
        bool handleMailbox(zmsg_t *message) override;
        bool handlePipe(zmsg_t *message) override;
        
        // List of supported commands with a reference to the handler for this command.
        std::map<Command, FctCommandHandler> m_supportedCommands;

        static std::shared_ptr<CredentialAssetMappingStorage> m_activeMapping;
        
        //Handler for all supported commands
        static std::string handleCreateMapping(const Sender & sender, const std::vector<std::string> & params);
        static std::string handleGetMapping(const Sender & sender, const std::vector<std::string> & params);
        static std::string handleUpdateCredentialMapping(const Sender & sender, const std::vector<std::string> & params);
        static std::string handleUpdateStatusMapping(const Sender & sender, const std::vector<std::string> & params);
        static std::string handleUpdateInfoMapping(const Sender & sender, const std::vector<std::string> & params);
        static std::string handleRemoveMapping(const Sender & sender, const std::vector<std::string> & params);
        static std::string handleGetAssetMappings(const Sender & sender, const std::vector<std::string> & params);
        static std::string handleGetCredentialMappings(const Sender & sender, const std::vector<std::string> & params);
        static std::string handleCountCredentialMappingsForCredential(const Sender & sender, const std::vector<std::string> & params);


        
        //Helpers
        static zmsg_t *generateErrorMsg(const std::string & correlationId, const std::string & errPayload);
    
    public:
        //Command list
        static constexpr const char* CREATE_MAPPING = "CREATE_MAPPING";
        static constexpr const char* GET_MAPPING = "GET_MAPPING";
        static constexpr const char* UPDATE_CREDENTIAL_MAPPING = "UPDATE_CREDENTIAL_MAPPING";
        static constexpr const char* UPDATE_STATUS_MAPPING = "UPDATE_STATUS_MAPPING";
        static constexpr const char* UPDATE_INFO_MAPPING = "UPDATE_INFO_MAPPING";
        static constexpr const char* REMOVE_MAPPING = "REMOVE_MAPPING";

        static constexpr const char* GET_ASSET_MAPPINGS = "GET_ASSET_MAPPINGS";
        static constexpr const char* GET_CRED_MAPPINGS = "GET_CRED_MAPPINGS";

        static constexpr const char* COUNT_CRED_MAPPINGS = "COUNT_CRED_MAPPINGS";

    };
    
} // namespace cam

//  @interface
//  Create an fty_credential_asset_mapping_server actor
void fty_credential_asset_mapping_server(zsock_t *pipe, void *endpoint);

void fty_credential_asset_mapping_server_test (bool verbose);

#endif
