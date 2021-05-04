/*  =========================================================================
    cam_accessor - Accessor to Credential asset mapping

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

#include "fty_security_wallet_classes.h"

#include "cam_accessor.h"
#include "cam_helpers.h"
#include "cam_credential_asset_mapping_server.h"

namespace cam
{
  Accessor::Accessor( mlm::MlmSyncClient & requestClient)
    : m_mlmClient(), m_requestClient(requestClient)
  {
  }

  Accessor::Accessor(   const ClientId & clientId,
                        uint32_t timeout,
                        const std::string & endPoint)
    : m_mlmClient(std::make_shared<mlm::MlmSyncClient>
                        (   clientId,
                            MAPPING_AGENT,
                            timeout,
                            endPoint
                        )),
     m_requestClient(*m_mlmClient)
  {

  }

  Accessor::~Accessor()
  {}

  void Accessor::createMapping( const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol,
                      const Port & port, const CredentialId & credentialId, Status status,
                      const MapExtendedInfo & extendedInfo)
  {
    CredentialAssetMapping mapping;

    mapping.m_assetId = assetId;
    mapping.m_serviceId = serviceId;
    mapping.m_protocol = protocol;
    mapping.m_port = port;
    mapping.m_credentialId = credentialId;
    mapping.m_status = status;
    mapping.m_extendedInfo = extendedInfo;

    createMapping(mapping);
  }

  void Accessor::createMapping( const CredentialAssetMapping & mapping)
  {
    cxxtools::SerializationInfo si;
    si <<= mapping;
    sendCommand(CredentialAssetMappingServer::CREATE_MAPPING, {serialize(si)} );
  }

  void Accessor::updateMapping( const CredentialAssetMapping & mapping)
  {
    cxxtools::SerializationInfo si;
    si <<= mapping;
    sendCommand(CredentialAssetMappingServer::UPDATE_MAPPING, {serialize(si)} );
  }

  const CredentialAssetMapping Accessor::getMapping(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol) const
  {
    CredentialAssetMapping mapping;

    std::vector<std::string> payload;

    payload = sendCommand(CredentialAssetMappingServer::GET_MAPPING, {assetId, serviceId, protocol} );

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= mapping;

    return mapping;
  }

  void Accessor::removeMapping(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol)
  {
    sendCommand(CredentialAssetMappingServer::REMOVE_MAPPING, {assetId, serviceId, protocol} );
  }

  /*bool isMappingExisting(const AssetId & assetId, const ServiceId & serviceId) const;*/

  void Accessor::updateCredentialId(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol, const CredentialId & credentialId)
  {
    CredentialAssetMapping mapping;
    mapping.m_assetId = assetId;
    mapping.m_serviceId = serviceId;
    mapping.m_protocol = protocol;
    mapping.m_credentialId = credentialId;

    cxxtools::SerializationInfo si;

    si <<= mapping;

    sendCommand(CredentialAssetMappingServer::UPDATE_CREDENTIAL_MAPPING, {serialize(si)} );
  }

  void Accessor::updatePort(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol, const Port & port)
  {
    CredentialAssetMapping mapping;
    mapping.m_assetId = assetId;
    mapping.m_serviceId = serviceId;
    mapping.m_protocol = protocol;
    mapping.m_port = port;

    cxxtools::SerializationInfo si;

    si <<= mapping;

    sendCommand(CredentialAssetMappingServer::UPDATE_PORT_MAPPING, {serialize(si)} );
  }

  void Accessor::updateStatus(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol, Status status)
  {
    CredentialAssetMapping mapping;
    mapping.m_assetId = assetId;
    mapping.m_serviceId = serviceId;
    mapping.m_protocol = protocol;
    mapping.m_status = status;

    cxxtools::SerializationInfo si;

    si <<= mapping;

    sendCommand(CredentialAssetMappingServer::UPDATE_STATUS_MAPPING, {serialize(si)} );
  }

  void Accessor::updateExtendedInfo(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol, const MapExtendedInfo & extendedInfo)
  {
    CredentialAssetMapping mapping;
    mapping.m_assetId = assetId;
    mapping.m_serviceId = serviceId;
    mapping.m_protocol = protocol;
    mapping.m_extendedInfo = extendedInfo;

    cxxtools::SerializationInfo si;

    si <<= mapping;

    sendCommand(CredentialAssetMappingServer::UPDATE_INFO_MAPPING, {serialize(si)} );
  }

  bool Accessor::isMappingExisting(const AssetId & assetId, const ServiceId & serviceId, const Protocol & protocol) const
  {
    bool exist = true;

    try
    {
      getMapping(assetId, serviceId, protocol);
    }
    catch(const CamMappingDoesNotExistException &)
    {
      exist = false;
    }

    return exist;
  }

  const std::vector<CredentialAssetMapping> Accessor::getAssetMappings(const AssetId & assetId) const
  {
    std::vector<CredentialAssetMapping> mappings;

    std::vector<std::string> payload;

    payload = sendCommand(CredentialAssetMappingServer::GET_ASSET_MAPPINGS, {assetId} );

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= mappings;

    return mappings;
  }

  const std::vector<CredentialAssetMapping> Accessor::getMappings(const AssetId & assetId, const ServiceId & serviceId) const
  {
    std::vector<CredentialAssetMapping> mappings;

    std::vector<std::string> payload;

    payload = sendCommand(CredentialAssetMappingServer::GET_MAPPINGS, {assetId, serviceId} );

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= mappings;

    return mappings;
  }

  const std::vector<CredentialAssetMapping> Accessor::getAllMappings() const
  {
    std::vector<CredentialAssetMapping> mappings;

    std::vector<std::string> payload;

    payload = sendCommand(CredentialAssetMappingServer::GET_ALL_MAPPINGS, {} );

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= mappings;

    return mappings;
  }

  const std::vector<CredentialAssetMapping> Accessor::getCredentialMappings(const CredentialId & credentialId) const
  {
    std::vector<CredentialAssetMapping> mappings;

    std::vector<std::string> payload;

    payload = sendCommand(CredentialAssetMappingServer::GET_CRED_MAPPINGS, {credentialId} );

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= mappings;

    return mappings;
  }

  uint32_t Accessor::countCredentialMappingsForCredential(const CredentialId & credentialId) const
  {
    uint32_t counter = 0;

    std::vector<std::string> payload;

    payload = sendCommand(CredentialAssetMappingServer::COUNT_CRED_MAPPINGS, {credentialId} );

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= counter;

    return counter;
  }

  const std::map<CredentialId, uint32_t> Accessor::getAllCredentialCounter() const
  {
    std::map<CredentialId, uint32_t> counters;

    const std::vector<CredentialAssetMapping> mappings = getAllMappings();

    for(const CredentialAssetMapping & mapping : mappings)
    {
      if(!mapping.m_credentialId.empty())
      {
        if(counters.count(mapping.m_credentialId) == 0)
        {
          counters[mapping.m_credentialId] = 1;
        }
        else
        {
          counters[mapping.m_credentialId] = counters[mapping.m_credentialId] + 1;
        }
      }
    }

    return counters;
  }

  std::vector<std::string> Accessor::sendCommand(const std::string & command, const std::vector<std::string> & frames) const
  {
    std::vector<std::string> payload = {command};
    std::copy(frames.begin(), frames.end(), back_inserter(payload));

    std::vector<std::string> receivedFrames = m_requestClient.syncRequestWithReply(payload);

    //check if the first frame we get is an error
    if(receivedFrames[0] == "ERROR")
    {
      //It's an error and we will throw directly the exceptions
      if(receivedFrames.size() == 2)
      {
        CamException::throwCamException(receivedFrames.at(1));
      }
      else
      {
        throw CamProtocolErrorException("Missing data for error");
      }

    }

    return receivedFrames;
  }

} //namespace cam


