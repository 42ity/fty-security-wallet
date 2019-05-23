/*  =========================================================================
    cam_accessor - Accessor to Credential asset mapping

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

#include "cam_accessor.h"

#include "cam_helpers.h"

#include <fty_common_mlm.h>

#include "fty_credential_asset_mapping_server.h"

#include "fty_security_wallet.h"

namespace cam
{
  Accessor::Accessor(const ClientId & clientId,
                  uint32_t timeout,
                  const std::string & endPoint):
    m_clientId(clientId),
    m_timeout(timeout),
    m_client(mlm_client_new())
  {
    mlm_client_connect(m_client, endPoint.c_str(), m_timeout, m_clientId.c_str());
  }

  Accessor::~Accessor()
  {
    mlm_client_destroy(&m_client);
  }

  void Accessor::createMapping( const AssetId & assetId, const UsageId & usageId,
                      const CredentialId & credentialId, CredentialStatus status,
                      const MapExtendedInfo & extendedInfo)
  {
    CredentialAssetMapping mapping;

    mapping.m_assetId = assetId;
    mapping.m_usageId = usageId;
    mapping.m_credentialId = credentialId;
    mapping.m_credentialStatus = status;
    mapping.m_extendedInfo = extendedInfo;

    cxxtools::SerializationInfo si;

    si <<= mapping;

    sendCommand(CredentialAssetMappingServer::CREATE_MAPPING, {serialize(si)} );
  }

  const CredentialAssetMapping Accessor::getMapping(const AssetId & assetId, const UsageId & usageId) const
  {
    CredentialAssetMapping mapping;

    std::vector<std::string> payload;

    payload = sendCommand(CredentialAssetMappingServer::GET_MAPPING, {assetId, usageId} );

    cxxtools::SerializationInfo si = deserialize(payload.at(0));

    si >>= mapping;

    return mapping;
  }

  void Accessor::removeMapping(const AssetId & assetId, const UsageId & usageId)
  {
    sendCommand(CredentialAssetMappingServer::REMOVE_MAPPING, {assetId, usageId} );
  }

  /*bool isMappingExisting(const AssetId & assetId, const UsageId & usageId) const;
  
  void updateCredentialId(const AssetId & assetId, const UsageId & usageId, const CredentialId & credentialId);
  void updateCredentialStatus(const AssetId & assetId, const UsageId & usageId, CredentialStatus status);
  void updateExtendedInfo(const AssetId & assetId, const UsageId & usageId, const MapExtendedInfo & extendedInfo);

  void removeMapping(const AssetId & assetId, const UsageId & usageId);*/

  std::vector<std::string> Accessor::sendCommand(const std::string & command, const std::vector<std::string> & frames) const
  {
    //Prepare the request:
    zmsg_t *request = zmsg_new();
    ZuuidGuard  zuuid(zuuid_new ());
    zmsg_addstr (request, zuuid_str_canonical (zuuid));

    //add the command
    zmsg_addstr (request, command.c_str());

    //add all the extra frames
    for(const std::string & frame : frames )
    {
      zmsg_addstr (request, frame.c_str());
    }

    //send the message
    mlm_client_sendto (m_client, MAPPING_AGENT, "REQUEST", NULL, m_timeout, &request);

    //Get the reply
    ZmsgGuard recv(mlm_client_recv (m_client));

    //Get number of frame all the frame
    size_t numberOfFrame = zmsg_size(recv);

    if(numberOfFrame < 2)
    {
      throw CamProtocolErrorException("Wrong number of frame");
    }

    //Check the message
    ZstrGuard str(zmsg_popstr (recv));
    if(!streq (str, zuuid_str_canonical (zuuid)))
    {
      throw CamProtocolErrorException("Mismatch correlation id");
    }

    std::vector<std::string> receivedFrames;

    //we unstack all the other frame starting by the 2rd one.
    for(size_t index = 1; index < numberOfFrame; index++)
    {
      ZstrGuard frame( zmsg_popstr(recv) );
      receivedFrames.push_back( std::string(frame.get()) );
    }

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


#define CAM_SELFTEST_CLIENT_ID "cam-client-test"

std::vector<std::pair<std::string,bool>> cam_accessor_test()
{
  std::vector<std::pair<std::string,bool>> testsResults;

  static const char* endpoint = "inproc://fty-credential-asset-mapping-test";
  
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
        Accessor accessor( CAM_SELFTEST_CLIENT_ID, 1000, endpoint);
        
        AssetId assetId("asset-1");
        UsageId usageId("test-usage");

        const CredentialAssetMapping mapping = accessor.getMapping(assetId, usageId);

        if(mapping.m_assetId != assetId)
        {
          throw std::runtime_error("Wrong asset id. Received '"+mapping.m_assetId+"' expected '"+assetId+"'");
        }

        if(mapping.m_usageId != usageId)
        {
          throw std::runtime_error("Wrong usage id. Received '"+mapping.m_usageId+"' expected '"+usageId+"'");
        }

        if(mapping.m_credentialId != "mapping-1")
        {
          throw std::runtime_error("Wrong credential id. Received '"+mapping.m_credentialId+"' expected 'mapping-1'");
        }

        if(mapping.m_credentialStatus != CredentialStatus::UNKNOWN)
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

    //test 1.2 => test exception CamMappingDoNotExistException
    testNumber = "1.2";
    testName = "getMapping => CamMappingDoNotExistException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      
      AssetId assetId("asset-XXXXX");
      UsageId usageId("usage-XXXXX");

      try
      {
        Accessor accessor( CAM_SELFTEST_CLIENT_ID, 1000, endpoint);
        const CredentialAssetMapping mapping = accessor.getMapping(assetId, usageId);
        throw std::runtime_error("Mapping is returned");
      }
      catch(const CamMappingDoNotExistException & e)
      {
        if( (e.getAssetId() == assetId) && ( e.getUsageId() == usageId) )
        {
          printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
          testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
        }
        else
        {
          printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
          printf("Error in excpetion format: AssetId expect '%s' received '%s', UsageId excpected '%s' received '%s' \n",
            assetId.c_str(), e.getAssetId().c_str(), usageId.c_str(), e.getUsageId().c_str());

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
    UsageId usageId("test-usage-2");
    CredentialId credId("Test-mapping");
    CredentialStatus status(CredentialStatus::VALID);
    
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
        Accessor accessor( CAM_SELFTEST_CLIENT_ID, 1000, endpoint);
        accessor.createMapping(assetId, usageId, credId, status, extendedInfo );
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
        Accessor accessor( CAM_SELFTEST_CLIENT_ID, 1000, endpoint);
        
        const CredentialAssetMapping mapping = accessor.getMapping(assetId, usageId);

        if(mapping.m_assetId != assetId)
        {
          throw std::runtime_error("Wrong asset id. Received '"+mapping.m_assetId+"' expected '"+assetId+"'");
        }

        if(mapping.m_usageId != usageId)
        {
          throw std::runtime_error("Wrong usage id. Received '"+mapping.m_usageId+"' expected '"+usageId+"'");
        }

        if(mapping.m_credentialId != credId)
        {
          throw std::runtime_error("Wrong credential id. Received '"+mapping.m_credentialId+"' expected '"+credId+"'");
        }

        if(mapping.m_credentialStatus != status)
        {
          throw std::runtime_error("Wrong credential status");
        }

        if((mapping.m_extendedInfo.size() != 1 ) || (mapping.m_extendedInfo.at(key) != data))
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

    //test 2.3 => test exception CamMappingAlreadyExistException
    testNumber = "2.3";
    testName = "createMapping => CamMappingAlreadyExistException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());

      try
      {
        Accessor accessor( CAM_SELFTEST_CLIENT_ID, 1000, endpoint);
        accessor.createMapping(assetId, usageId, credId, status, extendedInfo );

        throw std::runtime_error("Mapping is created");
      }
      catch(const CamMappingAlreadyExistException & e)
      {
        if( (e.getAssetId() == assetId) && ( e.getUsageId() == usageId) )
        {
          printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
          testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
        }
        else
        {
          printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
          printf("Error in excpetion format: AssetId expect '%s' received '%s', UsageId excpected '%s' received '%s' \n",
            assetId.c_str(), e.getAssetId().c_str(), usageId.c_str(), e.getUsageId().c_str());

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
    UsageId usageId("test-usage-2");

    //test 3.1 => test remove
    testNumber = "3.1";
    testName = "removeMapping => existing mapping";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor( CAM_SELFTEST_CLIENT_ID, 1000, endpoint);

        accessor.removeMapping(assetId, usageId);

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

    //test 3.2 => test exception CamMappingDoNotExistException
    testNumber = "3.2";
    testName = "getMapping => CamMappingDoNotExistException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor( CAM_SELFTEST_CLIENT_ID, 1000, endpoint);
        const CredentialAssetMapping mapping = accessor.getMapping(assetId, usageId);

        throw std::runtime_error("Mapping is returned");
      }
      catch(const CamMappingDoNotExistException & e)
      {
        if( (e.getAssetId() == assetId) && ( e.getUsageId() == usageId) )
        {
          printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
          testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
        }
        else
        {
          printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
          printf("Error in excpetion format: AssetId expect '%s' received '%s', UsageId excpected '%s' received '%s' \n",
            assetId.c_str(), e.getAssetId().c_str(), usageId.c_str(), e.getUsageId().c_str());

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

    //test 3.3 => test exception CamMappingDoNotExistException
    testNumber = "3.3";
    testName = "removeMapping => CamMappingDoNotExistException";
    printf("\n-----------------------------------------------------------------------\n");
    {
      printf(" *=>  Test #%s %s\n", testNumber.c_str(), testName.c_str());
      try
      {
        Accessor accessor( CAM_SELFTEST_CLIENT_ID, 1000, endpoint);
        accessor.removeMapping(assetId, usageId);
        
        throw std::runtime_error("Mapping is removed");
      }
      catch(const CamMappingDoNotExistException & e)
      {
        if( (e.getAssetId() == assetId) && ( e.getUsageId() == usageId) )
        {
          printf(" *<=  Test #%s > Ok\n", testNumber.c_str());
          testsResults.emplace_back (" Test #"+testNumber+" "+testName,true);
        }
        else
        {
          printf(" *<=  Test #%s > Failed\n", testNumber.c_str());
          printf("Error in excpetion format: AssetId expect '%s' received '%s', UsageId excpected '%s' received '%s' \n",
            assetId.c_str(), e.getAssetId().c_str(), usageId.c_str(), e.getUsageId().c_str());

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

  return testsResults;
  
}
