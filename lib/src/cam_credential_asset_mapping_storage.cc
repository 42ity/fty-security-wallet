/*  =========================================================================
    cam_credential_asset_mapping_storage - Credential asset mapping class to manage the storage

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

#include "cam_credential_asset_mapping_storage.h"
#include "cam_helpers.h"
#include <cxxtools/jsondeserializer.h>
#include <cxxtools/jsonserializer.h>
#include <fstream>
#include <fty_log.h>
#include <fty_common_json.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

namespace cam {

CredentialAssetMappingStorage::CredentialAssetMappingStorage(const std::string& databasePath)
    : m_pathDatabase(databasePath)
{
    // Load database
    try {
        read();
    } catch (const std::exception& e) {
        log_error("Error while reading mapping file %s\n %s", m_pathDatabase.c_str(), e.what());
        exit(EXIT_FAILURE);
    }

    // attempt to save the mapping and ensure that we can write
    try {
        save();
    } catch (const std::exception& e) {
        log_error("Error while saving into mapping file %s\n %s", m_pathDatabase.c_str(), e.what());
        exit(EXIT_FAILURE);
    }
}

void CredentialAssetMappingStorage::serialize(cxxtools::SerializationInfo& si) const
{
    si.clear();
    si.addMember("version") <<= MAPPING_VERSION;
    si.addMember("mappings") <<= getAllMappings();
}

void CredentialAssetMappingStorage::parse(const cxxtools::SerializationInfo& si)
{
    auto savedMappings = m_mappings;

    try {
        m_mappings.clear();

        uint8_t version = 0;
        si.getMember("version") >>= version;

        if (version == 1) {
            std::vector<CredentialAssetMapping> listOfmapping;
            si.getMember("mappings") >>= listOfmapping;

            for (const CredentialAssetMapping& mapping : listOfmapping) {
                try {
                    setMapping(mapping);
                } catch (const std::exception& e) {
                    log_error("Error on one element: %s", e.what());
                }
            }
        } else {
            throw std::runtime_error("Version not supported");
        }
    } catch (...) {
        m_mappings = savedMappings; // restored
        throw;
    }
}

void CredentialAssetMappingStorage::save() const
{
    log_debug("Save mapping database %s", m_pathDatabase.c_str());

    cxxtools::SerializationInfo si;
    serialize(si);
    JSON::writeToFile(m_pathDatabase, si, true);
}

void CredentialAssetMappingStorage::read()
{
    log_debug("Read mapping database %s", m_pathDatabase.c_str());

    struct stat buffer;
    bool fileExist = (stat(m_pathDatabase.c_str(), &buffer) == 0);

    // open/read database if exist
    if (fileExist) {
        cxxtools::SerializationInfo si;
        JSON::readFromFile(m_pathDatabase, si);
        parse(si);
    } else {
        log_info("No mapping database %s. Creating default...", m_pathDatabase.c_str());
        m_mappings.clear();
    }
}

const CredentialAssetMapping& CredentialAssetMappingStorage::getMapping(
    const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol) const
{
    Hash hash = computeHash(assetId, serviceId, protocol);

    try {
        return m_mappings.at(hash);
    } catch (const std::exception& e) {
        log_debug("getMapping exception: %s", e.what());
        throw CamMappingDoesNotExistException(assetId, serviceId, protocol);
    }
}

void CredentialAssetMappingStorage::setMapping(const CredentialAssetMapping& mapping)
{
    if (mapping.m_assetId.empty() || mapping.m_serviceId.empty() || mapping.m_protocol.empty()) {
        throw CamException("Bad format");
    }

    Hash hash        = computeHash(mapping.m_assetId, mapping.m_serviceId, mapping.m_protocol);
    m_mappings[hash] = mapping;
}

void CredentialAssetMappingStorage::removeMapping(
    const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol)
{
    Hash hash = computeHash(assetId, serviceId, protocol);

    size_t deleted = m_mappings.erase(hash);

    if (deleted == 0) {
        throw CamMappingDoesNotExistException(assetId, serviceId, protocol);
    }
}

bool CredentialAssetMappingStorage::isMappingExisting(
    const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol) const
{
    Hash hash = computeHash(assetId, serviceId, protocol);
    return m_mappings.count(hash);
}

std::vector<CredentialAssetMapping> CredentialAssetMappingStorage::getCredentialMappingsForService(
    const CredentialId& credentialId, const ServiceId& serviceId) const
{
    std::vector<CredentialAssetMapping> list;

    for (const auto& item : m_mappings) {
        if ((item.second.m_serviceId == serviceId) && (item.second.m_credentialId == credentialId)) {
            list.push_back(item.second);
        }
    }

    return list;
}

std::vector<CredentialAssetMapping> CredentialAssetMappingStorage::getCredentialMappings(
    const CredentialId& credentialId) const
{
    std::vector<CredentialAssetMapping> list;

    for (const auto& item : m_mappings) {
        if (item.second.m_credentialId == credentialId) {
            list.push_back(item.second);
        }
    }

    return list;
}

std::vector<CredentialAssetMapping> CredentialAssetMappingStorage::getMappings(
    const AssetId& assetId, const ServiceId& serviceId) const
{
    std::vector<CredentialAssetMapping> list;

    for (const auto& item : m_mappings) {
        if ((item.second.m_assetId == assetId) && (item.second.m_serviceId == serviceId)) {
            list.push_back(item.second);
        }
    }

    return list;
}

std::vector<CredentialAssetMapping> CredentialAssetMappingStorage::getAllMappings() const
{
    std::vector<CredentialAssetMapping> list;

    for (const auto& item : m_mappings) {
        list.push_back(item.second);
    }

    return list;
}

std::vector<CredentialAssetMapping> CredentialAssetMappingStorage::getAssetMappings(const AssetId& assetId) const
{
    std::vector<CredentialAssetMapping> list;

    for (const auto& item : m_mappings) {
        if (item.second.m_assetId == assetId) {
            list.push_back(item.second);
        }
    }

    return list;
}

Hash CredentialAssetMappingStorage::computeHash(
    const AssetId& assetId, const ServiceId& serviceId, const Protocol& protocol)
{
    return "A" + assetId + "|S" + serviceId + "|P:" + protocol;
}

} // namespace cam
