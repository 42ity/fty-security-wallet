#include "secw_consumer_accessor.h"
#include "secw_exception.h"
#include "consumer_accessor.h"
#include <catch2/catch.hpp>

void secwConsumerAccessorTest(fty::SocketSyncClient& syncClient, mlm::MlmStreamClient& streamClient)
{
    // test 1.1 => getPortfolioList
    {
        secw::ConsumerAccessor consumerAccessor(syncClient, streamClient);
        try {
            std::vector<std::string> portfolio = consumerAccessor.getPortfolioList();

            if (portfolio.at(0) != "default") {
                FAIL("Portfolio default is not in the list of portfolio");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 1.2  => SecwUnknownPortfolioException
    {
        std::string            portfolioName("XXXXX");
        secw::ConsumerAccessor consumerAccessor(syncClient, streamClient);

        try {
            consumerAccessor.getDocumentWithPrivateData(portfolioName, "XXXXX-XXXXXXXXX");
            throw std::runtime_error("Document is returned");
        } catch (const secw::SecwUnknownPortfolioException& e) {
            CHECK(e.getPortfolioName() == portfolioName);
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 2.1 => getConsumerUsages
    {
        secw::ConsumerAccessor consumerAccessor(syncClient, streamClient);
        try {
            std::set<std::string> usages = consumerAccessor.getConsumerUsages();

            if (usages.size() != 1) {
                FAIL("Wrong number of customer use returned");
            }

            if (usages.count("discovery_monitoring") == 0) {
                FAIL("Usage 'discovery_monitoring' is not in the list of consumer usages");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 3.1 => getListDocumentsWithPrivateData
    {
        secw::ConsumerAccessor consumerAccessor(syncClient, streamClient);
        try {
            std::vector<secw::DocumentPtr> doc = consumerAccessor.getListDocumentsWithPrivateData("default");

            if (doc.size() != 1) {
                FAIL("Not the good number of documents: expected 1, received " + std::to_string(doc.size()));
            }

            if (!doc[0]->isContainingPrivateData()) {
                FAIL("Document is not containing private data");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 3.2 => getListDocumentsWithPrivateData usage="discovery_monitoring"
    {
        secw::ConsumerAccessor consumerAccessor(syncClient, streamClient);
        try {
            std::vector<secw::DocumentPtr> doc =
                consumerAccessor.getListDocumentsWithPrivateData("default", "discovery_monitoring");

            if (doc.size() != 1) {
                FAIL("Not the good number of documents: expected 1, received " + std::to_string(doc.size()));
            }

            if (!doc[0]->isContainingPrivateData()) {
                FAIL("Document is not containing private data");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 3.3  => getListDocumentsWithPrivateData SecwIllegalAccess
    {
        secw::ConsumerAccessor consumerAccessor(syncClient, streamClient);
        try {
            consumerAccessor.getListDocumentsWithPrivateData("default", "mass_device_management");

            throw std::runtime_error("Documents are returned");
        } catch (const secw::SecwIllegalAccess& ) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 4.1 => getDocumentWithPrivateData
    {
        secw::ConsumerAccessor consumerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr doc = consumerAccessor.getDocumentWithPrivateData("default", "id_readable");

            if (!doc->isContainingPrivateData()) {
                FAIL("Document is not containing private data");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 4.2 => getDocumentWithPrivateData => SecwDocumentDoNotExistException
    {
        secw::ConsumerAccessor consumerAccessor(syncClient, streamClient);
        try {
            consumerAccessor.getDocumentWithPrivateData("default", "XXXXX-XXXXXXXXX");

            throw std::runtime_error("Document is return");
        } catch (const secw::SecwDocumentDoNotExistException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 4.3 => getDocumentWithPrivateData
    {
        secw::ConsumerAccessor consumerAccessor(syncClient, streamClient);
        try {
            secw::DocumentPtr doc = consumerAccessor.getDocumentWithPrivateDataByName("default", "myFirstDoc");

            if (!doc->isContainingPrivateData()) {
                FAIL("Document is not containing private data");
            }
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }

    // test 4.4 => getDocumentWithPrivateDataByName => SecwNameDoesNotExistException
    {
        secw::ConsumerAccessor consumerAccessor(syncClient, streamClient);
        try {
            consumerAccessor.getDocumentWithPrivateDataByName("default", "XXXXX-XXXXXXXXX");

            throw std::runtime_error("Document is return");
        } catch (const secw::SecwNameDoesNotExistException&) {
        } catch (const std::exception& e) {
            FAIL(e.what());
        }
    }
}
