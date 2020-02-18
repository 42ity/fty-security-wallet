/*  =========================================================================
    secw_security_wallet_database_proxy - Security Wallet database proxy

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
    secw_security_wallet_database_proxy - Security Wallet database proxy
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

#include <algorithm>
#include <map>
#include <vector>

namespace secw
{
    struct ProxyDocument
    {
        secw::Id id;
        secw::DocumentType type;
    };

    using ProxyDocuments = std::map<secw::Id, ProxyDocument>;

    bool operator<(const ProxyDocuments::value_type& a, const ProxyDocuments::value_type& b)
    {
        return a.first < b.first;
    }

    SecurityWalletDatabaseProxy::SecurityWalletDatabaseProxy(const std::string& dbUrl) :
        m_conn(tntdb::connectCached(dbUrl))
    {
    }

    void SecurityWalletDatabaseProxy::sync(SecurityWallet& wallet)
    {
        ProxyDocuments walletDocuments;
        ProxyDocuments proxyDocuments;

        // Grab all documents from wallet.
        for (auto& portfolioName : wallet.getPortfolioNames()) {
            auto& portfolio = wallet.getPortfolio(portfolioName);

            for (auto& document : portfolio.getListDocuments()) {
                ProxyDocument proxyDocument;
                proxyDocument.id = document->getId();
                proxyDocument.type = document->getType();
                walletDocuments.emplace(document->getId(), proxyDocument);
            } 
        }

        // Grab all proxy objects from database.
        tntdb::Statement proxySt = m_conn.prepare("select BIN_TO_UUID(id_secw_document), id_secw_document_type from t_bios_secw_document");
        for (const tntdb::Row& row : proxySt) {
            ProxyDocument proxyDocument;
            row[0].get(proxyDocument.id);
            row[1].get(proxyDocument.type);
            proxyDocuments.emplace(proxyDocument.id, proxyDocument);
        }

        // Compute operations to perform on proxy documents.
        std::vector<ProxyDocuments::value_type> proxyDocumentsToCreate;
        std::vector<ProxyDocuments::value_type> proxyDocumentsToDelete;

        std::set_difference(
            walletDocuments.begin(),
            walletDocuments.end(),
            proxyDocuments.begin(),
            proxyDocuments.end(),
            std::back_inserter(proxyDocumentsToCreate)
        );

        std::set_difference(
            proxyDocuments.begin(),
            proxyDocuments.end(),
            walletDocuments.begin(),
            walletDocuments.end(),
            std::back_inserter(proxyDocumentsToDelete)
        );

        // Update proxy documents in database.
        for (const auto& proxyDocument : proxyDocumentsToDelete) {
            log_trace("Removing proxy document %s from database.", proxyDocument.first.c_str());

            try {
                tntdb::Statement st = m_conn.prepare("delete from t_bios_secw_document where id_secw_document = UUID_TO_BIN(:uuid)");
                st.set("uuid", proxyDocument.first).execute();
            }
            catch (std::exception& e) {
                log_error("Couldn't update proxy document in database: %s.", e.what());
            }
        }

        for (const auto& walletDocument : proxyDocumentsToCreate) {
            log_trace("Adding proxy document %s to database.", walletDocument.first.c_str());

            try {
                tntdb::Statement st = m_conn.prepare("insert into t_bios_secw_document values (UUID_TO_BIN(:uuid), :type)");
                st.set("uuid", walletDocument.first)
                    .set("type", walletDocument.second.type)
                    .execute();
            }
            catch (std::exception& e) {
                log_error("Couldn't update proxy document in database: %s.", e.what());
            }
        }
    }
}
