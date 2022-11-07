/*  =========================================================================
    cam_credential_asset_mapping_server - Credential Asset Mapping Server

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

#pragma once

#include "cam_accessor.h"
#include "cam_credential_asset_mapping_storage.h"
#include <fty_common_client.h>
#include <fty_common_sync_server.h>
#include <functional>
#include <fty_srr_dto.h>
#include <fty_common_messagebus.h>
#include <mutex>

/**
 * \brief Agent CredentialAssetMappingServer main server actor
 */

namespace cam {

using Command = std::string;
using Sender  = std::string;
using Subject = std::string;

using FctCommandHandler = std::function<std::string(const Sender&, const std::vector<std::string>&)>;

class CredentialAssetMappingServer final : public fty::SyncServer
{

public:
    explicit CredentialAssetMappingServer(
        const std::string& storagePath,
        const std::string& srrEndpoint = "",
        const std::string& srrAgentName = ""
    );

    ~CredentialAssetMappingServer();

    std::vector<std::string> handleRequest(const Sender& sender, const std::vector<std::string>& payload) override;

    // List of supported commands with a reference to the handler for this command.
    std::map<Command, FctCommandHandler> m_supportedCommands;

    CredentialAssetMappingStorage m_activeMapping;

    // Handler for all supported commands
    std::string handleCreateMapping(const Sender& sender, const std::vector<std::string>& params);
    std::string handleGetMapping(const Sender& sender, const std::vector<std::string>& params);
    std::string handleUpdateMapping(const Sender& sender, const std::vector<std::string>& params);
    std::string handleUpdatePortMapping(const Sender& sender, const std::vector<std::string>& params);
    std::string handleUpdateCredentialMapping(const Sender& sender, const std::vector<std::string>& params);
    std::string handleUpdateStatusMapping(const Sender& sender, const std::vector<std::string>& params);
    std::string handleUpdateInfoMapping(const Sender& sender, const std::vector<std::string>& params);
    std::string handleRemoveMapping(const Sender& sender, const std::vector<std::string>& params);
    std::string handleGetAssetMappings(const Sender& sender, const std::vector<std::string>& params);
    std::string handleGetMappings(const Sender& sender, const std::vector<std::string>& params);
    std::string handleGetAllMappings(const Sender& sender, const std::vector<std::string>& params);
    std::string handleGetCredentialMappings(const Sender& sender, const std::vector<std::string>& params);
    std::string handleCountCredentialMappingsForCredential(const Sender& sender, const std::vector<std::string>& params);

public:
    // Command list
    static constexpr const char* CREATE_MAPPING            = "CREATE_MAPPING";
    static constexpr const char* GET_MAPPING               = "GET_MAPPING";
    static constexpr const char* UPDATE_MAPPING            = "UPDATE_MAPPING";
    static constexpr const char* UPDATE_PORT_MAPPING       = "UPDATE_PORT_MAPPING";
    static constexpr const char* UPDATE_CREDENTIAL_MAPPING = "UPDATE_CREDENTIAL_MAPPING";
    static constexpr const char* UPDATE_STATUS_MAPPING     = "UPDATE_STATUS_MAPPING";
    static constexpr const char* UPDATE_INFO_MAPPING       = "UPDATE_INFO_MAPPING";
    static constexpr const char* REMOVE_MAPPING            = "REMOVE_MAPPING";
    static constexpr const char* GET_ASSET_MAPPINGS        = "GET_ASSET_MAPPINGS";
    static constexpr const char* GET_CRED_MAPPINGS         = "GET_CRED_MAPPINGS";
    static constexpr const char* GET_MAPPINGS              = "GET_MAPPINGS";
    static constexpr const char* GET_ALL_MAPPINGS          = "GET_ALL_MAPPINGS";
    static constexpr const char* COUNT_CRED_MAPPINGS       = "COUNT_CRED_MAPPINGS";

public:
    // SRR support
    std::unique_ptr<messagebus::MessageBus>      m_msgBus;
    std::mutex                                   m_lock;
    std::unique_ptr<dto::srr::SrrQueryProcessor> m_srrProcessor;

    void                      handleSRRRequest(messagebus::Message msg);
    dto::srr::SaveResponse    handleSRRSave(const dto::srr::SaveQuery& query);
    dto::srr::RestoreResponse handleSRRRestore(const dto::srr::RestoreQuery& query);
};

} // namespace cam
