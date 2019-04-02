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

/*
@header
    secw_portfolio - Portfolio to handle documents
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

SecurityWalletPotfolio::SecurityWalletPotfolio(std::string name):
    m_name(name)
{

}

SecurityWalletPotfolio SecurityWalletStorage::getPortfolio(std::string name)
{
    return m_portfolios[name];
}


DummyStorage::DummyStorage()
{

}
int DummyStorage::load(const char*path)
{
    m_path=path;
    log_info("Loading data from %s",m_path.c_str());
    SecurityWalletPotfolio _defaultPortfolio("default");
    DummyDocument _dummyDocument("e3500faa-54c8-11e9-af3e-0800277def58",
        "myFirstDoc",
        "\"secLevel\" : \"noAuthNoPriv\",\
         \"secName\" : \"Dummy\",\
         \"authProtocol\": \"MD5\",\
         \"privProtocol\" : \"DES\""
        ,
        "\"authPassword\" : \"<CRYPT>authentication password</CRYPT>\",\
         \"privPassword\" : \"<CRYPT>privacy pass phrase </CRYPT>\"");
    std::vector<Document> documents;
    //TODO => use ref    
    _defaultPortfolio.m_documents.emplace_back(_dummyDocument);
    m_portfolios["default"]=_defaultPortfolio;

    return m_portfolios.size();
}


