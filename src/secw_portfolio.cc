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

namespace secw
{

/*----------------------------------------------------------------------*/
/*   Portfolio                                                          */
/*----------------------------------------------------------------------*/
//Public
    Portfolio::Portfolio(const std::string & name):
        m_name(name)
    {}

    Id Portfolio::add(const DocumentPtr & doc)
    {

        //Check to ensure that the name do not exist => name unique by portfolio
        if(m_mapNameDocuments.count(doc->getName()) > 0)
        {
            throw SecwNameAlreadyExistsException(doc->getName());
        }

        //create an id
        Id id;
        do
        {
            ZuuidGuard zuuid(zuuid_new ());
            id = std::string( zuuid_str_canonical(zuuid.get()) );
        }
        while(m_documents.count(id) != 0); //the id already exist, so we get a new one

        //make a copy using factory
        DocumentPtr copyDoc = doc->clone();

        copyDoc->m_id = id;
        
        m_documents[id] = copyDoc;
        m_mapNameDocuments[copyDoc->getName()] = copyDoc;

        return id;
    }

    void Portfolio::remove(const Id & id)
    {
        //Check if document exist
        if(m_documents.count(id) < 1)
        {
            throw SecwDocumentDoNotExistException(id);
        }

        m_mapNameDocuments.erase(m_documents[id]->getName());
        m_documents.erase(id);
    }
    
    void Portfolio::update(const DocumentPtr & doc)
    {
        Id id = doc->getId();

        //Check if document exist
        if(m_documents.count(id) < 1)
        {
            throw SecwDocumentDoNotExistException(id);
        }

        DocumentPtr oldDoc = m_documents[id];

        //ensure that if the name is modified, the new name do not already exist
        if(doc->getName() != oldDoc->getName())
        {
            if(m_mapNameDocuments.count(doc->getName()) > 0)
            {
                throw SecwNameAlreadyExistsException(doc->getName());
            }

            m_mapNameDocuments.erase(oldDoc->getName());


        }

        DocumentPtr copyDoc =  doc->clone();

        m_documents[id] = copyDoc;
        m_mapNameDocuments[copyDoc->getName()] = copyDoc;
    }
    
    
    DocumentPtr Portfolio::getDocument(const Id & id) const
    {
        if(m_documents.count(id) < 1)
        {
            throw SecwDocumentDoNotExistException(id);
        }
            
        return m_documents.at(id)->clone();
    }

    DocumentPtr Portfolio::getDocumentByName(const std::string & name) const
    {
        if(m_mapNameDocuments.count(name) < 1)
        {
            throw SecwNameDoesNotExistException(name);
        }
            
        return m_mapNameDocuments.at(name)->clone();
    }

    std::vector<DocumentPtr> Portfolio::getListDocuments() const
    {
        std::vector<DocumentPtr> returnList;

        for( const auto & item : m_documents)
        {
            returnList.push_back(item.second);
        }

        return returnList;
    }

    void Portfolio::loadPortfolio(const cxxtools::SerializationInfo& si)
    {
        uint8_t version = 0;

        try
        {
            si.getMember("version") >>= version;
        }
        catch(const std::exception& e)
        {
            throw SecwImpossibleToLoadPortfolioException("Bad format of the serialization data");
        }

        //remove former content
        m_documents.clear();
        m_mapNameDocuments.clear();

        switch(version)
        {
            case 1: loadPortfolioVersion1(si);
                    break;
            default: throw SecwImpossibleToLoadPortfolioException("Version "+std::to_string(version)+" not supported");
        }
        
    }

    void Portfolio::serializePortfolio(cxxtools::SerializationInfo& si) const
    {
        si.addMember("version") <<= PORTFOLIO_VERSION;
        si.addMember("name") <<= m_name;
        si.addMember("documents") <<= getListDocuments();
    }

    void Portfolio::loadPortfolioVersion1(const cxxtools::SerializationInfo& si)
    {
        try
        {
            si.getMember("name") >>= m_name;
            const cxxtools::SerializationInfo& documents = si.getMember("documents");
            
            size_t count = 0;
            
            for(size_t index = 0; index < documents.memberCount(); index++ )
            {
                try
                {
                    DocumentPtr doc;
                    documents.getMember(index) >>= doc;
                    
                    doc->validate();

                    m_documents[doc->getId()] = doc;
                    m_mapNameDocuments[doc->getName()] = doc;

                    count++;
                }
                catch(const std::exception& e)
                {
                    log_error("Impossible to load a document from portfolio %s: %s", m_name.c_str(), e.what());
                }
            }
            
            log_debug("Portfolio %s loaded with %i documents",m_name.c_str(), count );

        }
        catch(const std::exception& e)
        {
            throw SecwImpossibleToLoadPortfolioException("Bad format of the serialization data in portfolio " + m_name);
        }
    }

    void operator<<= (cxxtools::SerializationInfo& si, const Portfolio & portfolio)
    {
        portfolio.serializePortfolio(si);
    }

    void operator>>= (const cxxtools::SerializationInfo& si, Portfolio & portfolio)
    {
        portfolio.loadPortfolio(si);
    }

} //namepace secw


