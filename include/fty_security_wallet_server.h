/*  =========================================================================
    fty_security_wallet_server - secw broker agent

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

#ifndef FTY_SECURITY_WALLET_SERVER_H_INCLUDED
#define FTY_SECURITY_WALLET_SERVER_H_INCLUDED

#include "fty_common_mlm_agent.h"

#include <functional>

#include <cxxtools/serializationinfo.h>
/**
 * \brief Agent SecurityWalletServer main server actor
 */
namespace secw
{
    using Command   = std::string;
    using Sender    = std::string;
    using Subject   = std::string;
    
    using FctCommandHandler = std::function<std::string (const Sender &, const std::vector<std::string> &)>;

    class SecurityWallet;

    class SecurityWalletServer final : public mlm::MlmAgent
    {

    public:
        explicit SecurityWalletServer(zsock_t *pipe);

    private:
        bool handleMailbox(zmsg_t *message) override;
        bool handlePipe(zmsg_t *message) override;
        
        // List of supported commands with a reference to the handler for this command.
        std::map<Command, FctCommandHandler> m_supportedCommands;

        static std::shared_ptr<SecurityWallet> m_activeWallet;
        
        //Handler for all supported commands
        static std::string handleGetListDocumentsWithSecret(const Sender & sender, const std::vector<std::string> & params);
        static std::string handleGetListDocumentsWithoutSecret(const Sender & sender, const std::vector<std::string> & params);
        
        static std::string handleGetDocumentWithSecret(const Sender & sender, const std::vector<std::string> & params);
        static std::string handleGetDocumentWithoutSecret(const Sender & sender, const std::vector<std::string> & params);
        
        static std::string handleGetListReadableTags(const Sender & sender, const std::vector<std::string> & params);
        static std::string handleGetListEditableTags(const Sender & sender, const std::vector<std::string> & params);
        
        static std::string handleGetListPortfolio(const Sender & sender, const std::vector<std::string> & params);
        
        static std::string handleNotImplementedCmd(const Sender & sender, const std::vector<std::string> & params);
        
        //Helpers
        static zmsg_t *generateErrorMsg(const std::string & correlationId, const std::string & errPayload);
        static std::string toJsonFromSerializationInfo(const cxxtools::SerializationInfo & si);
        
        static std::string serializeListDocumentsPublic(const std::string & portfolioName, const std::vector<DocumentType> & types, const std::vector<Tag> & tags = {});
        static std::string serializeListDocumentsPrivate(const std::string & portfolioName, const std::vector<DocumentType> & types, const std::vector<Tag> & tags);
    
    public:
        //Command list
        static constexpr const char* GET_PORTFOLIO_LIST = "GET_PORTFOLIO_LIST";
        static constexpr const char* GET_LIST_WITH_SECRET = "GET_LIST_WITH_SECRET";
        static constexpr const char* GET_LIST_WITHOUT_SECRET = "GET_LIST_WITHOUT_SECRET";
        static constexpr const char* GET_READABLE_TAGS = "GET_READABLE_TAGS";
        static constexpr const char* GET_EDITABLE_TAGS = "GET_EDITABLE_TAGS";
        static constexpr const char* GET_WITHOUT_SECRET = "GET_WITHOUT_SECRET";
        static constexpr const char* GET_WITH_SECRET = "GET_WITH_SECRET";
        static constexpr const char* CREATE = "CREATE";
        static constexpr const char* DELETE = "DELETE";
        static constexpr const char* UPDATE = "UPDATE";        
 
    };



} // namespace secw

//  @interface
//  Create an fty_security_wallet_server actor
void fty_security_wallet_server(zsock_t *pipe, void *endpoint);

void fty_security_wallet_server_test (bool verbose);

#endif
