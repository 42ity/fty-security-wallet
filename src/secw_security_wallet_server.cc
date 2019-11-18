/*  =========================================================================
    secw_security_wallet_server - Security Wallet Server

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
    secw_security_wallet_server - secw broker agent
@discuss
@end
*/

#include "fty_security_wallet_classes.h"

#include <sstream>
#include <cxxtools/jsonserializer.h>

#include "secw_exception.h"
#include "secw_configuration.h"
#include "secw_document.h"

#include "secw_helpers.h"
#include "fty_srr_dto.h"

using namespace std::placeholders;

namespace secw
{

    SecurityWalletServer::SecurityWalletServer( const std::string & configurationPath,
                                                const std::string & databasePath,
                                                fty::StreamPublisher & streamPublisher,
                                                const std::string & srrEndpoint,
                                                const std::string & srrAgentName)
        :   m_activeWallet(configurationPath, databasePath),
            m_streamPublisher(streamPublisher)
    {
        //initiate the commands handlers
        m_supportedCommands[GET_PORTFOLIO_LIST] = std::bind(&SecurityWalletServer::handleGetListPortfolio, this, _1, _2);

        m_supportedCommands[GET_CONSUMER_USAGES] = std::bind(&SecurityWalletServer::handleGetConsumerUsages, this, _1, _2);
        m_supportedCommands[GET_PRODUCER_USAGES] = std::bind(&SecurityWalletServer::handleGetProducerUsages, this, _1, _2);

        m_supportedCommands[GET_LIST_WITH_SECRET] = std::bind(&SecurityWalletServer::handleGetListDocumentsWithSecret, this, _1, _2);
        m_supportedCommands[GET_LIST_WITHOUT_SECRET] = std::bind(&SecurityWalletServer::handleGetListDocumentsWithoutSecret, this, _1, _2);

        m_supportedCommands[GET_WITHOUT_SECRET] = std::bind(&SecurityWalletServer::handleGetDocumentWithoutSecret, this, _1, _2);
        m_supportedCommands[GET_WITH_SECRET] = std::bind(&SecurityWalletServer::handleGetDocumentWithSecret, this, _1, _2);

        m_supportedCommands[GET_WITHOUT_SECRET_BY_NAME] = std::bind(&SecurityWalletServer::handleGetDocumentWithoutSecretByName, this, _1, _2);
        m_supportedCommands[GET_WITH_SECRET_BY_NAME] = std::bind(&SecurityWalletServer::handleGetDocumentWithSecretByName, this, _1, _2);

        m_supportedCommands[CREATE] = std::bind(&SecurityWalletServer::handleCreate, this, _1, _2);
        m_supportedCommands[DELETE] = std::bind(&SecurityWalletServer::handleDelete, this, _1, _2);
        m_supportedCommands[UPDATE] = std::bind(&SecurityWalletServer::handleUpdate, this, _1, _2);


        //add support for SRR here (need to rework after)
        if((!srrEndpoint.empty()) && (!srrAgentName.empty()))
        {
            m_msgBus.reset(messagebus::MlmMessageBus(srrEndpoint, srrAgentName));
            m_msgBus->connect();

            // Listen all incoming request
            m_msgBus->receive("ETN.Q.IPMCORE.SECUWALLET", std::bind(&SecurityWalletServer::handleSRRRequest, this, _1));
        }  

    }

    SecurityWalletServer::~SecurityWalletServer()
    {
        //ensure nothing else is on going
        std::unique_lock<std::mutex>(m_lock);
    }

    std::vector<std::string> SecurityWalletServer::handleRequest(const Sender & sender, const std::vector<std::string> & payload)
    {
        //ensure nothing else is on going
        std::unique_lock<std::mutex>(m_lock);

        try
        {
            if(payload.size() == 0)
            {
                throw SecwProtocolErrorException("Command frame is empty");
            }

            Command cmd = payload.at(0);

            if(cmd == "ERROR" || cmd == "OK")
            {
                //avoid loop
                return {};
            }

            //check if the command exist in the system
            if( m_supportedCommands.count(cmd) == 0)
            {
                throw SecwUnsupportedCommandException(cmd + " not supported");
            }

            FctCommandHandler cmdHandler = m_supportedCommands[cmd];

            // Declaring new vector 
            std::vector<std::string> params(payload.begin()+1, payload.end());

            std::string result = cmdHandler(sender, params);

            return {result};
        }
        catch(SecwException &e)
        {
            log_warning("%s", e.what());
            return {"ERROR", e.toJson()};
        }
        catch (std::exception &e)
        {
            log_error("Unexpected error: %s", e.what());
            return {"ERROR",""};
        }
        catch (...) //show must go one => Log and ignore the unknown error
        {
            log_error("Unexpected error: unknown");
            return {"ERROR",""};
        }
    }