//  --------------------------------------------------------------------------
//  Test of this class => This is used by fty_credential_asset_mapping_server_test
//  --------------------------------------------------------------------------


std::vector<std::pair<std::string,bool>> cam_accessor_test(mlm::MlmSyncClient & syncClient)
{
  std::vector<std::pair<std::string,bool>> testsResults;

  using namespace cam;

  printf(" ** cam_accessor_test: \n");

  std::string testNumber, testName;

  //test 1.X
  {
    //test 1.1 => test retrieve a mapping
    testNumber = "1.1";
    testName = "getMapping => existing mapping";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        AssetId assetId("asset-1");
        ServiceId serviceId("test-usage");
        Protocol protocol("http");

        const CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

        if(mapping.m_assetId != assetId)
        {
          throw std::runtime_error("Wrong asset id. Received '"+mapping.m_assetId+"' expected '"+assetId+"'");
        }

        if(mapping.m_serviceId != serviceId)
        {
          throw std::runtime_error("Wrong usage id. Received '"+mapping.m_serviceId+"' expected '"+serviceId+"'");
        }

        if(mapping.m_protocol != protocol)
        {
          throw std::runtime_error("Wrong protocol. Received '"+mapping.m_protocol+"' expected '"+protocol+"'");
        }

        if(mapping.m_port != "80")
        {
          throw std::runtime_error("Wrong port. Received '"+mapping.m_port+"' expected '80'");
        }

        if(mapping.m_credentialId != "cred-1")
        {
          throw std::runtime_error("Wrong credential id. Received '"+mapping.m_credentialId+"' expected 'cred-1'");
        }


        if(mapping.m_status != Status::UNKNOWN)
        {
          throw std::runtime_error("Wrong credential status");
        }

        if((mapping.m_extendedInfo.size() != 1 ) || (mapping.m_extendedInfo.at("port") != "80"))
        {
          throw std::runtime_error("Wrong extended info");
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 1.2 => test exception CamMappingDoesNotExistException
    testNumber = "1.2";
    testName = "getMapping => CamMappingDoesNotExistException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());

      AssetId assetId("asset-XXXXX");
      ServiceId serviceId("usage-XXXXX");
      Protocol protocol("protocol-XXXXX");

      try
      {
        Accessor accessor(syncClient);
        const CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);
        throw std::runtime_error("Mapping is returned");
      }
      catch(const CamMappingDoesNotExistException & e)
      {
        if( (e.getAssetId() == assetId) && ( e.getServiceId() == serviceId) && (e.getProtocol() == protocol) )
        {
          printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
          testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
        }
        else
        {
          printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
          printf("Error in excpetion format: AssetId expect '%s' received '%s', ServiceId excpected '%s' received '%s', Protocol excpected '%s' received '%s' \n",
            assetId.c_str(), e.getAssetId().c_str(), serviceId.c_str(), e.getServiceId().c_str(), protocol.c_str(), e.getProtocol().c_str());

          testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
        }
      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

  } // 1.X

  //test 2.X
  {
    AssetId assetId("asset-2");
    ServiceId serviceId("test-usage-2");
    Protocol protocol("test-proto");
    Port port("80");
    CredentialId credId("Test-mapping");
    Status status(Status::VALID);

    std::string key("key");
    std::string data("data");
    MapExtendedInfo extendedInfo;
    extendedInfo[key] = data;

    //test 2.1 => test create
    testNumber = "2.1";
    testName = "createMapping";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);
        accessor.createMapping(assetId, serviceId, protocol, port, credId, status, extendedInfo );

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 2.2 => test retrieve a the created mapping
    testNumber = "2.2";
    testName = "getMapping => fresh created mapping";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        const CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

        if(mapping.m_assetId != assetId)
        {
          throw std::runtime_error("Wrong asset id. Received '"+mapping.m_assetId+"' expected '"+assetId+"'");
        }

        if(mapping.m_serviceId != serviceId)
        {
          throw std::runtime_error("Wrong usage id. Received '"+mapping.m_serviceId+"' expected '"+serviceId+"'");
        }

        if(mapping.m_credentialId != credId)
        {
          throw std::runtime_error("Wrong credential id. Received '"+mapping.m_credentialId+"' expected '"+credId+"'");
        }

        if(mapping.m_status != status)
        {
          throw std::runtime_error("Wrong credential status");
        }

        if(mapping.m_protocol != protocol)
        {
          throw std::runtime_error("Wrong protocol. Received '"+mapping.m_protocol+"' expected '"+protocol+"'");
        }

        if(mapping.m_port != "80")
        {
          throw std::runtime_error("Wrong port. Received '"+mapping.m_port+"' expected '80'");
        }

        if((mapping.m_extendedInfo.size() != extendedInfo.size() ) || (mapping.m_extendedInfo.at(key) != data))
        {
          throw std::runtime_error("Wrong extended info");
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 2.3 => test exception CamMappingAlreadyExistsException
    testNumber = "2.3";
    testName = "createMapping => CamMappingAlreadyExistsException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());

      try
      {
        Accessor accessor(syncClient);
        accessor.createMapping(assetId, serviceId, protocol, port, credId, status, extendedInfo );

        throw std::runtime_error("Mapping is created");
      }
      catch(const CamMappingAlreadyExistsException & e)
      {
        if( (e.getAssetId() == assetId) && ( e.getServiceId() == serviceId) && (e.getProtocol() == protocol))
        {
          printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
          testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
        }
        else
        {
          printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
          printf("Error in excpetion format: AssetId expect '%s' received '%s', ServiceId excpected '%s' received '%s' \n",
            assetId.c_str(), e.getAssetId().c_str(), serviceId.c_str(), e.getServiceId().c_str());

          testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
        }
      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

  } // 2.X

  //test 3.X -> need 2.X success
  {

    AssetId assetId("asset-2");
    ServiceId serviceId("test-usage-2");
    Protocol protocol("test-proto");

    //test 3.1 => test remove
    testNumber = "3.1";
    testName = "removeMapping => existing mapping";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        accessor.removeMapping(assetId, serviceId, protocol);

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 3.2 => test exception CamMappingDoesNotExistException
    testNumber = "3.2";
    testName = "getMapping => CamMappingDoesNotExistException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);
        const CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

        throw std::runtime_error("Mapping is returned");
      }
      catch(const CamMappingDoesNotExistException & e)
      {
        if( (e.getAssetId() == assetId) && ( e.getServiceId() == serviceId) )
        {
          printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
          testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
        }
        else
        {
          printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
          printf("Error in excpetion format: AssetId expect '%s' received '%s', ServiceId excpected '%s' received '%s' \n",
            assetId.c_str(), e.getAssetId().c_str(), serviceId.c_str(), e.getServiceId().c_str());

          testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
        }
      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 3.3 => test exception CamMappingDoesNotExistException
    testNumber = "3.3";
    testName = "removeMapping => CamMappingDoesNotExistException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);
        accessor.removeMapping(assetId, serviceId, protocol);

        throw std::runtime_error("Mapping is removed");
      }
      catch(const CamMappingDoesNotExistException & e)
      {
        if( (e.getAssetId() == assetId) && ( e.getServiceId() == serviceId) )
        {
          printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
          testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
        }
        else
        {
          printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
          printf("Error in excpetion format: AssetId expect '%s' received '%s', ServiceId excpected '%s' received '%s' \n",
            assetId.c_str(), e.getAssetId().c_str(), serviceId.c_str(), e.getServiceId().c_str());

          testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
        }
      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

  } // 3.X

  //test 4.X -> need 2.X success
  {
    AssetId assetId("asset-4");
    ServiceId serviceId("test-usage-4");
    Protocol protocol("http");
    Port port("80");
    CredentialId credId("Test-mapping-update");
    Status status(Status::VALID);

    std::string key("key");
    std::string data("data");
    MapExtendedInfo extendedInfo;
    extendedInfo[key] = data;

    //test 4.1 => test create
    testNumber = "4.1";
    testName = "createMapping for the next test";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);
        accessor.createMapping(assetId, serviceId, protocol, port, credId, status, extendedInfo );

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 4.2 => test update mapping
    testNumber = "4.2";
    testName = "updateCredentialId";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        credId = "new_cred";
        accessor.updateCredentialId(assetId, serviceId, protocol, credId);

        const CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

        if(mapping.m_assetId != assetId)
        {
          throw std::runtime_error("Wrong asset id. Received '"+mapping.m_assetId+"' expected '"+assetId+"'");
        }

        if(mapping.m_serviceId != serviceId)
        {
          throw std::runtime_error("Wrong usage id. Received '"+mapping.m_serviceId+"' expected '"+serviceId+"'");
        }

        if(mapping.m_credentialId != credId)
        {
          throw std::runtime_error("Wrong credential id. Received '"+mapping.m_credentialId+"' expected '"+credId+"'");
        }

        if(mapping.m_protocol != protocol)
        {
          throw std::runtime_error("Wrong protocol. Received '"+mapping.m_protocol+"' expected '"+protocol+"'");
        }

        if(mapping.m_port != port)
        {
          throw std::runtime_error("Wrong port. Received '"+mapping.m_port+"' expected '"+port+"'");
        }

        if(mapping.m_status != Status::UNKNOWN)
        {
          throw std::runtime_error("Wrong credential status");
        }

        if((mapping.m_extendedInfo.size() != extendedInfo.size() ) || (mapping.m_extendedInfo.at(key) != data))
        {
          throw std::runtime_error("Wrong extended info");
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 4.3 => test exception CamMappingDoesNotExistException
    testNumber = "4.3";
    testName = "updateCredentialId => CamMappingDoesNotExistException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);
        accessor.updateCredentialId("XXXXXX", "XXXXXX", "XXXXX", credId);

        throw std::runtime_error("Mapping is updated");
      }
      catch(const CamMappingDoesNotExistException &)
      {
        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 4.4 => test update mapping
    testNumber = "4.4";
    testName = "updateStatus";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        status = Status::ERROR;

        accessor.updateStatus(assetId, serviceId, protocol, status);

        const CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

        if(mapping.m_assetId != assetId)
        {
          throw std::runtime_error("Wrong asset id. Received '"+mapping.m_assetId+"' expected '"+assetId+"'");
        }

        if(mapping.m_serviceId != serviceId)
        {
          throw std::runtime_error("Wrong usage id. Received '"+mapping.m_serviceId+"' expected '"+serviceId+"'");
        }

        if(mapping.m_protocol != protocol)
        {
          throw std::runtime_error("Wrong protocol. Received '"+mapping.m_protocol+"' expected '"+protocol+"'");
        }

        if(mapping.m_port != port)
        {
          throw std::runtime_error("Wrong port. Received '"+mapping.m_port+"' expected '"+port+"'");
        }

        if(mapping.m_credentialId != credId)
        {
          throw std::runtime_error("Wrong credential id. Received '"+mapping.m_credentialId+"' expected '"+credId+"'");
        }

        if(mapping.m_status != status)
        {
          throw std::runtime_error("Wrong credential status");
        }

        if((mapping.m_extendedInfo.size() != extendedInfo.size() ) || (mapping.m_extendedInfo.at(key) != data))
        {
          throw std::runtime_error("Wrong extended info");
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 4.5 => test exception CamMappingDoesNotExistException
    testNumber = "4.5";
    testName = "updateStatus => CamMappingDoesNotExistException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);
        accessor.updateStatus("XXXXXX", "XXXXXX", "XXXX", status);

        throw std::runtime_error("Mapping is updated");
      }
      catch(const CamMappingDoesNotExistException &)
      {
        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 4.6 => test update mapping
    testNumber = "4.6";
    testName = "updateExtendedInfo";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        data = "45";
        std::string extraKey("update-key");
        std::string extraData("update-data");

        extendedInfo[key] = data;
        extendedInfo[extraKey] = extraData;

        accessor.updateExtendedInfo(assetId, serviceId, protocol, extendedInfo);

        const CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

        if(mapping.m_assetId != assetId)
        {
          throw std::runtime_error("Wrong asset id. Received '"+mapping.m_assetId+"' expected '"+assetId+"'");
        }

        if(mapping.m_serviceId != serviceId)
        {
          throw std::runtime_error("Wrong usage id. Received '"+mapping.m_serviceId+"' expected '"+serviceId+"'");
        }

        if(mapping.m_protocol != protocol)
        {
          throw std::runtime_error("Wrong protocol. Received '"+mapping.m_protocol+"' expected '"+protocol+"'");
        }

        if(mapping.m_port != port)
        {
          throw std::runtime_error("Wrong port. Received '"+mapping.m_port+"' expected '"+port+"'");
        }

        if(mapping.m_credentialId != credId)
        {
          throw std::runtime_error("Wrong credential id. Received '"+mapping.m_credentialId+"' expected '"+credId+"'");
        }

        if(mapping.m_status != status)
        {
          throw std::runtime_error("Wrong credential status");
        }

        if((mapping.m_extendedInfo.size() != extendedInfo.size() ) || (mapping.m_extendedInfo.at(key) != data) || mapping.m_extendedInfo.at(extraKey) != extraData)
        {
          throw std::runtime_error("Wrong extended info");
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 4.7 => test exception CamMappingDoesNotExistException
    testNumber = "4.7";
    testName = "updateExtendedInfo => CamMappingDoesNotExistException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);
        accessor.updateExtendedInfo("XXXXXX", "XXXXXX", "XXXXXX", extendedInfo);

        throw std::runtime_error("Mapping is updated");
      }
      catch(const CamMappingDoesNotExistException &)
      {
        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 4.8 => test update mapping
    testNumber = "4.8";
    testName = "updatePort";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        port = "443";

        accessor.updatePort(assetId, serviceId, protocol, port);

        const CredentialAssetMapping mapping = accessor.getMapping(assetId, serviceId, protocol);

        if(mapping.m_assetId != assetId)
        {
          throw std::runtime_error("Wrong asset id. Received '"+mapping.m_assetId+"' expected '"+assetId+"'");
        }

        if(mapping.m_serviceId != serviceId)
        {
          throw std::runtime_error("Wrong usage id. Received '"+mapping.m_serviceId+"' expected '"+serviceId+"'");
        }

        if(mapping.m_protocol != protocol)
        {
          throw std::runtime_error("Wrong protocol. Received '"+mapping.m_protocol+"' expected '"+protocol+"'");
        }

        if(mapping.m_port != port)
        {
          throw std::runtime_error("Wrong port. Received '"+mapping.m_port+"' expected '"+port+"'");
        }

        if(mapping.m_credentialId != credId)
        {
          throw std::runtime_error("Wrong credential id. Received '"+mapping.m_credentialId+"' expected '"+credId+"'");
        }

        if(mapping.m_status != Status::UNKNOWN)
        {
          throw std::runtime_error("Wrong credential status");
        }

        if((mapping.m_extendedInfo.size() != extendedInfo.size() ) || (mapping.m_extendedInfo.at(key) != data))
        {
          throw std::runtime_error("Wrong extended info");
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 4.9 => test exception CamMappingDoesNotExistException
    testNumber = "4.9";
    testName = "updatePort => CamMappingDoesNotExistException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);
        accessor.updatePort("XXXXXX", "XXXXXX", "XXXX", port);

        throw std::runtime_error("Mapping is updated");
      }
      catch(const CamMappingDoesNotExistException &)
      {
        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 4.10 => test remove
    testNumber = "4.10";
    testName = "removeMapping => end of tests 4";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        accessor.removeMapping(assetId, serviceId, protocol);

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

  } // 4.X

  //test 5.X
  {
    //test 5.1
    testNumber = "5.1";
    testName = "isMappingExisting => true";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        AssetId assetId("asset-1");
        ServiceId serviceId("test-usage");
        Protocol protocol("http");

        if(! accessor.isMappingExisting(assetId,serviceId, protocol))
        {
          throw std::runtime_error("Mapping does not exist");
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 5.2
    testNumber = "5.2";
    testName = "isMappingExisting => false";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        AssetId assetId("asset-XXXXX");
        ServiceId serviceId("usage-XXXXX");
        Protocol protocol("protocol-XXXXXX");


        if(accessor.isMappingExisting(assetId,serviceId, protocol))
        {
          throw std::runtime_error("Mapping exists");
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

  } // 5.X


  //test 6.X
  {
    //test 6.1
    testNumber = "6.1";
    testName = "getAssetMappings => 3 mappings";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        AssetId assetId("assetA");

        const std::vector<CredentialAssetMapping> mappings = accessor.getAssetMappings(assetId);

        if(mappings.size() != 3)
        {
          throw std::runtime_error("Expected 3 mappings received "+std::to_string(mappings.size()));
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 6.2
    testNumber = "6.2";
    testName = "getAssetMappings => 0 mapping";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        AssetId assetId("XXXXXX");

        const std::vector<CredentialAssetMapping> mappings = accessor.getAssetMappings(assetId);

        if(mappings.size() != 0)
        {
          throw std::runtime_error("Expected 0 mapping received "+std::to_string(mappings.size()));
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

  } // 6.X

  //test 7.X
  {
    //test 7.1
    testNumber = "7.1";
    testName = "getAssetMappings => 3 mappings";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        CredentialId credId("credC");

        const std::vector<CredentialAssetMapping> mappings = accessor.getCredentialMappings(credId);

        if(mappings.size() != 3)
        {
          throw std::runtime_error("Expected 3 mappings received "+std::to_string(mappings.size()));
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 7.2
    testNumber = "7.2";
    testName = "getCredentialMappings => 0 mapping";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        CredentialId credId("XXXXXX");

        const std::vector<CredentialAssetMapping> mappings = accessor.getCredentialMappings(credId);

        if(mappings.size() != 0)
        {
          throw std::runtime_error("Expected 0 mapping received "+std::to_string(mappings.size()));
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

  } // 7.X

  //test 8.X
  {
    //test 8.1
    testNumber = "8.1";
    testName = "countCredentialMappingsForCredential => 3 mappings";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        CredentialId credId("credC");

        uint32_t counter = accessor.countCredentialMappingsForCredential(credId);

        if(counter != 3)
        {
          throw std::runtime_error("Expected 3 mappings received "+std::to_string(counter));
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 8.2
    testNumber = "8.2";
    testName = "countCredentialMappingsForCredential => 0 mapping";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        CredentialId credId("XXXXXX");

        uint32_t counter = accessor.countCredentialMappingsForCredential(credId);

        if(counter != 0)
        {
          throw std::runtime_error("Expected 0 mapping received "+std::to_string(counter));
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

  } // 8.X

  //test 9.X
  {
    //test 9.1
    testNumber = "9.1";
    testName = "getAllCredentialCounter => 3 mappings";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        const std::map<CredentialId, uint32_t> counters = accessor.getAllCredentialCounter();

        CredentialId credId("credC");

        if(counters.count(credId) == 0)
        {
          throw std::runtime_error("No mapping counted for "+credId);
        }

        if(counters.at(credId) != 3)
        {
          throw std::runtime_error("Expected 3 mappings received "+std::to_string(counters.at(credId)));
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 9.2
    testNumber = "9.2";
    testName = "getAllCredentialCounter => 0 mapping";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor(syncClient);

        CredentialId credId("XXXXXX");
        const std::map<CredentialId, uint32_t> counters = accessor.getAllCredentialCounter();

        if(counters.count(credId) != 0)
        {
          throw std::runtime_error("Mapping counted for "+credId);
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

  } // 9.X

  //test 10.X
  {
    //test 10.1
    testNumber = "10.1";
    testName = "several requests with one accessor";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor0(syncClient);

        for(uint8_t index = 0; index < 5; index++)
        {
          accessor0.getAllCredentialCounter();
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);


      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

    //test 10.2
    testNumber = "10.2";
    testName = "several requests with creating accessor each time";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        for(uint8_t index = 0; index < 5; index++)
        {
          Accessor accessor0(syncClient);
          accessor0.getAllCredentialCounter();
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

     //test 10.3
    testNumber = "10.3";
    testName = "several requests with 2 accessors";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor0(syncClient);
        Accessor accessor1(syncClient);

        for(uint8_t index = 0; index < 5; index++)
        {
          accessor0.getAllCredentialCounter();
          accessor1.getAllCredentialCounter();
        }

        printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);

      }
      catch(const std::exception& e)
      {
        printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
        printf("Error: %s\n",e.what());
        testsResults.emplace_back (" Test #"+testNumber+" "+testName,false);
      }
    }

  } // 10.X


  return testsResults;

}
