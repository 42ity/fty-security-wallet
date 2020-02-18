/*  =========================================================================
    secw_security_wallet_server - Security Wallet Server

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

#ifndef SECW_SECURITY_WALLET_SERVER_H_INCLUDED
#define SECW_SECURITY_WALLET_SERVER_H_INCLUDED

#include <functional>
#include <memory>
#include "secw_security_wallet.h"
#include "secw_security_wallet_database_proxy.h"

#include "fty_common_client.h"
#include "fty_common_sync_server.h"

#include <cxxtools/serializationinfo.h>

#include "fty_srr_dto.h"
/**
 * \brief Agent SecurityWalletServer main server actor
 */
namespace secw
{
    using Command   = std::string;
    using Sender    = std::string;
    
    using FctCommandHandler = std::function<std::string (const Sender &, const std::vector<std::string> &)>;

    class SecurityWalletServer final : public fty::SyncServer
    {

    public:
        explicit SecurityWalletServer(  const std::string & configurationPath,
                                        const std::string & databasePath,
                                        fty::StreamPublisher & streamPublisher,
                                        const std::string & srrEndpoint = "",
                                        const std::string & srrAgentName = "",
                                        const std::string & dbUrl = "");

        ~SecurityWalletServer();
        
        std::vector<std::string> handleRequest(const Sender & sender, const std::vector<std::string> & payload) override;
        
    private:
        // List of supported commands with a reference to the handler for this command.
        std::map<Command, FctCommandHandler> m_supportedCommands;

        SecurityWallet m_activeWallet;
        fty::StreamPublisher & m_streamPublisher;
        std::unique_ptr<secw::SecurityWalletDatabaseProxy> m_dbProxy;
        
        //Handler for all supported commands
        std::string handleGetListDocumentsWithSecret(const Sender & sender, const std::vector<std::string> & params);
        std::string handleGetListDocumentsWithoutSecret(const Sender & sender, const std::vector<std::string> & params);
        
        std::string handleGetDocumentWithSecret(const Sender & sender, const std::vector<std::string> & params);
        std::string handleGetDocumentWithoutSecret(const Sender & sender, const std::vector<std::string> & params);

        std::string handleGetDocumentWithSecretByName(const Sender & sender, const std::vector<std::string> & params);
        std::string handleGetDocumentWithoutSecretByName(const Sender & sender, const std::vector<std::string> & params);
        
        std::string handleGetListPortfolio(const Sender & sender, const std::vector<std::string> & params);

        std::string handleGetConsumerUsages(const Sender & sender, const std::vector<std::string> & params);
        std::string handleGetProducerUsages(const Sender & sender, const std::vector<std::string> & params);
        
        std::string handleCreate(const Sender & sender, const std::vector<std::string> & params);
        std::string handleDelete(const Sender & sender, const std::vector<std::string> & params);
        std::string handleUpdate(const Sender & sender, const std::vector<std::string> & params);

        //Notification
        void sendNotificationOnCreate (const std::string & portfolio, const DocumentPtr newDocument);
        void sendNotificationOnDelete (const std::string & portfolio, const DocumentPtr oldDocument);
        void sendNotificationOnUpdate (const std::string & portfolio, const DocumentPtr oldDocument, const DocumentPtr newDocument);

        
        std::string serializeListDocumentsPrivate(const std::string & portfolioName, const std::set<UsageId> & usages);
        std::string serializeListDocumentsPublic(const std::string & portfolioName, const std::set<UsageId> & usages);

        //srr
        void handleSRRRequest(messagebus::Message msg);
        dto::srr::SaveResponse handleSave(const dto::srr::SaveQuery & query);
        dto::srr::RestoreResponse handleRestore(const dto::srr::RestoreQuery & query);
    
    public:
        //Command list
        static constexpr const char* GET_PORTFOLIO_LIST = "GET_PORTFOLIO_LIST";

        static constexpr const char* GET_CONSUMER_USAGES = "GET_CONSUMER_USAGES";
        static constexpr const char* GET_PRODUCER_USAGES = "GET_PRODUCER_USAGES";

        static constexpr const char* GET_LIST_WITH_SECRET = "GET_LIST_WITH_SECRET";
        
        static constexpr const char* GET_LIST_WITHOUT_SECRET = "GET_LIST_WITHOUT_SECRET";

        static constexpr const char* GET_WITHOUT_SECRET = "GET_WITHOUT_SECRET";
        static constexpr const char* GET_WITHOUT_SECRET_BY_NAME = "GET_WITHOUT_SECRET_BY_NAME";

        static constexpr const char* GET_WITH_SECRET = "GET_WITH_SECRET";
        static constexpr const char* GET_WITH_SECRET_BY_NAME = "GET_WITH_SECRET_BY_NAME";

        static constexpr const char* CREATE = "CREATE";
        static constexpr const char* DELETE = "DELETE";
        static constexpr const char* UPDATE = "UPDATE";

        //SRR
        std::unique_ptr<messagebus::MessageBus> m_msgBus;
        std::mutex m_lock;
        dto::srr::SrrQueryProcessor m_srrProcessor;       
 
    };

} // namespace secw

#endif