    void SecurityWalletServer::handleSRRRequest(messagebus::Message msg)
    {
        log_debug("Configuration handle request");
        try
        {
            dto::UserData userData;
            // Get request
            dto::UserData data = msg.userData();
            dto::srr::ConfigQueryDto configQuery;
            data >> configQuery;

            //log_debug("Config query-> action: %s", configQuery.action.c_str());
            
            if(configQuery.action == dto::srr::Action::SAVE)
            {
                //ensure nothing else is on going
                std::unique_lock<std::mutex>(m_lock);
                //get the srr files
                
                //dto::config::ConfigResponseDto respDto(""/*configQuery.featureName*/, STATUS_FAILED);
                /*std::map<std::string, cxxtools::SerializationInfo> configSiList;
                for(auto const& feature: configQuery.features)
                {
                    // Get the configuration file path name from class variable m_parameters
                    std::string configurationFileName = AUGEAS_FILES + m_parameters.at(feature) + ANY_NODES;
                    log_debug("Configuration file name: %s", configurationFileName.c_str());
                    
                    cxxtools::SerializationInfo si;
                    getConfigurationToJson(si, configurationFileName);
                    configSiList[feature] = si;
                }
                // Set response
                setSaveResponse(configSiList, respDto);
                userData << respDto;*/
            } 
            else if ((configQuery.action == dto::srr::Action::RESTORE) && (!configQuery.data.empty()))
            {
                //ensure nothing else is on going
                std::unique_lock<std::mutex>(m_lock);
                // To store response
                /*dto::srr::SrrRestoreDtoList srrRestoreDtoList(STATUS_SUCCESS); 
                // Get request and serialize it
                cxxtools::SerializationInfo restoreSi;
                readFromString(configQuery.data, restoreSi);
                // Get data member
                cxxtools::SerializationInfo siData = restoreSi.getMember(DATA_MEMBER);
                cxxtools::SerializationInfo::Iterator it;
                for (it = siData.begin(); it != siData.end(); ++it)
                {
                    dto::srr::SrrRestoreDto respDto(it->begin()->name(), STATUS_FAILED);
                    // Build the augeas configuration file name.
                    std::string configurationFileName = AUGEAS_FILES + m_parameters.at(respDto.name);
                    log_debug("Restore configuration for: %s, with configuration file: %s", respDto.name.c_str(), configurationFileName.c_str());
                    
                    cxxtools::SerializationInfo siData = it->getMember(respDto.name).getMember(DATA_MEMBER);
                    int returnValue = setConfiguration(&siData, configurationFileName);
                    if (returnValue == 0)
                    {
                        log_debug("Restore configuration for: %s succeed!", respDto.name.c_str());
                        respDto.status = STATUS_SUCCESS;
                    } 
                    else
                    {
                        std::string errorMsg = "Restore configuration for: " + respDto.name + " failed!";
                        log_error(errorMsg.c_str());
                        respDto.error = errorMsg;
                        srrRestoreDtoList.status = STATUS_FAILED;
                    }
                    srrRestoreDtoList.responseList.push_back(respDto);
               }
               userData << srrRestoreDtoList;*/
            } 
            else
            {
                throw std::runtime_error("Request not valid");
            }
            // Send response
            //sendResponse(msg, userData, configQuery.action);
        }
        catch (const std::out_of_range& oor)
        {
            log_error("Feature name not found");
        }
        catch (std::exception &e)
        {
            log_error("Unexpected error: %s", e.what());
        }
        catch (...)
        {
            log_error("Unexpected error: unknown");
        }
    }


