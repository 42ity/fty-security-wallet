/*  =========================================================================
    fty_credential_asset_mapping_server - Credential Asset Mapping Server

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

#include "fty_credential_asset_mapping_server.h"

#include "cam_credential_asset_mapping.h"
#include "cam_credential_asset_mapping_storage.h"
#include "cam_exception.h"
#include "cam_helpers.h"

#include "fty_security_wallet.h"

#include <sstream>
#include <cxxtools/jsonserializer.h>

namespace cam
{

    std::shared_ptr<CredentialAssetMappingStorage> CredentialAssetMappingServer::m_activeMapping = std::shared_ptr<CredentialAssetMappingStorage>(nullptr);

    CredentialAssetMappingServer::CredentialAssetMappingServer(zsock_t *pipe)
        : mlm::MlmAgent(pipe)
    {
        //initiate the commands handlers
        m_supportedCommands[CREATE_MAPPING] = handleCreateMapping;

        m_supportedCommands[GET_MAPPING] = handleGetMapping;

        m_supportedCommands[UPDATE_PORT_MAPPING] = handleUpdatePortMapping;
        m_supportedCommands[UPDATE_CREDENTIAL_MAPPING] = handleUpdateCredentialMapping;
        m_supportedCommands[UPDATE_STATUS_MAPPING] = handleUpdateStatusMapping;
        m_supportedCommands[UPDATE_INFO_MAPPING] = handleUpdateInfoMapping;
        m_supportedCommands[UPDATE_MAPPING] = handleUpdateMapping;

        m_supportedCommands[REMOVE_MAPPING] = handleRemoveMapping;

        m_supportedCommands[GET_ASSET_MAPPINGS] = handleGetAssetMappings;
        m_supportedCommands[GET_CRED_MAPPINGS] = handleGetCredentialMappings;
        m_supportedCommands[GET_MAPPINGS] = handleGetMappings;
        m_supportedCommands[GET_ALL_MAPPINGS] = handleGetAllMappings;

        m_supportedCommands[COUNT_CRED_MAPPINGS] = handleCountCredentialMappingsForCredential;
    }

    /* Commands implementation section*/
    //TODO remove: throw CamException("Command is not implemented yet!!");

    std::string CredentialAssetMappingServer::handleGetMapping(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. asset id
        * 1. usage id
        * 2. protocol
        */

        if(params.size() < 2)
        {
            throw CamBadCommandArgumentException("", "Command need at least 2 arguments");
        }

        const AssetId & assetId = params.at(0);
        const ServiceId & serviceId = params.at(1);
        const Protocol & protocol = params.at(2);

        const CredentialAssetMapping & mapping = m_activeMapping->getMapping(assetId, serviceId, protocol);

        cxxtools::SerializationInfo si;
    
        mapping.fillSerializationInfo(si);
        
        return serialize(si);
    }

    std::string CredentialAssetMappingServer::handleCreateMapping(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. CredentialMapping object in json
        */
        
        if(params.size() < 1)
        {
            throw CamBadCommandArgumentException("", "Command need at least 1 argument");
        }
        
        const cxxtools::SerializationInfo si = deserialize(params.at(0));

        CredentialAssetMapping mapping;
        
        mapping.fromSerializationInfo(si);
        
        if(m_activeMapping->isMappingExisting(mapping.m_assetId, mapping.m_serviceId, mapping.m_protocol))
        {
            throw CamMappingAlreadyExistsException(mapping.m_assetId, mapping.m_serviceId, mapping.m_protocol);
        }
        
        m_activeMapping->setMapping(mapping);
        
        m_activeMapping->save();
        
        return "";
    }

    std::string CredentialAssetMappingServer::handleRemoveMapping(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. asset id
        * 1. usage id
        * 2. protocol
        */

        if(params.size() < 2)
        {
            throw CamBadCommandArgumentException("", "Command need at least 2 arguments");
        }

        const AssetId & assetId = params.at(0);
        const ServiceId & serviceId = params.at(1);
        const Protocol & protocol = params.at(2);

        m_activeMapping->removeMapping(assetId, serviceId, protocol);
        
        m_activeMapping->save();
        
        return "";
    }

    std::string CredentialAssetMappingServer::handleUpdateMapping(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. CredentialMapping object in json
        */
        
        if(params.size() < 1)
        {
            throw CamBadCommandArgumentException("", "Command need at least 1 argument");
        }
        
        const cxxtools::SerializationInfo si = deserialize(params.at(0));

        CredentialAssetMapping mapping;
        
        mapping.fromSerializationInfo(si);
        
        if(! m_activeMapping->isMappingExisting(mapping.m_assetId, mapping.m_serviceId, mapping.m_protocol))
        {
            throw CamMappingDoesNotExistException(mapping.m_assetId, mapping.m_serviceId, mapping.m_protocol);
        }
        
        m_activeMapping->setMapping(mapping);
        
        m_activeMapping->save();
        
        return "";
    }

    std::string CredentialAssetMappingServer::handleUpdatePortMapping(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. CredentialMapping object in json
        */

        if(params.size() < 1)
        {
            throw CamBadCommandArgumentException("", "Command need at least 1 argument");
        }

        const cxxtools::SerializationInfo si = deserialize(params.at(0));

        CredentialAssetMapping receivedMapping;
        
        receivedMapping.fromSerializationInfo(si);

        CredentialAssetMapping existingMapping = m_activeMapping->getMapping(receivedMapping.m_assetId, receivedMapping.m_serviceId, receivedMapping.m_protocol);

        //update the mapping
        existingMapping.m_port = receivedMapping.m_port;
        existingMapping.m_status = Status::UNKNOWN;

        m_activeMapping->setMapping(existingMapping);
        
        m_activeMapping->save();
        
        return "";
    }

    std::string CredentialAssetMappingServer::handleUpdateCredentialMapping(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. CredentialMapping object in json
        */

        if(params.size() < 1)
        {
            throw CamBadCommandArgumentException("", "Command need at least 1 argument");
        }

        const cxxtools::SerializationInfo si = deserialize(params.at(0));

        CredentialAssetMapping receivedMapping;
        
        receivedMapping.fromSerializationInfo(si);

        CredentialAssetMapping existingMapping = m_activeMapping->getMapping(receivedMapping.m_assetId, receivedMapping.m_serviceId, receivedMapping.m_protocol);

        //update the mapping
        existingMapping.m_credentialId = receivedMapping.m_credentialId;
        existingMapping.m_status = Status::UNKNOWN;

        m_activeMapping->setMapping(existingMapping);
        
        m_activeMapping->save();
        
        return "";
    }

    std::string CredentialAssetMappingServer::handleUpdateStatusMapping(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. CredentialMapping object in json
        */

        if(params.size() < 1)
        {
            throw CamBadCommandArgumentException("", "Command need at least 1 argument");
        }

        const cxxtools::SerializationInfo si = deserialize(params.at(0));

        CredentialAssetMapping receivedMapping;
        
        receivedMapping.fromSerializationInfo(si);

        CredentialAssetMapping existingMapping = m_activeMapping->getMapping(receivedMapping.m_assetId, receivedMapping.m_serviceId, receivedMapping.m_protocol);

        //update the mapping
        existingMapping.m_status = receivedMapping.m_status;

        m_activeMapping->setMapping(existingMapping);
        
        m_activeMapping->save();
        
        return "";
    }

    std::string CredentialAssetMappingServer::handleUpdateInfoMapping(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. CredentialMapping object in json
        */

        if(params.size() < 1)
        {
            throw CamBadCommandArgumentException("", "Command need at least 1 argument");
        }

        const cxxtools::SerializationInfo si = deserialize(params.at(0));

        CredentialAssetMapping receivedMapping;
        
        receivedMapping.fromSerializationInfo(si);

        CredentialAssetMapping existingMapping = m_activeMapping->getMapping(receivedMapping.m_assetId, receivedMapping.m_serviceId, receivedMapping.m_protocol);

        //update the mapping
        existingMapping.m_extendedInfo = receivedMapping.m_extendedInfo;

        m_activeMapping->setMapping(existingMapping);
        
        m_activeMapping->save();
        
        return "";
    }

    std::string CredentialAssetMappingServer::handleGetAssetMappings(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. asset id
        */

        if(params.size() < 1)
        {
            throw CamBadCommandArgumentException("", "Command need at least 1 argument");
        }

        const AssetId & assetId = params.at(0);

        std::vector<CredentialAssetMapping> mappings = m_activeMapping->getAssetMappings(assetId);
        
        cxxtools::SerializationInfo si;

        si <<= mappings;

        return serialize(si);
    }

    std::string CredentialAssetMappingServer::handleGetMappings(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. asset id
        * 1. service id
        */

        if(params.size() < 2)
        {
            throw CamBadCommandArgumentException("", "Command need at least 2 arguments");
        }

        const AssetId & assetId = params.at(0);
        const ServiceId & serviceId = params.at(1);

        std::vector<CredentialAssetMapping> mappings = m_activeMapping->getMappings(assetId, serviceId);
        
        cxxtools::SerializationInfo si;

        si <<= mappings;

        return serialize(si);
    }

    std::string CredentialAssetMappingServer::handleGetAllMappings(const Sender & /*sender*/, const std::vector<std::string> & /*params*/)
    {
        /*
        * Parameters for this command:
        */

        std::vector<CredentialAssetMapping> mappings = m_activeMapping->getAllMappings();
        
        cxxtools::SerializationInfo si;

        si <<= mappings;

        return serialize(si);
    }



    std::string CredentialAssetMappingServer::handleGetCredentialMappings(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. credential id
        */

        if(params.size() < 1)
        {
            throw CamBadCommandArgumentException("", "Command need at least 1 argument");
        }

        const CredentialId & credentialId = params.at(0);

        std::vector<CredentialAssetMapping> mappings = m_activeMapping->getCredentialMappings(credentialId);
        
        cxxtools::SerializationInfo si;

        si <<= mappings;

        return serialize(si);
    }

    std::string CredentialAssetMappingServer::handleCountCredentialMappingsForCredential(const Sender & /*sender*/, const std::vector<std::string> & params)
    {
        /*
        * Parameters for this command:
        * 
        * 0. credential id
        */

        if(params.size() < 1)
        {
            throw CamBadCommandArgumentException("", "Command need at least 1 argument");
        }

        const CredentialId & credentialId = params.at(0);

        std::vector<CredentialAssetMapping> mappings = m_activeMapping->getCredentialMappings(credentialId);
        
        cxxtools::SerializationInfo si;

        si <<= static_cast<uint32_t>(mappings.size());

        return serialize(si);
    }

    /*
    * Agents methods
    */

    static std::string g_storageMapping = DEFAULT_STORAGE_MAPPING_PATH;

    bool CredentialAssetMappingServer::handlePipe(zmsg_t *message)
    {
        bool rv = true;
        ZstrGuard actor_command(zmsg_popstr(message));

        // $TERM actor command implementation is required by zactor_t interface.
        if (streq(actor_command, "$TERM"))
        {
            rv = false;
        }
        else if (streq (actor_command, "CONNECT"))
        {
            m_activeMapping = std::make_shared<CredentialAssetMappingStorage>(g_storageMapping);
            
            log_debug("Wallet found in %s", g_storageMapping.c_str());

            ZstrGuard endpoint(zmsg_popstr(message));
            ZstrGuard name(zmsg_popstr (message));
            if (endpoint && name)
            {
                connect(endpoint,name);
            }
        }
        else if (streq (actor_command, "STORAGE_MAPPING_PATH"))
        {
            ZstrGuard storage_mapping_path(zmsg_popstr(message));
            g_storageMapping = std::string(storage_mapping_path.get());
        }
        else
        {
            log_error("Unknown pipe command '%s'", actor_command.get());
        }

        return rv;
    }

    bool CredentialAssetMappingServer::handleMailbox(zmsg_t *message)
    {
        std::string correlationId;
        
        //try to address the request
        try
        {
            Subject subject(mlm_client_subject(client()));
            Sender uniqueSender(mlm_client_sender(client()));
                
            //ignore none "REQUEST" message
            if (subject != "REQUEST")
            {
                log_warning ("Received mailbox message with subject '%s' from '%s', ignoring", subject.c_str(), uniqueSender.c_str());
                return true;
            }
            
            //Get number of frame all the frame
            size_t numberOfFrame = zmsg_size(message);
            
            log_debug("Received mailbox message with subject '%s' from '%s' with %i frames", subject.c_str(), uniqueSender.c_str(), numberOfFrame);

            
            /*  Message is valid if the header contain at least the following frame:
            * 0. Correlation id
            * 1. Command
            */
            
            //TODO define a maximum to avoid DOS
            
            ZstrGuard ptrCorrelationId( zmsg_popstr(message) );
            ZstrGuard ptrCommand( zmsg_popstr(message) );
            
            std::vector<std::string> params;
        
            //we unstack all the other starting by the 3rd one.
            for(size_t index = 2; index < numberOfFrame; index++)
            {
                ZstrGuard param( zmsg_popstr(message) );
                params.push_back( std::string(param.get()) );
            }
            
            //Ensure the presence of data from the request
            if(ptrCorrelationId != nullptr)
            {
                correlationId = std::string(ptrCorrelationId.get());
            }
            
            if(correlationId.empty())
            {
                throw CamProtocolErrorException("Correlation id frame is empty");
            }
            
            Command command;
            
            if(ptrCommand != nullptr)
            {
                command = Command(ptrCommand.get());
            }
                
            if (command.empty())
            {
                throw CamProtocolErrorException("Command frame is empty");
            }
            
            // Don't reply to ERROR messages
            if (command == "ERROR")
            {
                log_warning ("Received <%s> message from '%s', ignoring", command.c_str(),
                    mlm_client_sender (client()) );
                
                return true;
            }
            
            //check if the command exist in the system
            if( m_supportedCommands.count(command) == 0)
            {
                throw CamUnsupportedCommandException(command + " not supported");
            }

            FctCommandHandler cmdHandler = m_supportedCommands[command];

            //extract the sender from unique sender id: <Sender>.[thread id in hexa]
            Sender sender = uniqueSender.substr(0, (uniqueSender.size()-(sizeof(pid_t)*2)-1));
            
            //Execute the command
            std::string result = cmdHandler(sender, params);

            //send the result
            zmsg_t *reply = zmsg_new();
            
            zmsg_addstr (reply, correlationId.c_str());
            zmsg_addstr (reply, result.c_str());

            int rv = mlm_client_sendto (client(), mlm_client_sender (client()),
                                "REPLY", NULL, 1000, &reply);
            if (rv != 0)
            {
                log_error ("s_handle_mailbox: failed to send reply to %s ",
                        mlm_client_sender (client()));
            }

        }
        catch(CamException &e)
        {
            log_warning("%s", e.what());
            
            //send the error
            zmsg_t *reply = generateErrorMsg(correlationId, e.toJson());
            
            int rv = mlm_client_sendto (client(), mlm_client_sender (client()),
                                    "REPLY", NULL, 1000, &reply);
            if (rv != 0)
            {
                log_error ("secw_handle_mailbox: failed to send reply to %s ",
                            mlm_client_sender (client()));
            }
        }
        catch (std::exception &e)
        {
            log_error("Unexpected error: %s", e.what());
            
            //send the error
            zmsg_t *reply = generateErrorMsg(correlationId, "");
            
            int rv = mlm_client_sendto (client(), mlm_client_sender (client()),
                                    "REPLY", NULL, 1000, &reply);
            if (rv != 0)
            {
                log_error ("secw_handle_mailbox: failed to send reply to %s ",
                            mlm_client_sender (client()));
            }
        }
        catch (...) //show must go one => Log and ignore the unknown error
        {
            log_error("Unexpected error: unknown");
            
            //send the error
            zmsg_t *reply = generateErrorMsg(correlationId, "");
            
            int rv = mlm_client_sendto (client(), mlm_client_sender (client()),
                                    "REPLY", NULL, 1000, &reply);
            if (rv != 0)
            {
                log_error ("secw_handle_mailbox: failed to send reply to %s ",
                            mlm_client_sender (client()));
            }
        }

        return true;
    }

    /* Helpers section */

    zmsg_t * CredentialAssetMappingServer::generateErrorMsg( const std::string & correlationId,  const std::string & errPayload)
    {
        zmsg_t *error = zmsg_new();
        zmsg_addstr (error, correlationId.c_str());
        zmsg_addstr (error, "ERROR");
        zmsg_addstr (error, errPayload.c_str());
        return error;
    }

} //namespace cam

