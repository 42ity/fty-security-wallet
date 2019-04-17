/*  =========================================================================
    secw_security_wallet - Security wallet to manage the storage and access

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
    secw_security_wallet - Security wallet to manage the storage and access
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

#include <fstream>
#include <iostream>
#include <cxxtools/jsonserializer.h>
#include <cxxtools/jsondeserializer.h>

#include <sys/stat.h>
#include <unistd.h>

namespace secw
{
/*-----------------------------------------------------------------------------*/
/*   SecurityWallet                                                            */
/*-----------------------------------------------------------------------------*/
//Public
    SecurityWallet::SecurityWallet(const std::string & accessPath, const std::string & databasePath):
        m_pathDatabase(databasePath),
        m_pathAccess(accessPath)
    { 
        //Load Access and then Database
        
        //Load Access
        try
        {
            struct stat buffer;   
            bool fileExist =  (stat (m_pathAccess.c_str(), &buffer) == 0);
        
            if(!fileExist)
            {
                throw std::runtime_error("File does not exist!");
            }
            
            std::ifstream input;

            input.open(m_pathAccess);

            cxxtools::SerializationInfo rootSi;
            cxxtools::JsonDeserializer deserializer(input);
            deserializer.deserialize(rootSi);

            std::vector<TagAccess> tagList;

            rootSi.getMember("tags") >>= tagList;
            
            for(const TagAccess & tagAccess : tagList)
            {
                m_mapTagAccess[tagAccess.getId()] = tagAccess;
            }
            
            log_info("%i tags found", m_mapTagAccess.size());
        }
        catch(const std::exception& e)
        {
            log_error("Error while loading Access file %s\n %s",m_pathAccess.c_str(), e.what());
            exit(EXIT_FAILURE);
        }
        
        
        //Load database
        try
        {
            struct stat buffer;   
            bool fileExist =  (stat (m_pathDatabase.c_str(), &buffer) == 0);
        
            //try to open portfolio if exist
            if(fileExist)
            {
                std::ifstream input;

                input.open(m_pathDatabase);
                
                cxxtools::SerializationInfo rootSi;
                cxxtools::JsonDeserializer deserializer(input);
                deserializer.deserialize(rootSi);

                uint8_t version = 0;
                rootSi.getMember("version") >>= version;

                if(version == 1)
                {
                    rootSi.getMember("portfolios") >>= m_portfolios;
                }
            }
            else
            {
                log_info(" No database %s. Creating default database...", m_pathDatabase.c_str());
                m_portfolios.emplace_back(Portfolio("default"));
            }
        }
        catch(const std::exception& e)
        {
            log_error("Error while loading database file %s\n %s",m_pathDatabase.c_str(), e.what());
            exit(EXIT_FAILURE);
        }

        //attempt to save the database and ensure that we can write
        try
        {
            save();
        }
        catch(const std::exception& e)
        {
            log_error("Error while saving into database file %s\n %s",m_pathDatabase.c_str(), e.what());
            exit(EXIT_FAILURE);
        }
        
    }

    void SecurityWallet::save() const
    {
        //create the file content
        cxxtools::SerializationInfo rootSi;

        rootSi.addMember("version") <<= SECW_VERSION;
        rootSi.addMember("portfolios") <<= m_portfolios;

        //open the file
        std::ofstream output(m_pathDatabase.c_str());
        
        cxxtools::JsonSerializer serializer(output);
        serializer.beautify(true);
        serializer.serialize(rootSi);
    }

    std::vector<std::string> SecurityWallet::getPortfolioNames() const
    {
        std::vector<std::string> list;

        for( const Portfolio & portfolio : m_portfolios)
        {
            list.push_back(portfolio.getName());
        }

        return list;
    }

    Portfolio & SecurityWallet::getPortfolio(const std::string & name)
    {
        for( Portfolio & portfolio : m_portfolios)
        {
            if(portfolio.getName() == name)
            {
                return portfolio;
            }
        }
       
        throw SecwUnknownPortfolioException(" Portfolio "+name+" does not exist");
    }
    
    std::vector<Tag> SecurityWallet::getAvailableTags() const
    {
        std::vector<Tag> tags;
        
        for(const auto & item : m_mapTagAccess)
        {
            tags.push_back(item.first);
        }
        
        return tags;
    }

    std::vector<Tag> SecurityWallet::getAccessibleTags(const std::string & client, AccessMethods method) const
    {
        std::vector<Tag> setOfTags;

        for(const auto & item : m_mapTagAccess)
        {
            log_info("Check the tag %s", item.first.c_str());
            if(item.second.checkAccess(client,method))
            {
                setOfTags.push_back(item.first);
                log_info("  >> Add the tag %s", item.first.c_str());
            }
        }

        return setOfTags;
    }
    
    bool SecurityWallet::checkTagAccess(const Tag & tag, const std::string & client, AccessMethods method) const
    {
        if(m_mapTagAccess.count(tag) < 1)
        {
            throw SecwUnknownTagException("Tag "+tag+" is unknown");
        }
        
        return m_mapTagAccess.at(tag).checkAccess(client, method);
    }

} //namepace secw