    std::string SecurityWalletServer::handleGetListPortfolio(const Sender & /*sender*/, const std::vector<std::string> & /*params*/)
    {
        /*
         * No parameters for this command.
         * 
         */

        cxxtools::SerializationInfo si;
        si <<= m_activeWallet.getPortfolioNames();

        return serialize(si);
    }

    std::string SecurityWalletServer::handleGetConsumerUsages(const Sender & sender, const std::vector<std::string> & params)
    {
        /*
         * Parameters for this command:
         * 
         * 0. name of the portfolio
         */

        if(params.size() != 1)
        {
            throw SecwBadCommandArgumentException("Command needs at least 1 argument");
        }

        const std::string & portfolioName = params[0];

        cxxtools::SerializationInfo si;
        si <<= m_activeWallet.getConfiguration(portfolioName).getUsageIdsForConsummer(sender);

        return serialize(si);
    }

    std::string SecurityWalletServer::handleGetProducerUsages(const Sender & sender, const std::vector<std::string> & params)
    {
        /*
         * Parameters for this command:
         * 
         * 0. name of the portfolio
         */

        if(params.size() != 1)
        {
            throw SecwBadCommandArgumentException("Command needs at least 1 argument");
        }

        const std::string & portfolioName = params[0];

        cxxtools::SerializationInfo si;
        si <<= m_activeWallet.getConfiguration(portfolioName).getUsageIdsForProducer(sender);

        return serialize(si);
    }


    std::string SecurityWalletServer::handleGetDocumentWithSecret(const Sender & sender, const std::vector<std::string> & params)
    {
        /*
         * Parameters for this command:
         * 
         * 0. name of the portfolio
         * 1. document id
         */

        if(params.size() != 2)
        {
            throw SecwBadCommandArgumentException("Command needs at least 2 arguments");
        }

        const std::string & portfolioName = params[0];
        const Id & id = params[1];

        //check global access
        std::set<UsageId> allowedUsageIds = m_activeWallet.getConfiguration(portfolioName).getUsageIdsForConsummer(sender);

        if( allowedUsageIds.size() == 0)
        {
            throw SecwIllegalAccess("You do not have access to this document");
        }

        DocumentPtr doc  = m_activeWallet.getPortfolio(portfolioName).getDocument(id);

        if(!hasCommonUsageIds(allowedUsageIds, doc->getUsageIds()))
        {
            throw SecwIllegalAccess("You do not have access to this document");
        }

        cxxtools::SerializationInfo si;

        doc->fillSerializationInfoWithSecret(si);

        return serialize(si);
    }

    std::string SecurityWalletServer::handleGetDocumentWithoutSecret(const Sender & sender, const std::vector<std::string> & params)
    {
        /*
         * Parameters for this command:
         * 
         * 0. name of the portfolio
         * 1. document id
         */

        if(params.size() != 2)
        {
            throw SecwBadCommandArgumentException("Command needs at least 2 arguments");
        }

        const std::string & portfolioName = params[0];
        const Id & id = params[1];

        DocumentPtr doc  = m_activeWallet.getPortfolio(portfolioName).getDocument(id);

        cxxtools::SerializationInfo si;

        doc->fillSerializationInfoWithoutSecret(si);

        return serialize(si);
    }

    std::string SecurityWalletServer::handleGetDocumentWithSecretByName(const Sender & sender, const std::vector<std::string> & params)
    {
        /*
         * Parameters for this command:
         * 
         * 0. name of the portfolio
         * 1. document name
         */

        if(params.size() != 2)
        {
            throw SecwBadCommandArgumentException("Command needs at least 2 arguments");
        }

        const std::string & portfolioName = params[0];
        const std::string & name = params[1];

        //check global access
        std::set<UsageId> allowedUsageIds = m_activeWallet.getConfiguration(portfolioName).getUsageIdsForConsummer(sender);

        if( allowedUsageIds.size() == 0)
        {
            throw SecwIllegalAccess("You do not have access to this document");
        }

         DocumentPtr doc  = m_activeWallet.getPortfolio(portfolioName).getDocumentByName(name);

        if(!hasCommonUsageIds(allowedUsageIds, doc->getUsageIds()))
        {
            throw SecwIllegalAccess("You do not have access to this document");
        }

        cxxtools::SerializationInfo si;

        doc->fillSerializationInfoWithSecret(si);

        return serialize(si);
    }

