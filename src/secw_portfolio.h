/*  =========================================================================
    secw_portfolio - Portfolio to handle documents

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

#ifndef SECW_PORTFOLIO_H_INCLUDED
#define SECW_PORTFOLIO_H_INCLUDED

#include "secw_document.h"

/**
 * \brief portfolio wallet
 */
class SecurityWalletPotfolio
{
public:
    SecurityWalletPotfolio(){};
    SecurityWalletPotfolio(std::string name);
    std::vector<Document> m_documents;
protected:
    std::string m_name;
};

//Virtual Storage 
class SecurityWalletStorage
{
public:
    virtual int load(const char*path) = 0;
    virtual SecurityWalletPotfolio getPortfolio(std::string name);
protected:
    std::string m_path;
    std::map<std::string,SecurityWalletPotfolio> m_portfolios;
};

//Dummy Storage for dev purpouse 
class DummyStorage final : public SecurityWalletStorage
{
public:
    DummyStorage();
    int load(const char*path) override;

};




#endif
