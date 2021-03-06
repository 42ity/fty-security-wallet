#define TEST_TIMEOUT 5
#include <fty_log.h>
#include <fty_security_wallet.h>
#include <catch2/catch.hpp>
#include <mutex>
#include <condition_variable>

using namespace std::placeholders;

// callback for test
secw::DocumentPtr g_newDoc;
secw::DocumentPtr g_oldDoc;
std::string       g_portfolio;
std::string       g_action;
bool              g_nonSecretChanged, g_secretChanged;

void callbackCreate(
    const std::string& portfolio, secw::DocumentPtr newDoc, std::mutex* mut, std::condition_variable* condvar)
{
    log_debug("callback CREATED");
    std::unique_lock<std::mutex> lock(*mut);
    g_action    = "CREATED";
    g_portfolio = portfolio;
    g_newDoc    = newDoc->clone();
    condvar->notify_all();
}

void callbackUpdated(const std::string& portfolio, secw::DocumentPtr oldDoc, secw::DocumentPtr newDoc,
    bool nonSecretChanged, bool secretChanged, std::mutex* mut, std::condition_variable* condvar)
{
    log_debug("callback UPDATED");
    std::unique_lock<std::mutex> lock(*mut);
    g_action           = "UPDATED";
    g_portfolio        = portfolio;
    g_nonSecretChanged = nonSecretChanged;
    g_secretChanged    = secretChanged;
    g_newDoc           = newDoc->clone();
    g_oldDoc           = oldDoc->clone();
    condvar->notify_all();
}

void callbackDeleted(
    const std::string& portfolio, secw::DocumentPtr oldDoc, std::mutex* mut, std::condition_variable* condvar)
{
    log_debug("callback DELETED");
    std::unique_lock<std::mutex> lock(*mut);
    g_action    = "DELETED";
    g_portfolio = portfolio;
    g_oldDoc    = oldDoc->clone();
    condvar->notify_all();
}

/*void callbackStarted() Cannot be tested here
{
  std::unique_lock<std::mutex> lock(mut);
  g_action = "STARTED";
  g_condvar.notify_all();
}*/

