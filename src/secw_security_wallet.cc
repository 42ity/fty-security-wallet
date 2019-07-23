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

            //it's an array
            for(size_t index = 0; index < rootSi.memberCount(); index++)
            {
                const cxxtools::SerializationInfo & portfolioConfigSi  = rootSi.getMember(index);
                PortfolioConfiguration config(portfolioConfigSi);

                if(m_configurations.count(config.getPortfolioName()) > 0)
                {
                    throw std::runtime_error("Portfolio "+config.getPortfolioName()+" already exist in the configuration.");
                }

                m_configurations.emplace(config.getPortfolioName(), config);
            }

            if(m_configurations.empty())
            {
                throw std::runtime_error("No Portfolio in the configuration.");
            }

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
            }

            for(const auto & item : m_configurations)
            {
                //check that we have the portfolio in the list
                bool found = false;
                for(const auto & portfolio : m_portfolios )
                {
                    if(item.first == portfolio.getName())
                    {
                        found = true;
                        break;
                    }
                }

                //if it not exist we add it.
                if(!found)
                {
                    m_portfolios.emplace_back(Portfolio(item.first));
                }
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

        for( const auto & item : m_configurations)
        {
            list.push_back(item.first);
        }

        return list;
    }

    const PortfolioConfiguration & SecurityWallet::getConfiguration(const std::string & portfolioName) const
    {
        if(m_configurations.count(portfolioName) != 1)
        {
            throw SecwUnknownPortfolioException(portfolioName);
        }

        return m_configurations.at(portfolioName);
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
