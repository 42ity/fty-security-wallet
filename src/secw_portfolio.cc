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

    void Portfolio::add(const Document & doc)
    {
        //make a copy using factory
        DocumentPtr copyDoc = doc.clone();

        m_documents[copyDoc->getId()] = copyDoc;
        m_mapTypesToIdDocs[copyDoc->getType()].insert(copyDoc->getId());
    }

    void Portfolio::remove(const Id & id)
    {
        DocumentPtr doc = m_documents.at(id);

        //Remove from index for types
        m_mapTypesToIdDocs[doc->getType()].erase(id);
    }
    
    
    DocumentPtr Portfolio::getDocument(const Id & id) const
    {
        if(m_documents.count(id) < 1)
        {
            throw SecwDocumentDoNotExistException("Document with id "+ id +" do not exist");
        }
            
        return m_documents.at(id)->clone();
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

        switch(version)
        {
            case 1: loadPortfolioVersion1(si);
                    break;
            default: throw SecwImpossibleToLoadPortfolioException("Version "+std::to_string(version)+" not supported");
        }

        //Create the index for type => map of type pointing to set of Id of Document
        for(auto & item : m_documents)
        {
            const DocumentPtr & doc = item.second;
            m_mapTypesToIdDocs[doc->getType()].insert(doc->getId());
            log_debug(" Add document %s with type %s", doc->getId().c_str(), doc->getType().c_str() );
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
                    
                    if(doc->isContainingPrivateData())
                    {
                        m_documents[doc->getId()] = doc;
                        count++;
                    }
                    else
                    {
                        log_error("Impossible to load the document with the id %s from portfolio because private part is missing", doc->getId().c_str());
                    }

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


