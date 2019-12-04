/*  =========================================================================
    secw_security_wallet - Security wallet to manage the storage and configuration

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

#ifndef SECW_SECURITY_WALLET_H_INCLUDED
#define SECW_SECURITY_WALLET_H_INCLUDED

#include "secw_document.h"
#include "secw_portfolio.h"
#include "secw_configuration.h"

#include <cxxtools/serializationinfo.h>

#include <memory>

namespace secw
{
    class SecurityWallet
    {
    public:
        explicit SecurityWallet(const std::string & configurationPath, const std::string & databasePath);
        void save() const;
        void reload();
        Portfolio & getPortfolio(const std::string & name);
        std::vector<std::string> getPortfolioNames() const;

        const PortfolioConfiguration & getConfiguration(const std::string & portfolioName = "default") const;

        cxxtools::SerializationInfo getSrrSaveData(const std::string & passphrase);
        void restoreSRRData(const cxxtools::SerializationInfo & si, const std::string & passphrase, const std::string & version);

        static constexpr const uint8_t SECW_VERSION = 1;

    private:
        std::string m_pathConfiguration;
        std::string m_pathDatabase;
        
        std::map<std::string, PortfolioConfiguration> m_configurations;
        std::vector<Portfolio> m_portfolios;
    };

} // namepsace secw 

#endif