void fty_credential_asset_mapping_server(zsock_t *pipe, void *args)
{
    cam::CredentialAssetMappingServer server(pipe);
    server.mainloop();
}

//  --------------------------------------------------------------------------
//  Self test of this class

// If your selftest reads SCMed fixture data, please keep it in
// src/selftest-ro; if your test creates filesystem objects, please
// do so under src/selftest-rw.
// The following pattern is suggested for C selftest code:
//    char *filename = NULL;
//    filename = zsys_sprintf ("%s/%s", SELFTEST_DIR_RO, "mytemplate.file");
//    assert (filename);
//    ... use the "filename" for I/O ...
//    zstr_free (&filename);
// This way the same "filename" variable can be reused for many subtests.
#define SELFTEST_DIR_RO "src/selftest-ro"
#define SELFTEST_DIR_RW "src/selftest-rw"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#include <fstream>

void
fty_credential_asset_mapping_server_test (bool verbose)
{
    std::vector<std::pair<std::string,bool>> testsServerResults;
    printf ("\n ** fty_credential_asset_mapping_server: \n");
    assert (SELFTEST_DIR_RO);
    assert (SELFTEST_DIR_RW);

    static const char* endpoint = "inproc://fty-credential-asset-mapping-test";

    //Copy the mapping file
    {
        std::ifstream source(SELFTEST_DIR_RO"/mapping.json", std::ios::binary);
        std::ofstream dest(SELFTEST_DIR_RW"/mapping.json", std::ios::binary | std::ofstream::trunc );
        dest << source.rdbuf();

        dest.close();
        source.close();
    }

    zactor_t *broker = zactor_new (mlm_server, (void*) "Malamute");
    zstr_sendx (broker, "BIND", endpoint, NULL);
    if (verbose)
        zstr_send (broker, "VERBOSE");
    
    zactor_t *server = zactor_new (fty_credential_asset_mapping_server, (void *)endpoint);
    //set configuration parameters
    zstr_sendx (server, "STORAGE_MAPPING_PATH", SELFTEST_DIR_RW"/mapping.json", NULL);
    zstr_sendx (server, "CONNECT", endpoint, MAPPING_AGENT, NULL);

    mlm_client_t *client = mlm_client_new();
    mlm_client_connect (client, endpoint, 1000, "cam-server-test");

    //test 1 => Invalid REQUEST command
    printf("\n-----------------------------------------------------------------------\n");
    try
    {
        log_debug ("*=> Test #1 Invalid REQUEST command");
            
        zmsg_t *request = zmsg_new();
        ZuuidGuard  zuuid(zuuid_new ());
        zmsg_addstr (request, zuuid_str_canonical (zuuid));
        zmsg_addstr (request, "NON_EXISTENT_COMMAND");
        mlm_client_sendto (client, MAPPING_AGENT, "REQUEST", NULL, 1000, &request);

        ZmsgGuard recv(mlm_client_recv (client));
        if(zmsg_size (recv) != 3)
        {
            throw std::runtime_error("Bad number of frames received");
        }
        ZstrGuard str(zmsg_popstr (recv));
        if(!streq (str, zuuid_str_canonical (zuuid)))
        {
            throw std::runtime_error("Bad correlation id received");
        }

        str = zmsg_popstr (recv);
        if(!streq (str, "ERROR"))
        {
            throw std::runtime_error("Bad message type received: "+std::string(str));
        }

        testsServerResults.emplace_back("Test #1 Invalid REQUEST command",true);

    }
    catch(const std::exception& e)
    {
        log_debug(" *<= Test #1 > Failed");
        log_debug("Error: %s\n\n",e.what());

        testsServerResults.emplace_back("Test #1 Invalid REQUEST command",false);
    }
    
    //end of the server tests
    mlm_client_destroy(&client);

    //Tests from the lib
    std::vector<std::pair<std::string,bool>> testLibResults = cam_accessor_test();

    printf("\n-----------------------------------------------------------------------\n");
    
    uint32_t testsPassed = 0;
    uint32_t testsFailed = 0;
    
    //Print all the result
    printf("\n\tTests from the server:\n");
    for(const auto & result : testsServerResults)
    {
        if(result.second)
        {
            printf(ANSI_COLOR_GREEN"\tOK " ANSI_COLOR_RESET "\t%s\n",result.first.c_str());
            testsPassed++;
        }
        else
        {
            printf(ANSI_COLOR_RED"\tNOK" ANSI_COLOR_RESET "\t%s\n",result.first.c_str());
            testsFailed++;
        }
    }
    
    printf("\n\tTests from the lib: \n");
    for(const auto & result : testLibResults)
    {
        if(result.second)
        {
            printf(ANSI_COLOR_GREEN"\tOK " ANSI_COLOR_RESET "\t%s\n",result.first.c_str());
            testsPassed++;
        }
        else
        {
            printf(ANSI_COLOR_RED"\tNOK" ANSI_COLOR_RESET "\t%s\n",result.first.c_str());
            testsFailed++;
        }
    }

    printf("\n-----------------------------------------------------------------------\n");
    
    if(testsFailed == 0)
    {
        printf(ANSI_COLOR_GREEN"\n %i tests passed, everything is ok\n" ANSI_COLOR_RESET "\n",testsPassed);
        
        /*std::ifstream database(SELFTEST_DIR_RW"/mapping.json", std::ios::binary);
        std::cerr << database.rdbuf() << std::endl;

        database.close();*/
    }
    else
    {
        printf(ANSI_COLOR_RED"\n!!!!!!!! %i/%i tests did not pass !!!!!!!! \n" ANSI_COLOR_RESET "\n",testsFailed,(testsPassed+testsFailed));

        printf("Content of the mapping at the end of tests: \n");
        printf("\n\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

        std::ifstream database(SELFTEST_DIR_RW"/mapping.json", std::ios::binary);
        std::cerr << database.rdbuf() << std::endl;

        database.close();
        printf("\n\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

        assert(false);
    }


    zstr_sendm (server, "$TERM");
    sleep(1);

    zactor_destroy (&server);
    zactor_destroy (&broker);
}
