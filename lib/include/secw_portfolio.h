/*  =========================================================================
    secw_portfolio - Portfolio to handle documents

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

#include "secw_document.h"
#include <memory>

/// portfolio wallet
namespace secw {

/// @brief Class to represent a portfolio of documents
///
/// This class contain the interface description use for action in the portfolio.
/// Portfolio is keeped in memory and it is the responsability of the owner to
/// save it permanently if needed.
class Portfolio
{
public:
    explicit Portfolio(const std::string& name = "default");

    const std::string& getName() const
    {
        return m_name;
    }

    Id   add(const DocumentPtr& doc);
    void remove(const Id& id);
    void update(const DocumentPtr& doc);

    DocumentPtr getDocument(const Id& id) const;
    DocumentPtr getDocumentByName(const std::string& name) const;

    std::vector<DocumentPtr> getListDocuments() const;

    void loadPortfolio(const cxxtools::SerializationInfo& si);
    void serializePortfolio(cxxtools::SerializationInfo& si) const;

    void loadPortfolioFromSRR(
        const cxxtools::SerializationInfo& si, const std::string& encryptiondKey, bool isSameInstance = false);
    void serializePortfolioSRR(cxxtools::SerializationInfo& si, const std::string& encryptiondKey) const;

    static constexpr const uint8_t PORTFOLIO_VERSION = 1;

private:
    std::string m_name;

    // Map containing all the document of the portfolio
    std::map<Id, DocumentPtr>          m_documents;
    std::map<std::string, DocumentPtr> m_mapNameDocuments;

    void loadPortfolioVersion1(const cxxtools::SerializationInfo& si);
    void loadPortfolioSRRVersion1(
        const cxxtools::SerializationInfo& si, const std::string& encryptiondKey, bool isSameInstance);
};

void operator<<=(cxxtools::SerializationInfo& si, const Portfolio& portfolio);
void operator>>=(const cxxtools::SerializationInfo& si, Portfolio& portfolio);

} // namespace secw