    std::string SecurityWalletServer::handleGetDocumentWithoutSecretByName(const Sender & sender, const std::vector<std::string> & params)
    {
        /*
         * Parameters for this command:
         * 
         * 0. name of the portfolio
         * 1. document name
         */

        if(params.size() != 2)
        {
            throw SecwBadCommandArgumentException("Command needs at least 2 arguments");
        }

        const std::string & portfolioName = params[0];
        const std::string & name = params[1];

        DocumentPtr doc  = m_activeWallet.getPortfolio(portfolioName).getDocumentByName(name);

        cxxtools::SerializationInfo si;

        doc->fillSerializationInfoWithoutSecret(si);

        return serialize(si);
    }

    std::string SecurityWalletServer::handleGetListDocumentsWithSecret(const Sender & sender, const std::vector<std::string> & params)
    {
        /*
         * Parameters for this command:
         * 
         * 0. name of the portfolio
         * 1. Usage of documents (optional)
         */


        if(params.size() < 1)
        {
            throw SecwBadCommandArgumentException("Command needs at least argument");
        }

        const std::string & portfolioName = params[0];

        //check global access
        std::set<UsageId> allowedUsageIds = m_activeWallet.getConfiguration(portfolioName).getUsageIdsForConsummer(sender);

        if( allowedUsageIds.size() == 0)
        {
            throw SecwIllegalAccess("You do not have access to this command");
        }


        std::string debugInfo = "Do GetListDocumentsWithSecret on portfolio <"+portfolioName+">";

        UsageId usage = ""; //by default all the available usage for this consumer

        if(params.size() >= 2)
        {
            usage = params[1];
        }

        if(!usage.empty())
        {
            debugInfo+= " with the specific usage '"+usage+"'";
        }
        else
        {
            debugInfo += " without specific usage";
        }

        log_debug("%s",debugInfo.c_str());

        //prepare result
        std::string result;

        //check if the usage is accessible
        if(!usage.empty())
        {
            if(allowedUsageIds.count(usage) == 0)
            {
                throw SecwIllegalAccess("You do not have access to this command");
            } 

            result = serializeListDocumentsPrivate(portfolioName, {usage});
        }
        else
        {
            result = serializeListDocumentsPrivate(portfolioName, allowedUsageIds);
        }

        return result;
    }

    std::string SecurityWalletServer::handleGetListDocumentsWithoutSecret(const Sender & sender, const std::vector<std::string> & params)
    {
        /*
         * Parameters for this command:
         * 
         * 0. name of the portfolio
         * 1. Usage of documents (optional)
         */

        if(params.size() < 1)
        {
            throw SecwBadCommandArgumentException("Command needs at least argument");
        }

        const std::string & portfolioName = params[0];

        std::string debugInfo = "Do GetListDocumentsWithoutSecret on portfolio <"+portfolioName+">";

        UsageId usage = ""; //by default all the available usage for this consumer

        if(params.size() >= 2)
        {
            usage = params[1];
        }

        auto usageIDs = m_activeWallet.getConfiguration(portfolioName).getAllUsageId();
        if (!usage.empty() && usageIDs.find(usage) == usageIDs.end()) {
            throw SecwUnknownUsageIDException(usage);
        }

        if(!usage.empty())
        {
            debugInfo+= " with the specific usage '"+usage+"'";
        }
        else
        {
            debugInfo += " without specific usage";
        }

        log_debug("%s",debugInfo.c_str());

        //prepare request
        std::string result;

        //check if the usage we specify a usage
        if(!usage.empty())
        {   
            result = serializeListDocumentsPublic(portfolioName, {usage});
        }
        else
        {
            result = serializeListDocumentsPublic(portfolioName, {});
        }

        return result;
    }

    /* Notifications */

    void
    SecurityWalletServer::sendNotificationOnCreate (const std::string & portfolio, const DocumentPtr newDocument)
    {
        try
        {
            cxxtools::SerializationInfo rootSi;
            rootSi.addMember("action") <<= "CREATED";
            rootSi.addMember("portfolio") <<= portfolio;
            rootSi.addMember("old_data");
            newDocument->fillSerializationInfoWithoutSecret (rootSi.addMember("new_data"));

            m_streamPublisher.publish({serialize(rootSi)});
        }
        catch (const std::exception &e)
        {
            log_error ("Error while sending notification about document create: %s", e.what());
        }
        catch (...) {
            log_error ("Error while sending notification about document create: unknown error");
        }
    }

