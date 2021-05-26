/*  =========================================================================
  secw_consumer_accessor - Accessor to return documents from the agent

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
  secw_consumer_accessor - Accessor to return documents from the agent
@discuss
@end
*/

#include "secw_consumer_accessor.h"
#include "secw_client_accessor.h"
#include "secw_helpers.h"
#include "secw_security_wallet_server.h"
#include <fty_common_mlm_stream_client.h>
#include <fty_common_socket_sync_client.h>
#include <cxxtools/serializationinfo.h>

namespace secw {
ConsumerAccessor::ConsumerAccessor(fty::SocketSyncClient& requestClient)
{
    m_clientAccessor = std::make_shared<ClientAccessor>(requestClient);
}

ConsumerAccessor::ConsumerAccessor(fty::SocketSyncClient& requestClient, mlm::MlmStreamClient& subscriberClient)
{
    m_clientAccessor = std::make_shared<ClientAccessor>(requestClient, subscriberClient);
}

std::vector<std::string> ConsumerAccessor::getPortfolioList() const
{
    std::vector<std::string> frames = m_clientAccessor->sendCommand(SecurityWalletServer::GET_PORTFOLIO_LIST, {});

    if (frames.size() < 1) {
        throw SecwProtocolErrorException("Empty answer from server");
    }

    cxxtools::SerializationInfo si = deserialize(frames.at(0));

    std::vector<std::string> portfolioNames;

    si >>= portfolioNames;

    return portfolioNames;
}

std::set<UsageId> ConsumerAccessor::getConsumerUsages(const std::string& portfolioName) const
{
    std::set<UsageId> usages;

    std::vector<std::string> frames =
        m_clientAccessor->sendCommand(SecurityWalletServer::GET_CONSUMER_USAGES, {portfolioName});

    if (frames.size() < 1) {
        throw SecwProtocolErrorException("Empty answer from server");
    }

    cxxtools::SerializationInfo si = deserialize(frames.at(0));

    si >>= usages;

    return usages;
}

std::vector<DocumentPtr> ConsumerAccessor::getListDocumentsWithPrivateData(
    const std::string& portfolio, const UsageId& usageId) const
{
    std::vector<std::string> frames =
        m_clientAccessor->sendCommand(SecurityWalletServer::GET_LIST_WITH_SECRET, {portfolio, usageId});

    // the first frame should contain the data
    if (frames.size() < 1) {
        throw SecwProtocolErrorException("Empty answer from server");
    }

    cxxtools::SerializationInfo si = deserialize(frames.at(0));

    std::vector<DocumentPtr> documents;

    si >>= documents;

    return documents;
}

std::vector<DocumentPtr> ConsumerAccessor::getListDocumentsWithPrivateData(
    const std::string& portfolio, const std::vector<Id>& ids) const
{
    std::vector<DocumentPtr> docs;

    for (const Id& id : ids) {
        try {
            docs.push_back(getDocumentWithPrivateData(portfolio, id));
        } catch (const SecwException& e) {
            // filter exceptions => ID not found and noit access to this id
            if ((e.getErrorCode() != DOCUMENT_DO_NOT_EXIST) && (e.getErrorCode() != ILLEGAL_ACCESS)) {
                throw; // throw
            }
        }
    }

    return docs;
}

DocumentPtr ConsumerAccessor::getDocumentWithPrivateData(const std::string& portfolio, const Id& id) const
{
    std::vector<std::string> frames =
        m_clientAccessor->sendCommand(SecurityWalletServer::GET_WITH_SECRET, {portfolio, id});

    // the first frame should contain the data
    if (frames.size() < 1) {
        throw SecwProtocolErrorException("Empty answer from server");
    }

    cxxtools::SerializationInfo si = deserialize(frames.at(0));

    DocumentPtr document;

    si >>= document;

    return document;
}

DocumentPtr ConsumerAccessor::getDocumentWithPrivateDataByName(
    const std::string& portfolio, const std::string& name) const
{
    std::vector<std::string> frames =
        m_clientAccessor->sendCommand(SecurityWalletServer::GET_WITH_SECRET_BY_NAME, {portfolio, name});

    // the first frame should contain the data
    if (frames.size() < 1) {
        throw SecwProtocolErrorException("Empty answer from server");
    }

    cxxtools::SerializationInfo si = deserialize(frames.at(0));

    DocumentPtr document;

    si >>= document;

    return document;
}

void ConsumerAccessor::setCallbackOnUpdate(UpdatedCallback updatedCallback)
{
    m_clientAccessor->setCallbackOnUpdate(updatedCallback);
}

void ConsumerAccessor::setCallbackOnCreate(CreatedCallback createdCallback)
{
    m_clientAccessor->setCallbackOnCreate(createdCallback);
}

void ConsumerAccessor::setCallbackOnDelete(DeletedCallback deletedCallback)
{
    m_clientAccessor->setCallbackOnDelete(deletedCallback);
}

void ConsumerAccessor::setCallbackOnStart(StartedCallback startedCallback)
{
    m_clientAccessor->setCallbackOnStart(startedCallback);
}

} // namespace secw
