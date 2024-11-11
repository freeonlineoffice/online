/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config.h"

#include "WopiTestServer.hpp"
#include "WOPIUploadConflictCommon.hpp"
#include "Unit.hpp"
#include "lokassert.hpp"
#include "testlog.hpp"
#include "FileUtil.hpp"

#include <Poco/Net/HTTPRequest.h>

/// This test simulates a permanently-failing upload.
class UnitQuarantineConflict : public WOPIUploadConflictCommon
{
    using Base = WOPIUploadConflictCommon;

    using Base::Phase;
    using Base::Scenario;

    using Base::OriginalDocContent;

    std::string _quarantinePath;
    bool _unloadingModifiedDocDetected;
    bool _putFailed;

    static constexpr std::size_t LimitStoreFailures = 2;
    static constexpr bool SaveOnExit = true;

public:
    UnitQuarantineConflict()
        : Base("UnitQuarantineConflict", OriginalDocContent)
        , _unloadingModifiedDocDetected(true)
        , _putFailed(false)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        // Small value to shorten the test run time.
        config.setUInt("per_document.limit_store_failures", LimitStoreFailures);
        config.setBool("per_document.always_save_on_exit", SaveOnExit);

        config.setBool("quarantine_files[@enable]", true);
        auto rootPath = Poco::Path(config.getString("child_root_path", ""));
        rootPath.popDirectory().pushDirectory("quarantine");
        _quarantinePath = FileUtil::createRandomTmpDir(rootPath.toString());
        LOG_TST("Quarantine path set to [" << _quarantinePath << ']');
        config.setString("quarantine_files.path", _quarantinePath);
    }

    void onDocBrokerCreate(const std::string& docKey) override
    {
        // reset for the next document
        _putFailed = false;

        Base::onDocBrokerCreate(docKey);

        if (_scenario == Scenario::VerifyOverwrite)
        {
            // By default, we don't upload when verifying (unless always_save_on_exit is set).
            setExpectedPutFile(SaveOnExit ? 2 : 0);
        }
        else
        {
            // With always_save_on_exit=true and limit_store_failures=LimitStoreFailures,
            // we expect exactly two PutFile requests per document.
            setExpectedPutFile(LimitStoreFailures);
            setExpectedCheckFileInfo(2); // Conflict recovery requires second CFI.
        }
    }

    std::unique_ptr<http::Response>
    assertGetFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        LOG_TST("Testing " << name(_scenario));
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        assertGetFileCount();

        //FIXME: check that unloading modified documents trigger test failure.
        // LOK_ASSERT_EQUAL_MESSAGE("Expected modified document detection to have triggered", true,
        //                          _unloadingModifiedDocDetected);
        _unloadingModifiedDocDetected = false; // Reset.

        return nullptr; // Success.
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        LOG_TST("Testing " << name(_scenario));
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        assertPutFileCount();

        const std::string wopiTimestamp = request.get("X-LOOL-WOPI-Timestamp", std::string());
        const bool force = wopiTimestamp.empty(); // Without a timestamp we force to always store.

        // We don't expect overwriting by forced uploading.
        LOK_ASSERT_EQUAL_MESSAGE("Unexpected overwritting the document in storage", _putFailed, force);

        _putFailed = true;

        // Internal Server Error.
        return std::make_unique<http::Response>(http::StatusCode::InternalServerError);
    }

    bool onDocumentModified(const std::string& message) override
    {
        LOG_TST("Testing " << name(_scenario) << ": [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitDocClose);

        switch (_scenario)
        {
            case Scenario::Disconnect:
                // Just disconnect.
                LOG_TST("Disconnecting");
                deleteSocketAt(0);
                break;
            case Scenario::SaveDiscard:
            case Scenario::SaveOverwrite:
                // Save the document.
                LOG_TST("Saving the document");
                WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");
                break;
            case Scenario::CloseDiscard:
                // Close the document.
                LOG_TST("Closing the document");
                WSD_CMD("closedocument");
                break;
            case Scenario::VerifyOverwrite:
                LOK_ASSERT_FAIL("Unexpected modification in " + toString(_scenario));
                break;
        }

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        LOG_TST("Testing " << name(_scenario) << ": [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        if (getCountCheckFileInfo() == 1)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expect only savefailed errors on first upload",
                                     std::string("error: cmd=storage kind=savefailed"), message);
        }
        else
        {
            // Once the first upload fails, we issue CheckFileInfo, which detects the conflict.
            LOK_ASSERT_EQUAL_MESSAGE(
                "Expect only documentconflict errors after the second CheckFileInfo",
                std::string("error: cmd=storage kind=documentconflict"), message);

            // Close the document.
            LOG_TST("Closing the document");
            WSD_CMD("closedocument");
        }

        return true;
    }

    // Called when we have modified document data at exit.
    bool onDataLoss(const std::string& reason) override
    {
        LOG_TST("Modified document being unloaded: " << reason);

        // We expect this to happen only with the disonnection test,
        // because only in that case there is no user input.
        LOK_ASSERT_MESSAGE("Expected reason to be 'Data-loss detected'",
                           reason.starts_with("Data-loss detected"));
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);
        _unloadingModifiedDocDetected = true;

        return failed();
    }

    // Wait for clean unloading.
    void onDocBrokerDestroy(const std::string& docKey) override
    {
        LOG_TST("Testing " << name(_scenario) << " with dockey [" << docKey << "] closed.");
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        // Uploading fails and we can't have anything but the original.
        LOK_ASSERT_EQUAL_MESSAGE("Unexpected contents in storage", std::string(OriginalDocContent),
                                 getFileContent());

        std::vector<std::string> files;
        Poco::File(_quarantinePath).list(files);

        LOK_ASSERT_MESSAGE("Expected 1 quaratined files in [" << _quarantinePath << ']',
                           files.size() == 1);

        Base::onDocBrokerDestroy(docKey);
    }
};

UnitBase* unit_create_wsd(void) { return new UnitQuarantineConflict(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