    void
    SecurityWalletServer::sendNotificationOnDelete (const std::string & portfolio, const DocumentPtr oldDocument)
    {
        try
        {
            cxxtools::SerializationInfo rootSi;
            rootSi.addMember("action") <<= "DELETED";
            rootSi.addMember("portfolio") <<= portfolio;
            rootSi.addMember("new_data");
            oldDocument->fillSerializationInfoWithoutSecret (rootSi.addMember("old_data"));

            m_streamPublisher.publish({serialize(rootSi)});
        }
        catch (const std::exception &e)
        {
            log_error ("Error while sending notification about document delete: %s", e.what());
        }
        catch (...) {
            log_error ("Error while sending notification about document delete: unknown error");
        }
    }

    void
    SecurityWalletServer::sendNotificationOnUpdate (const std::string & portfolio, const DocumentPtr oldDocument, const DocumentPtr newDocument)
    {
        try
        {
            cxxtools::SerializationInfo rootSi;
            rootSi.addMember("action") <<= "UPDATED";
            rootSi.addMember("portfolio") <<= portfolio;
            oldDocument->fillSerializationInfoWithoutSecret (rootSi.addMember("old_data"));
            newDocument->fillSerializationInfoWithoutSecret (rootSi.addMember("new_data"));

            m_streamPublisher.publish({serialize(rootSi)});
        }
        catch (const std::exception &e)
        {
            log_error ("Error while sending notification about document update: %s", e.what());
        }
        catch (...) {
            log_error ("Error while sending notification about document update: unknown error");
        }
    }

    std::string SecurityWalletServer::handleCreate(const Sender & sender, const std::vector<std::string> & params)
    {
        /*
         * Parameters for this command:
         * 
         * 0. name of the portfolio
         * 1. Document to be create
         */

        if(params.size() < 2)
        {
            throw SecwBadCommandArgumentException("Command need 2 arguments");
        }

        const std::string & portfolioName = params[0];
        const std::string & document = params[1];

        //check global access
        std::set<UsageId> allowedUsageIds = m_activeWallet.getConfiguration(portfolioName).getUsageIdsForProducer(sender);

        if(allowedUsageIds.size() == 0)
        {
            throw SecwIllegalAccess("You do not have access to this command");
        }

        log_debug("Do Create on portfolio <%s>", portfolioName.c_str());

        Portfolio & portfolio = m_activeWallet.getPortfolio(portfolioName);

        //deserialize
        const cxxtools::SerializationInfo si(deserialize(document));
        DocumentPtr doc;
        si >>= doc;

        //check if the document is valid
        doc->validate();

        //check if we are allow to insert
        for(const UsageId & usage : doc->getUsageIds())
        {
            //if on document usage do not belong to the user, reject the insert
            if(allowedUsageIds.count(usage) != 1)
            {
                throw SecwIllegalAccess("You do not have access to usage <"+usage+">");
            }
        }

        //prepare result => new id
        std::string newId = portfolio.add(doc);

        m_activeWallet.save();

        sendNotificationOnCreate(portfolioName, portfolio.getDocument(newId));
        return newId;
    }

    std::string SecurityWalletServer::handleDelete(const Sender & sender, const std::vector<std::string> & params)
    {
        /*
         * Parameters for this command:
         * 
         * 0. name of the portfolio
         * 1. id of Document to be delete
         */

        if(params.size() < 2)
        {
            throw SecwBadCommandArgumentException("Command need 2 arguments");
        }

        const std::string & portfolioName = params[0];
        const std::string & id = params[1];

        //check global access
        std::set<UsageId> allowedUsageIds = m_activeWallet.getConfiguration(portfolioName).getUsageIdsForProducer(sender);

        if(allowedUsageIds.size() == 0)
        {
            throw SecwIllegalAccess("You do not have access to this command");
        } 



        log_debug("Do Delete on portfolio <%s> for id <%s>", portfolioName.c_str(), id.c_str());

        Portfolio & portfolio = m_activeWallet.getPortfolio(portfolioName);

        //get the document
        DocumentPtr doc = portfolio.getDocument(id);

        //check if we are allow to remove
        for(const UsageId & usage : doc->getUsageIds())
        {
            //if on document usage do not belong to the user, reject the insert
            if(allowedUsageIds.count(usage) != 1)
            {
                throw SecwIllegalAccess("You do not have access to usage <"+usage+">");
            }
        }

        //remove and save
        portfolio.remove(id);

        m_activeWallet.save();

        sendNotificationOnDelete(portfolioName, doc);
        return "OK";
    }

