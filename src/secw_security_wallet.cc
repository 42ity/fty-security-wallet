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

/*
@header
    secw_security_wallet - Security wallet to manage the storage and configuration
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
    SecurityWallet::SecurityWallet(const std::string & configurationPath, const std::string & databasePath):
        m_pathDatabase(databasePath)
    { 
        //Load Config and then Database
        
        //Load Config
        try
        {
            struct stat buffer;   
            bool fileExist =  (stat (configurationPath.c_str(), &buffer) == 0);
        
            if(!fileExist)
            {
                throw std::runtime_error("File does not exist!");
            }
            
            std::ifstream input;

            input.open(configurationPath);

            cxxtools::SerializationInfo rootSi;
            cxxtools::JsonDeserializer deserializer(input);
            deserializer.deserialize(rootSi);

            m_configuration = std::make_shared<SecwConfiguration>(rootSi);

        }
        catch(const std::exception& e)
        {
            log_error("Error while loading configuration file %s\n %s",configurationPath.c_str(), e.what());
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

    const SecwConfiguration & SecurityWallet::getConfiguration() const
    {
        return *(m_configuration);
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
       
        throw SecwUnknownPortfolioException(name);
    }
    
} //namepace secw