void secwProducerAccessorTest(fty::SocketSyncClient& syncClient, mlm::MlmStreamClient& streamClient)
{
    std::mutex              g_lock;
    std::condition_variable g_condvar;

    std::string testNumber;
    std::string testName;


    // test 1.1 => getPortfolioList
    {
        printf(" *=>  Test #1.1 getPortfolioList\n");
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            std::vector<std::string> portfolio = producerAccessor.getPortfolioList();

            if (portfolio.at(0) != "default") {
                throw std::runtime_error("Portfolio default is not in the list of portfolio");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 1.2  => SecwUnknownPortfolioException
    {
        std::string      portfolioName("XXXXX");
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            producerAccessor.getDocumentWithoutPrivateData(portfolioName, "XXXXX-XXXXXXXXX");

            throw std::runtime_error("Document is returned");
        } catch (const secw::SecwUnknownPortfolioException& e) {
            CHECK(e.getPortfolioName() == portfolioName);
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 2.1 => getProducerUsages
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            std::set<std::string> usages = producerAccessor.getProducerUsages();

            if (usages.size() != 2) {
                throw std::runtime_error("Wrong number of customer use returned");
            }

            if (usages.count("discovery_monitoring") == 0) {
                throw std::runtime_error("Usage 'discovery_monitoring' is not in the list of producer usages");
            }

            if (usages.count("mass_device_management") == 0) {
                throw std::runtime_error("Usage 'mass_device_management' is not in the list of producer usages");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 3.1 => getListDocumentsWithoutPrivateData
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            std::vector<secw::DocumentPtr> doc = producerAccessor.getListDocumentsWithoutPrivateData("default");

            if (doc.size() != 4) {
                throw std::runtime_error(
                    "Not the good number of documents: expected 4, received " + std::to_string(doc.size()));
            }

            if (doc[0]->isContainingPrivateData()) {
                throw std::runtime_error("Document is containing private data");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 3.2 => getListDocumentsWithoutPrivateData usage="discovery_monitoring"
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            std::vector<secw::DocumentPtr> doc =
                producerAccessor.getListDocumentsWithoutPrivateData("default", "discovery_monitoring");

            if (doc.size() != 1) {
                throw std::runtime_error(
                    "Not the good number of documents: expected 1, received " + std::to_string(doc.size()));
            }

            if (doc[0]->isContainingPrivateData()) {
                throw std::runtime_error("Document is containing private data");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 3.3 => getListDocumentsWithoutPrivateData with list of id
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            std::vector<secw::Id>          ids  = {"id_readable", "id_notReadable"};
            std::vector<secw::DocumentPtr> docs = producerAccessor.getListDocumentsWithoutPrivateData("default", ids);

            if (docs.size() != 2) {
                throw std::runtime_error("Bad number of documents returned");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 4.1 => getDocumentWithoutPrivateData
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr doc = producerAccessor.getDocumentWithoutPrivateData("default", "id_readable");

            if (doc->isContainingPrivateData()) {
                throw std::runtime_error("Document is containing private data");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 4.2 => getDocumentWithoutPrivateData => SecwDocumentDoNotExistException
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            producerAccessor.getDocumentWithoutPrivateData("default", "XXXXX-XXXXXXXXX");
            throw std::runtime_error("Document is return");
        } catch (const secw::SecwDocumentDoNotExistException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 4.3 => getDocumentWithoutPrivateData
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr doc = producerAccessor.getDocumentWithoutPrivateDataByName("default", "myFirstDoc");

            if (doc->isContainingPrivateData()) {
                throw std::runtime_error("Document is containing private data");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 4.4 => getDocumentWithoutPrivateDataByName => SecwNameDoesNotExistException
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            producerAccessor.getDocumentWithoutPrivateDataByName("default", "XXXXX-XXXXXXXXX");
            throw std::runtime_error("Document is return");
        } catch (const secw::SecwNameDoesNotExistException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    secw::Id id;
    // test 5.1 => insertNewDocument SNMPV3
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);

        // register the callback on create
        producerAccessor.setCallbackOnCreate(std::bind(callbackCreate, _1, _2, &g_lock, &g_condvar));

        try {
            secw::Snmpv3Ptr snmpv3Doc =
                std::make_shared<secw::Snmpv3>("Test insert snmpv3", secw::Snmpv3SecurityLevel::AUTH_PRIV, "test security name",
                    secw::Snmpv3AuthProtocol::MD5, "test auth password", secw::Snmpv3PrivProtocol::AES, "test priv password");

            snmpv3Doc->addUsage("discovery_monitoring");

            // lock to wait for the callback => the callback will notify this thread if called properly
            std::unique_lock<std::mutex> lock(g_lock);
            g_action    = "";
            g_portfolio = "";
            g_newDoc    = nullptr;

            id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<secw::Document>(snmpv3Doc));

            // wait for the callback to finish
            if (g_condvar.wait_for(lock, std::chrono::seconds(TEST_TIMEOUT)) == std::cv_status::timeout) {
                throw std::runtime_error("Timed out when waiting for callback to finish");
            }

            // check that the doc is inserted and the callback called
            if (g_action != "CREATED")
                throw std::runtime_error("Wrong action in the callback");
            if (g_portfolio != "default")
                throw std::runtime_error("Wrong portfolio in the created callback");
            if (g_newDoc == nullptr)
                throw std::runtime_error("No new data in the created callback");
            if (g_newDoc->getId() != id)
                throw std::runtime_error("Wrong id in the new document in the created callback");
            if (g_newDoc->getName() != "Test insert snmpv3")
                throw std::runtime_error("Wrong name in the new document in the created callback");

            secw::DocumentPtr snmpv3DocClone = snmpv3Doc->clone();

            if (!snmpv3Doc->isNonSecretEquals(snmpv3DocClone))
                throw std::runtime_error("Error in the comparaison of non secret of clone");
            if (!snmpv3Doc->isSecretEquals(snmpv3DocClone))
                throw std::runtime_error("Error in the comparaison of secret of clone");

            if (!g_newDoc->isNonSecretEquals(snmpv3Doc))
                throw std::runtime_error("Error in the comparaison of non secret of received element");
            if (g_newDoc->isSecretEquals(snmpv3Doc))
                throw std::runtime_error(
                    "Error in the comparaison of secret of received element => comparaison resturn true");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 5.2 => insertNewDocument -> retrieve data
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::Snmpv3Ptr snmpv3 = secw::Snmpv3::tryToCast(insertedDoc);

            if (snmpv3 == nullptr)
                throw std::runtime_error("No document retrieved");

            if (snmpv3->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (snmpv3->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

            if (snmpv3->getName() != "Test insert snmpv3")
                throw std::runtime_error("Bad document retrieved: name do not match");

            if (snmpv3->getSecurityName() != "test security name")
                throw std::runtime_error("Bad document retrieved: security name do not match");
            if (snmpv3->getSecurityLevel() != secw::Snmpv3SecurityLevel::AUTH_PRIV)
                throw std::runtime_error("Bad document retrieved: security level do not match");
            if (snmpv3->getAuthProtocol() != secw::Snmpv3AuthProtocol::MD5)
                throw std::runtime_error("Bad document retrieved: auth protocol do not match");
            if (snmpv3->getPrivProtocol() != secw::Snmpv3PrivProtocol::AES)
                throw std::runtime_error("Bad document retrieved: priv protocol do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 5.3 => updateDocument SNMPV3
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);

        // register the callback on update
        producerAccessor.setCallbackOnUpdate(std::bind(callbackUpdated, _1, _2, _3, _4, _5, &g_lock, &g_condvar));

        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::Snmpv3Ptr snmpv3 = secw::Snmpv3::tryToCast(insertedDoc);

            if (snmpv3 == nullptr)
                throw std::runtime_error("No document retrieved");

            // update security name and priv password
            snmpv3->setName("Test update snmpv3");
            snmpv3->setSecurityName("test update security snmpv3");
            snmpv3->setPrivPassword("new password");

            // lock to wait for the callback => the callback will notify this thread if called properly
            std::unique_lock<std::mutex> lock(g_lock);
            g_action           = "";
            g_portfolio        = "";
            g_oldDoc           = nullptr;
            g_newDoc           = nullptr;
            g_nonSecretChanged = false;
            g_secretChanged    = false;

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(snmpv3));

            // wait for the callback to finish
            if (g_condvar.wait_for(lock, std::chrono::seconds(TEST_TIMEOUT)) == std::cv_status::timeout) {
                throw std::runtime_error("Timed out when waiting for callback to finish");
            }

            // check that the doc is inserted and the callback called
            if (g_action != "UPDATED")
                throw std::runtime_error("Wrong action in the callback");
            if (g_portfolio != "default")
                throw std::runtime_error("Wrong portfolio in the updated callback");
            if (g_oldDoc == nullptr)
                throw std::runtime_error("No old data in the updated callback");
            if (g_oldDoc->getId() != id)
                throw std::runtime_error("Wrong id in the old document in the updated callback");
            if (g_oldDoc->getName() != "Test insert snmpv3")
                throw std::runtime_error("Wrong name in the old document in the updated callback");
            if (g_newDoc == nullptr)
                throw std::runtime_error("No new data in the updated callback");
            if (g_newDoc->getId() != id)
                throw std::runtime_error("Wrong id in the new document in the updated callback");
            if (g_newDoc->getName() != "Test update snmpv3")
                throw std::runtime_error("Wrong name in the new document in the updated callback");

            if (!g_nonSecretChanged)
                throw std::runtime_error("Wrong value in g_nonSecretChanged, should be true");
            if (!g_secretChanged)
                throw std::runtime_error("Wrong value in g_secretChanged, should be true");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 5.4 => updateDocument SNMPV3 -> retrieve data
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::Snmpv3Ptr snmpv3 = secw::Snmpv3::tryToCast(insertedDoc);

            if (snmpv3 == nullptr)
                throw std::runtime_error("No document retrieved");

            if (snmpv3->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (snmpv3->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

            if (snmpv3->getName() != "Test update snmpv3")
                throw std::runtime_error("Bad document retrieved: name do not match: " + snmpv3->getName());

            if (snmpv3->getSecurityName() != "test update security snmpv3")
                throw std::runtime_error("Bad document retrieved: security name do not match");
            if (snmpv3->getSecurityLevel() != secw::Snmpv3SecurityLevel::AUTH_PRIV)
                throw std::runtime_error("Bad document retrieved: security level do not match");
            if (snmpv3->getAuthProtocol() != secw::Snmpv3AuthProtocol::MD5)
                throw std::runtime_error("Bad document retrieved: auth protocol do not match");
            if (snmpv3->getPrivProtocol() != secw::Snmpv3PrivProtocol::AES)
                throw std::runtime_error("Bad document retrieved: priv protocol do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }


    // test 5.5 => updateDocument SNMPV3 -> bad format
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);

        // register the callback on update
        producerAccessor.setCallbackOnUpdate(std::bind(callbackUpdated, _1, _2, _3, _4, _5, &g_lock, &g_condvar));

        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::Snmpv3Ptr snmpv3 = secw::Snmpv3::tryToCast(insertedDoc);

            if (snmpv3 == nullptr)
                throw std::runtime_error("No document retrieved");

            // update with wrong data
            snmpv3->setSecurityName("");

            g_action           = "";
            g_portfolio        = "";
            g_newDoc           = nullptr;
            g_nonSecretChanged = false;
            g_secretChanged    = false;


            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(snmpv3));

            if (g_action != "")
                throw std::runtime_error("Non-empty action");
            if (g_portfolio != "")
                throw std::runtime_error("Non-empty portfolio");
            if (g_newDoc != nullptr)
                throw std::runtime_error("Non-empty data");

            if (g_nonSecretChanged)
                throw std::runtime_error("Wrong value in g_nonSecretChanged, should be false");
            if (g_secretChanged)
                throw std::runtime_error("Wrong value in g_secretChanged, should be false");

            throw std::runtime_error("Document has been updated");
        } catch (const secw::SecwInvalidDocumentFormatException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 5.6 => deleteDocument SNMPV3
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);

        // register the callback on create
        producerAccessor.setCallbackOnDelete(std::bind(callbackDeleted, _1, _2, &g_lock, &g_condvar));

        try {
            // lock to wait for the callback => the callback will notify this thread if called properly
            std::unique_lock<std::mutex> lock(g_lock);
            g_action    = "";
            g_portfolio = "";
            g_oldDoc    = nullptr;

            producerAccessor.deleteDocument("default", id);

            // wait for the callback to finish
            if (g_condvar.wait_for(lock, std::chrono::seconds(TEST_TIMEOUT)) == std::cv_status::timeout) {
                throw std::runtime_error("Timed out when waiting for callback to finish");
            }

            // check that the doc is inserted and the callback called
            if (g_action != "DELETED")
                throw std::runtime_error("Wrong action in the callback");
            if (g_portfolio != "default")
                throw std::runtime_error("Wrong portfolio in the deleted callback");
            if (g_oldDoc == nullptr)
                throw std::runtime_error("No old data in the deleted callback");
            if (g_oldDoc->getId() != id)
                throw std::runtime_error("Wrong id in the old document in the deleted callback");
            if (g_oldDoc->getName() != "Test update snmpv3")
                throw std::runtime_error("Wrong name in the old document in the deleted callback");

            // check that the document is removed
            std::vector<secw::Id> ids = {id};
            if (producerAccessor.getListDocumentsWithoutPrivateData("default", ids).size() != 0) {
                throw std::runtime_error("Document is not removed");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 6.1 => insertNewDocument User and Password
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::UserAndPasswordPtr doc = std::make_shared<secw::UserAndPassword>("Test insert username", "username", "password");

            doc->addUsage("discovery_monitoring");

            id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 6.2 => insertNewDocument -> retrieve data
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::UserAndPasswordPtr doc = secw::UserAndPassword::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            if (doc->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (doc->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

            if (doc->getName() != "Test insert username")
                throw std::runtime_error("Bad document retrieved: name do not match");

            if (doc->getUsername() != "username")
                throw std::runtime_error("Bad document retrieved: username do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 6.3 => insertNewDocument User and Password
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::UserAndPasswordPtr doc = std::make_shared<secw::UserAndPassword>("Test insert username", "username", "password");

            doc->addUsage("discovery_monitoring");

            id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));

            throw std::runtime_error("Document has been added");
        } catch (const secw::SecwNameAlreadyExistsException& e) {
            CHECK(e.getName() == "Test insert username");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 6.4 => updateDocument User and Password
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::UserAndPasswordPtr doc = secw::UserAndPassword::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            // update security name and priv password
            doc->setName("Test update username");
            doc->setUsername("new_username");
            doc->setPassword("new_password");

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 6.6 => updateDocument User and Password -> retrieve data
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::UserAndPasswordPtr doc = secw::UserAndPassword::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            if (doc->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (doc->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");


            if (doc->getName() != "Test update username")
                throw std::runtime_error("Bad document retrieved: name do not match");
            if (doc->getUsername() != "new_username")
                throw std::runtime_error("Bad document retrieved: username do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 6.7 => updateDocument User and Password -> retrieve data getDocumentWithoutPrivateDataByName
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc =
                producerAccessor.getDocumentWithoutPrivateDataByName("default", "Test update username");

            secw::UserAndPasswordPtr doc = secw::UserAndPassword::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            if (doc->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (doc->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");


            if (doc->getId() != id)
                throw std::runtime_error("Bad document retrieved: id do not match");
            if (doc->getUsername() != "new_username")
                throw std::runtime_error("Bad document retrieved: username do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 6.8 => updateDocument User and Password -> getDocumentWithoutPrivateDataByName =>
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            producerAccessor.getDocumentWithoutPrivateDataByName("default", "Test insert username");

            throw std::runtime_error("Document is return");
        } catch (const secw::SecwNameDoesNotExistException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 6.9 => updateDocument User and Password
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::UserAndPasswordPtr doc = secw::UserAndPassword::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            // update name
            doc->setName("myFirstDoc");

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));

            throw std::runtime_error("Document has been updated");
        } catch (const secw::SecwNameAlreadyExistsException& e) {
            CHECK(e.getName() == "myFirstDoc");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 6.10 => updateDocument User and Password -> bad format
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::UserAndPasswordPtr doc = secw::UserAndPassword::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            // update with wrong data
            doc->setUsername("");

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));

            throw std::runtime_error("Document has been updated");
        } catch (const secw::SecwInvalidDocumentFormatException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 6.11 => deleteDocument User and Password
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            producerAccessor.deleteDocument("default", id);

            // check that the document is removed
            std::vector<secw::Id> ids = {id};
            if (producerAccessor.getListDocumentsWithoutPrivateData("default", ids).size() != 0) {
                throw std::runtime_error("Document is not removed");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 7.1 => insertNewDocument Snmpv1
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::Snmpv1Ptr doc = std::make_shared<secw::Snmpv1>("Test insert snmpv1", "community");

            doc->addUsage("discovery_monitoring");

            id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 7.2 => insertNewDocument Snmpv1 -> retrieve data
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::Snmpv1Ptr doc = secw::Snmpv1::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            if (doc->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (doc->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

            if (doc->getName() != "Test insert snmpv1")
                throw std::runtime_error("Bad document retrieved: name do not match");

            if (doc->getCommunityName() != "community")
                throw std::runtime_error("Bad document retrieved: community do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 7.3 => updateDocument Snmpv1
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::Snmpv1Ptr doc = secw::Snmpv1::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            // update
            doc->setName("Test update snmpv1");
            doc->setCommunityName("new_community");

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 7.4 => updateDocument Snmpv1 -> retrieve data
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::Snmpv1Ptr doc = secw::Snmpv1::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            if (doc->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (doc->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");


            if (doc->getName() != "Test update snmpv1")
                throw std::runtime_error("Bad document retrieved: name do not match");
            if (doc->getCommunityName() != "new_community")
                throw std::runtime_error("Bad document retrieved: community do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 7.5 => updateDocument Snmpv1 -> bad format
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::Snmpv1Ptr doc = secw::Snmpv1::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            // update with wrong data
            doc->setCommunityName("");

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));

            throw std::runtime_error("Document has been updated");
        } catch (const secw::SecwInvalidDocumentFormatException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 7.6 => deleteDocument Snmpv1
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            producerAccessor.deleteDocument("default", id);

            // check that the document is removed
            std::vector<secw::Id> ids = {id};
            if (producerAccessor.getListDocumentsWithoutPrivateData("default", ids).size() != 0) {
                throw std::runtime_error("Document is not removed");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    std::string document;
    // test 8.1 => test serialization in string of doc
    {
        try {
            secw::Snmpv3Ptr snmpv3Doc =
                std::make_shared<secw::Snmpv3>("Test insert snmpv3", secw::Snmpv3SecurityLevel::AUTH_PRIV, "test security name",
                    secw::Snmpv3AuthProtocol::MD5, "test auth password", secw::Snmpv3PrivProtocol::AES, "test priv password");

            snmpv3Doc->validate();

            document <<= snmpv3Doc;
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 8.2 => test deserialization of doc from string
    {
        try {
            secw::DocumentPtr doc;

            document >>= doc;

            doc->validate();
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    std::string cert1 =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIB2zCCAYWgAwIBAgIUN/HvX8YlwCFba7U0qW0BPgwcAHMwDQYJKoZIhvcNAQEL\n"
        "BQAwQjELMAkGA1UEBhMCRlIxFTATBgNVBAcMDERlZmF1bHQgQ2l0eTEcMBoGA1UE\n"
        "CgwTRGVmYXVsdCBDb21wYW55IEx0ZDAeFw0xOTEyMDMxODE1NDJaFw0yMDEyMDIx\n"
        "ODE1NDJaMEIxCzAJBgNVBAYTAkZSMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAa\n"
        "BgNVBAoME0RlZmF1bHQgQ29tcGFueSBMdGQwXDANBgkqhkiG9w0BAQEFAANLADBI\n"
        "AkEA1/UeazUNyuF0drHPcyzE17GIkuK2U5GkEQlpB3OcL1ngfvLUD014Wbzhn47G\n"
        "wKTggcqerU85veFJntMNEmZYpQIDAQABo1MwUTAdBgNVHQ4EFgQUcZ2Mbx7cybRC\n"
        "dyp9TK5YmZcstJ4wHwYDVR0jBBgwFoAUcZ2Mbx7cybRCdyp9TK5YmZcstJ4wDwYD\n"
        "VR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAANBAJ+Dl5PvMKBAkZ3ozMjbi01O\n"
        "7ARuj9IBQLsZ+AI9FpqJpc2GQxL+T6KK8Tdyra9V/ogcNZNsSoUsPI421dPKFp0=\n"
        "-----END CERTIFICATE-----\n";

    std::string key1 =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIBVAIBADANBgkqhkiG9w0BAQEFAASCAT4wggE6AgEAAkEA1/UeazUNyuF0drHP\n"
        "cyzE17GIkuK2U5GkEQlpB3OcL1ngfvLUD014Wbzhn47GwKTggcqerU85veFJntMN\n"
        "EmZYpQIDAQABAkEAqMsV84WMOj7t4LgqBUPAtzY0IVrCV59GNWq9hO1/7iFJJXvA\n"
        "g/wvLrJzXpWnuCKaHglAFEr3fUZYZhTTFlsoAQIhAPzpa0sjYUFfmj19tzpz33A6\n"
        "q1RSA7BMImulNKNRXDPBAiEA2pgvBr5wxtrxGBKaoAWNktGFQIiUH7uP0h5V4Zob\n"
        "TeUCIEe0klQCWu+jAGMQwqNS+PWj3LGSczNH0rZ8Z3kqdx7BAiAmTrcnCma/Io1P\n"
        "t6rrUi3ORfOBLK4wpXD91J0eTSBt1QIgA2zWS9eZzbksJKFeKBDxsY+gsamu93v7\n"
        "0sE2dZY61aM=\n"
        "-----END PRIVATE KEY-----\n";

    std::string cert2 =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIB2zCCAYWgAwIBAgIUUmvLdtGfhPkB3gQTiIoSZcsb5DIwDQYJKoZIhvcNAQEL\n"
        "BQAwQjELMAkGA1UEBhMCWFgxFTATBgNVBAcMDERlZmF1bHQgQ2l0eTEcMBoGA1UE\n"
        "CgwTRGVmYXVsdCBDb21wYW55IEx0ZDAeFw0xOTEyMDMxODE5NDRaFw0yMDEyMDIx\n"
        "ODE5NDRaMEIxCzAJBgNVBAYTAlhYMRUwEwYDVQQHDAxEZWZhdWx0IENpdHkxHDAa\n"
        "BgNVBAoME0RlZmF1bHQgQ29tcGFueSBMdGQwXDANBgkqhkiG9w0BAQEFAANLADBI\n"
        "AkEA089g+gpl/ILO62D8geGoHYhQaD6Ceh2ew7hfRa27wmIHA2o0TbZ/5hD+/mAX\n"
        "BT4YlXiP4eYldLr9XJ38/2PbQwIDAQABo1MwUTAdBgNVHQ4EFgQUAgbmUIAaiJ5N\n"
        "qooY6x5J7zAmdtMwHwYDVR0jBBgwFoAUAgbmUIAaiJ5NqooY6x5J7zAmdtMwDwYD\n"
        "VR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAANBAI+gZZIkmCs5KnjmgzdBuk+N\n"
        "AHxrP15dci7DCMBYNXQu8jKrg8M/SVKGZgjPx4O9KHnLHf8L/KNWIJRQNfo9AhA=\n"
        "-----END CERTIFICATE-----\n";

    std::string key2 =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIBVQIBADANBgkqhkiG9w0BAQEFAASCAT8wggE7AgEAAkEA089g+gpl/ILO62D8\n"
        "geGoHYhQaD6Ceh2ew7hfRa27wmIHA2o0TbZ/5hD+/mAXBT4YlXiP4eYldLr9XJ38\n"
        "/2PbQwIDAQABAkEAnfkKnBKLVBR/nWAdlAUobLZROh59E/Tph5IIRKC569PzWwWR\n"
        "iA1YHEqLYgppfnQRs+PiilZzV3Hxn140suMSMQIhAPC00HwcPoW3olr+rDSeKO8L\n"
        "QOW8yE3NrYqzL2BlZmerAiEA4USONGqat1vnwfripCwcj9pqPtLjmpsOraeQLunn\n"
        "YskCIANAHdSfOQTrPukFqWOogxr/RugTTY0nauGFm+0sUV6zAiAKgDaczOdfasJX\n"
        "8YkFHCVMs2LGgPApMdcyUyBOf4rQuQIhAKEd5J2xBmB38z5MjTH+azDSpyONDMWz\n"
        "Bsr0n+mn+QIQ\n"
        "-----END PRIVATE KEY-----\n";


    // test 9.1 => insertNewDocument External Certificate
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::ExternalCertificatePtr doc =
                std::make_shared<secw::ExternalCertificate>("Test insert external certificate", cert1);

            doc->addUsage("discovery_monitoring");

            id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 9.2 => insertNewDocument -> retrieve data
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::ExternalCertificatePtr doc = secw::ExternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            if (doc->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (doc->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

            if (doc->getName() != "Test insert external certificate")
                throw std::runtime_error("Bad document retrieved: name do not match");

            if (doc->getPem() != cert1)
                throw std::runtime_error("Bad document retrieved: pem do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 9.3 => insertNewDocument Certificate
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::ExternalCertificatePtr doc =
                std::make_shared<secw::ExternalCertificate>("Test insert external certificate", cert1);

            doc->addUsage("discovery_monitoring");

            id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));

            throw std::runtime_error("Document has been added");
        } catch (const secw::SecwNameAlreadyExistsException& e) {
            CHECK(e.getName() == "Test insert external certificate");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 9.4 => updateDocument external certificate
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::ExternalCertificatePtr doc = secw::ExternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            // update security name and priv password
            doc->setName("Test update external certificate");
            doc->setPem(cert2);

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 9.6 => updateDocument external certificate -> retrieve data
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::ExternalCertificatePtr doc = secw::ExternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            if (doc->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (doc->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");


            if (doc->getName() != "Test update external certificate")
                throw std::runtime_error("Bad document retrieved: name do not match");
            if (doc->getPem() != cert2)
                throw std::runtime_error("Bad document retrieved: pem do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 9.7 => updateDocument external certificate -> retrieve data getDocumentWithoutPrivateDataByName
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc =
                producerAccessor.getDocumentWithoutPrivateDataByName("default", "Test update external certificate");

            secw::ExternalCertificatePtr doc = secw::ExternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            if (doc->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (doc->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");


            if (doc->getId() != id)
                throw std::runtime_error("Bad document retrieved: id do not match");
            if (doc->getPem() != cert2)
                throw std::runtime_error("Bad document retrieved: pem do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 9.8 => updateDocument external certificate -> getDocumentWithoutPrivateDataByName =>
    // SecwNameDoesNotExistException
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            producerAccessor.getDocumentWithoutPrivateDataByName("default", "Test insert external certificate");

            throw std::runtime_error("Document is return");
        } catch (const secw::SecwNameDoesNotExistException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 9.9 => updateDocument external certificate
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::ExternalCertificatePtr doc = secw::ExternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            // update name
            doc->setName("myFirstDoc");

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));

            throw std::runtime_error("Document has been updated");
        } catch (const secw::SecwNameAlreadyExistsException& e) {
            CHECK(e.getName() == "myFirstDoc");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 9.10 => updateDocument external certificate -> bad format
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::ExternalCertificatePtr doc = secw::ExternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            // update with wrong data
            doc->setPem("TEST");

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));

            throw std::runtime_error("Document has been updated");
        } catch (const secw::SecwInvalidDocumentFormatException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 9.11 => deleteDocument external certificate
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            producerAccessor.deleteDocument("default", id);

            // check that the document is removed
            std::vector<secw::Id> ids = {id};
            if (producerAccessor.getListDocumentsWithoutPrivateData("default", ids).size() != 0) {
                throw std::runtime_error("Document is not removed");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 10.1 => insertNewDocument Internal Certificate
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::InternalCertificatePtr doc =
                std::make_shared<secw::InternalCertificate>("Test insert Internal Certificate", cert1, key1);

            doc->addUsage("discovery_monitoring");

            id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 10.2 => insertNewDocument -> retrieve data
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::InternalCertificatePtr doc = secw::InternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            if (doc->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (doc->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");

            if (doc->getName() != "Test insert Internal Certificate")
                throw std::runtime_error("Bad document retrieved: name do not match");

            if (doc->getPem() != cert1)
                throw std::runtime_error("Bad document retrieved: pem do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 10.3 => insertNewDocument Certificate
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::InternalCertificatePtr doc =
                std::make_shared<secw::InternalCertificate>("Test insert Internal Certificate", cert1, key1);

            doc->addUsage("discovery_monitoring");

            id = producerAccessor.insertNewDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));

            throw std::runtime_error("Document has been added");
        } catch (const secw::SecwNameAlreadyExistsException& e) {
            CHECK(e.getName() == "Test insert Internal Certificate");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 10.4 => updateDocument Internal Certificate
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::InternalCertificatePtr doc = secw::InternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            // update security name and priv password
            doc->setName("Test update Internal Certificate");
            doc->setPem(cert2);
            doc->setPrivateKeyPem(key2);

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 10.6 => updateDocument Internal Certificate -> retrieve data
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::InternalCertificatePtr doc = secw::InternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            if (doc->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (doc->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");


            if (doc->getName() != "Test update Internal Certificate")
                throw std::runtime_error("Bad document retrieved: name do not match");
            if (doc->getPem() != cert2)
                throw std::runtime_error("Bad document retrieved: pem do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 10.7 => updateDocument Internal Certificate -> retrieve data getDocumentWithoutPrivateDataByName
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc =
                producerAccessor.getDocumentWithoutPrivateDataByName("default", "Test update Internal Certificate");

            secw::InternalCertificatePtr doc = secw::InternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            if (doc->getUsageIds().count("discovery_monitoring") == 0)
                throw std::runtime_error("Bad document retrieved: bad usage, discovery_monitoring is missing");
            if (doc->getUsageIds().size() != 1)
                throw std::runtime_error("Bad document retrieved: bad usage, bad number of usages id");


            if (doc->getId() != id)
                throw std::runtime_error("Bad document retrieved: id do not match");
            if (doc->getPem() != cert2)
                throw std::runtime_error("Bad document retrieved: pem do not match");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 10.8 => updateDocument Internal Certificate -> getDocumentWithoutPrivateDataByName =>
    // SecwNameDoesNotExistException
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            producerAccessor.getDocumentWithoutPrivateDataByName("default", "Test insert Internal Certificate");

            throw std::runtime_error("Document is return");
        } catch (const secw::SecwNameDoesNotExistException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 10.9 => updateDocument Internal Certificate
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::InternalCertificatePtr doc = secw::InternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            // update name
            doc->setName("myFirstDoc");

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));

            throw std::runtime_error("Document has been updated");
        } catch (const secw::SecwNameAlreadyExistsException& e) {
            CHECK(e.getName() == "myFirstDoc");
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 10.10 => updateDocument Internal Certificate -> bad format
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr insertedDoc = producerAccessor.getDocumentWithoutPrivateData("default", id);

            secw::InternalCertificatePtr doc = secw::InternalCertificate::tryToCast(insertedDoc);

            if (doc == nullptr)
                throw std::runtime_error("No document retrieved");

            // update with wrong data
            doc->setPem(cert2);
            doc->setPrivateKeyPem(key1);

            // update
            producerAccessor.updateDocument("default", std::dynamic_pointer_cast<secw::Document>(doc));

            throw std::runtime_error("Document has been updated");
        } catch (const secw::SecwInvalidDocumentFormatException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 10.11 => deleteDocument Internal Certificate
    {
        secw::ProducerAccessor producerAccessor(syncClient, streamClient);
        try {
            producerAccessor.deleteDocument("default", id);

            // check that the document is removed
            std::vector<secw::Id> ids = {id};
            if (producerAccessor.getListDocumentsWithoutPrivateData("default", ids).size() != 0) {
                throw std::runtime_error("Document is not removed");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }
}