    std::string SecurityWalletServer::handleUpdate(const Sender & sender, const std::vector<std::string> & params)
    {
        /*
         * Parameters for this command:
         * 
         * 0. name of the portfolio
         * 1. Document to be update
         */

        if(params.size() < 2)
        {
            throw SecwBadCommandArgumentException("Command need 2 arguments");
        }

        const std::string & portfolioName = params[0];
        const std::string & document = params[1];

        //check global access
        std::set<UsageId> allowedUsageIds = m_activeWallet.getConfiguration(portfolioName).getUsageIdsForProducer(sender);

        if(allowedUsageIds.size() == 0)
        {
            throw SecwIllegalAccess("You do not have access to this command");
        }



        log_debug("Do Update on portfolio <%s>", portfolioName.c_str());

        Portfolio & portfolio = m_activeWallet.getPortfolio(portfolioName);

        //de-serialize
        const cxxtools::SerializationInfo si(deserialize(document));
        DocumentPtr doc;
        si >>= doc;

        //std::cerr << "Received data:\n" << doc << std::endl;

        //recover the existing
        DocumentPtr copyOfExistingDoc = portfolio.getDocument(doc->getId());
        //clone the old document for notification purposes
        DocumentPtr docBeforeUpdate = copyOfExistingDoc->clone ();

        //std::cerr << "Existing data:\n" << copyOfExistingDoc << std::endl;

        //check that we can do this kind of update
        std::set<UsageId> diff = differenceBetween2UsagesIdSet(doc->getUsageIds(), copyOfExistingDoc->getUsageIds());

        for(const UsageId & usage : diff )
        {
            //if on document usage do not belong to the user, reject the update
            if(allowedUsageIds.count(usage) != 1)
            {
                throw SecwIllegalAccess("You do not have access to usage <"+usage+">");
            }
        }

        //override the copy of existing doc
        si >>= copyOfExistingDoc;

        //std::cerr << "Updated data:\n" << copyOfExistingDoc << std::endl;

        copyOfExistingDoc->validate();

        //do the update
        portfolio.update(copyOfExistingDoc);

        sendNotificationOnUpdate (portfolioName, docBeforeUpdate, doc);
        return "OK";
    }

    std::string SecurityWalletServer::serializeListDocumentsPublic(const std::string & portfolioName, const std::set<UsageId> & usages)
    {
        Portfolio & portfolio = m_activeWallet.getPortfolio(portfolioName);

        //get the documents
        cxxtools::SerializationInfo si;

        for (const auto pDoc : portfolio.getListDocuments() )
        {
            if((usages.empty() ) || (hasCommonUsageIds(pDoc->getUsageIds(), usages)))
            {
                pDoc->fillSerializationInfoWithoutSecret(si.addMember("")); 
            }
        }

        si.setCategory(cxxtools::SerializationInfo::Array);

        return serialize(si);

    }

    std::string SecurityWalletServer::serializeListDocumentsPrivate(const std::string & portfolioName, const std::set<UsageId> & usages)
    {
        Portfolio & portfolio = m_activeWallet.getPortfolio(portfolioName);

        //get the documents
        cxxtools::SerializationInfo si;

        for (const auto pDoc : portfolio.getListDocuments() )
        {
            if(hasCommonUsageIds(pDoc->getUsageIds(), usages))
            {
                pDoc->fillSerializationInfoWithSecret(si.addMember("")); 
            }
        }

        si.setCategory(cxxtools::SerializationInfo::Array);

        return serialize(si);
    }
} //namespace secw