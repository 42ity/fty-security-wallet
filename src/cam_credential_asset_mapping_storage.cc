/*  =========================================================================
    cam_credential_asset_mapping_storage - Credential asset mapping class to manage the storage

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
#include "cam_credential_asset_mapping_storage.h"

#include <fty_log.h>

#include <fstream>
#include <iostream>
#include <cxxtools/jsonserializer.h>
#include <cxxtools/jsondeserializer.h>

#include <sys/stat.h>
#include <unistd.h>

namespace cam
{

    CredentialAssetMappingStorage::CredentialAssetMappingStorage(const std::string & databasePath):
    m_pathDatabase(databasePath)
    {
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
                    std::vector<CredentialAssetMapping> listOfmapping;
                    rootSi.getMember("mappings") >>= listOfmapping;
                    
                    for( const CredentialAssetMapping &  mapping : listOfmapping)
                    {
                        try
                        {
                            setMapping(mapping);
                        }
                        catch(const std::exception& e) //show must go on
                        {
                            log_error("Error on one element: %s",e.what());
                        }
                    }
                }
                else
                {
                    throw std::runtime_error("Version not supported");
                }
            }
            else
            {
                log_info(" No mapping %s. Creating default mapping...", m_pathDatabase.c_str());
            }

            /*CredentialAssetMapping mapping;
            mapping.m_usageId = "test-usage";
            mapping.m_assetId = "asset-1";
            mapping.m_credentialId = "cred-1";

            setMapping(mapping);*/
        }
        catch(const std::exception& e)
        {
            log_error("Error while loading mapping file %s\n %s",m_pathDatabase.c_str(), e.what());
            exit(EXIT_FAILURE);
        }

        //attempt to save the mapping and ensure that we can write
        try
        {
            save();
        }
        catch(const std::exception& e)
        {
            log_error("Error while saving into mapping file %s\n %s",m_pathDatabase.c_str(), e.what());
            exit(EXIT_FAILURE);
        }
    }

    void CredentialAssetMappingStorage::save() const
    {
        //create the file content
        cxxtools::SerializationInfo rootSi;

        rootSi.addMember("version") <<= MAPPING_VERSION;

        std::vector<CredentialAssetMapping> list;

        for( const auto item : m_mappings)
        {
            list.push_back(item.second);
        }

        rootSi.addMember("mappings") <<= list;

        //open the file
        std::ofstream output(m_pathDatabase.c_str());
        
        cxxtools::JsonSerializer serializer(output);
        serializer.beautify(true);
        serializer.serialize(rootSi);
    }
    
    const CredentialAssetMapping & CredentialAssetMappingStorage::getMapping(const AssetId & assetId, const UsageId & usageId) const
    {
        Hash hash = computeHash(assetId, usageId);

        try
        {
            return m_mappings.at(hash);
        }
        catch(const std::exception& e)
        {
            throw CamMappingDoNotExistException("Error: "+std::string(e.what()));
        }
    }

    void CredentialAssetMappingStorage::setMapping(const CredentialAssetMapping & mapping)
    {
        if( mapping.m_assetId.empty() || mapping.m_usageId.empty() || mapping.m_credentialId.empty() )
        {
            throw CamException("Bad format");
        }

        Hash hash = computeHash(mapping.m_assetId, mapping.m_usageId);
        m_mappings[hash] = mapping;
    }

    void CredentialAssetMappingStorage::removeMapping(const AssetId & assetId, const UsageId & usageId)
    {
        Hash hash = computeHash(assetId, usageId);

        size_t deleted = m_mappings.erase(hash);

        if(deleted == 0)
        {
            throw CamMappingDoNotExistException(hash);
        }

    }

    bool CredentialAssetMappingStorage::isMappingExisting(const AssetId & assetId, const UsageId & usageId) const
    {
        Hash hash = computeHash(assetId, usageId);
        
        bool isMappingExisting = true;
        try
        {
            m_mappings.at(hash);
        }
        catch(const std::exception&)
        {
            isMappingExisting = false;
        }

        return isMappingExisting;
    }

    std::vector<CredentialAssetMapping> CredentialAssetMappingStorage::getCredentialMappingsForUsage( const CredentialId & credentialId,
                                                                        const UsageId & usageId) const
    {
        std::vector<CredentialAssetMapping> list;

        for( const auto item : m_mappings)
        {
            if( (item.second.m_usageId == usageId) && (item.second.m_credentialId == credentialId) )
            {
                list.push_back(item.second);
            }
        }

        return  list;   
    }

    std::vector<CredentialAssetMapping> CredentialAssetMappingStorage::getCredentialMappings(const CredentialId & credentialId) const
    {
        std::vector<CredentialAssetMapping> list;

        for( const auto item : m_mappings)
        {
            if( item.second.m_credentialId == credentialId )
            {
                list.push_back(item.second);
            }
        }

        return list;  
    }

    std::vector<CredentialAssetMapping> CredentialAssetMappingStorage::getAssetMappings(const AssetId & assetId) const
    {
        std::vector<CredentialAssetMapping> list;

        for( const auto item : m_mappings)
        {
            if( item.second.m_assetId == assetId )
            {
                list.push_back(item.second);
            }
        }

        return  list; 
    }

    Hash CredentialAssetMappingStorage::computeHash(const AssetId & assetId, const UsageId & usageId)
    {
        return "A:"+assetId+"|U:"+usageId;
    }

} // namepsace cam 