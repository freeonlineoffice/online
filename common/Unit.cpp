/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "Unit.hpp"

#include <cassert>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

#include <sysexits.h>

#include <Poco/Util/LayeredConfiguration.h>
#include <Poco/Util/Application.h>

#include "Log.hpp"
#include "Util.hpp"
#include <test/testlog.hpp>

#include <common/JsonUtil.hpp>
#include <common/Message.hpp>
#include <common/SigUtil.hpp>
#include <common/StringVector.hpp>

std::atomic<UnitKit *>GlobalKit = nullptr;
std::atomic<UnitWSD *>GlobalWSD = nullptr;
std::atomic<UnitTool *>GlobalTool = nullptr;
UnitBase** UnitBase::GlobalArray = nullptr;
int UnitBase::GlobalIndex = -1;
char* UnitBase::UnitLibPath = nullptr;
void* UnitBase::DlHandle = nullptr;
UnitBase::TestOptions UnitBase::GlobalTestOptions;
UnitBase::TestResult UnitBase::GlobalResult = UnitBase::TestResult::Ok;

namespace
{
std::thread TimeoutThread;
std::mutex TimeoutThreadMutex;
std::condition_variable TimeoutConditionVariable;
bool KitWorkFinished = false;

} // namespace

/// Controls whether experimental features/behavior is enabled or not.
bool EnableExperimental = false;

void UnitBase::initTestSuiteOptions()
{
    static const char* TestOptions = getenv("LOOL_TEST_OPTIONS");
    if (TestOptions == nullptr)
        return;

    StringVector tokens = StringVector::tokenize(std::string(TestOptions), ':');

    for (const auto& token : tokens)
    {
        // Expect name=value pairs.
        const auto pair = Util::split(tokens.getParam(token), '=');

        // If there is no value, assume it's a filter string.
        if (pair.second.empty())
        {
            const std::string filter = Util::toLower(pair.first);
            LOG_INF("Setting the 'filter' test option to [" << filter << ']');
            GlobalTestOptions.setFilter(filter);
        }
        else if (pair.first == "keepgoing")
        {
            const bool keepgoing = pair.second == "1" || pair.second == "true";
            LOG_INF("Setting the 'keepgoing' test option to " << keepgoing);
            GlobalTestOptions.setKeepgoing(keepgoing);
        }
    }
}

void UnitBase::filter()
{
    const auto& filter = GlobalTestOptions.getFilter();
    for (; GlobalArray[GlobalIndex] != nullptr; ++GlobalIndex)
    {
        const std::string& name = GlobalArray[GlobalIndex]->getTestname();
        if (strstr(Util::toLower(name).c_str(), filter.c_str()))
            break;

        LOG_INF("Skipping test [" << name << "] per filter [" << filter << ']');
    }
}

void UnitBase::selfTest()
{
    assert(init(UnitType::Wsd, std::string()));
    assert(!UnitBase::get().isFinished());
    assert(!UnitWSD::get().isFinished());
    assert(GlobalArray);
    assert(GlobalIndex == 0);
    assert(&UnitBase::get() == GlobalArray[0]);
    delete GlobalArray[0];
    delete[] GlobalArray;
    GlobalArray = nullptr;
    GlobalIndex = -1;
    GlobalKit = nullptr;
    GlobalWSD = nullptr;
    GlobalTool = nullptr;

    assert(init(UnitType::Kit, std::string()));
    assert(!UnitBase::get().isFinished());
    assert(!UnitKit::get().isFinished());
    assert(GlobalArray);
    assert(GlobalIndex == 0);
    assert(&UnitBase::get() == GlobalArray[0]);
    delete GlobalArray[0];
    delete[] GlobalArray;
    GlobalArray = nullptr;
    GlobalIndex = -1;
    GlobalKit = nullptr;
    GlobalWSD = nullptr;
    GlobalTool = nullptr;
}

bool UnitBase::init([[maybe_unused]] UnitType type, [[maybe_unused]] const std::string& unitLibPath)
{
    if constexpr (!Util::isMobileApp())
        LOG_ASSERT(!get(type));
    else
    {
        // The LOOLWSD initialization is called in a loop on mobile, allow reuse
        if (get(type))
            return true;
    }

    LOG_ASSERT(GlobalArray == nullptr);
    LOG_ASSERT(GlobalIndex == -1);
    GlobalArray = nullptr;
    GlobalIndex = -1;
    GlobalKit = nullptr;
    GlobalWSD = nullptr;
    GlobalTool = nullptr;

    // Only in debug builds do we support tests.
#if ENABLE_DEBUG
    if (!unitLibPath.empty())
    {
        GlobalArray = linkAndCreateUnit(type, unitLibPath);
        if (GlobalArray == nullptr)
        {
            // Error is logged already.
            return false;
        }

        // For now enable full logging
        // FIXME: remove this when time sensitive WOPI
        // tests are fixed.
        Log::setDisabledAreas("");

        initTestSuiteOptions();

        // Filter tests.
        GlobalIndex = 0;
        filter();

        UnitBase* instance = GlobalArray[GlobalIndex];
        if (instance)
        {
            rememberInstance(type, instance);
            TST_LOG_NAME("UnitBase",
                         "Starting test #1: " << GlobalArray[GlobalIndex]->getTestname());
            instance->initialize();

            if (instance && type == UnitType::Kit)
            {
                std::unique_lock<std::mutex> lock(TimeoutThreadMutex);
                TimeoutThread = std::thread(
                    [instance]
                    {
                        Util::setThreadName("unit timeout");

                        std::unique_lock<std::mutex> lock2(TimeoutThreadMutex);
                        if (TimeoutConditionVariable.wait_for(lock2,
                                                              instance->_timeoutMilliSeconds,
                                                              [] { return KitWorkFinished; }))
                        {
                            LOG_DBG(instance->getTestname() << ": Unit test finished in time");
                        }
                        else
                        {
                            LOG_ERR(instance->getTestname() << ": Unit test timeout after "
                                                            << instance->_timeoutMilliSeconds);
                            instance->timeout();
                        }
                    });
            }

            return get(type) != nullptr;
        }
    }
#endif // ENABLE_DEBUG

    // Fallback.
    switch (type)
    {
        case UnitType::Wsd:
            rememberInstance(UnitType::Wsd, new UnitWSD("UnitWSD"));
            GlobalArray = new UnitBase* [2] { GlobalWSD, nullptr };
            GlobalIndex = 0;
            break;
        case UnitType::Kit:
            rememberInstance(UnitType::Kit, new UnitKit("UnitKit"));
            GlobalArray = new UnitBase* [2] { GlobalKit, nullptr };
            GlobalIndex = 0;
            break;
        case UnitType::Tool:
            rememberInstance(UnitType::Tool, new UnitTool("UnitTool"));
            GlobalArray = new UnitBase* [2] { GlobalTool, nullptr };
            GlobalIndex = 0;
            break;
        default:
            assert(false);
            break;
    }

    return get(type) != nullptr;
}

UnitBase* UnitBase::get(UnitType type)
{
    switch (type)
    {
    case UnitType::Wsd:
        return GlobalWSD;
        break;
    case UnitType::Kit:
        return GlobalKit;
        break;
    case UnitType::Tool:
        return GlobalTool;
        break;
    default:
        assert(false);
        break;
    }

    return nullptr;
}

void UnitBase::rememberInstance(UnitType type, UnitBase* instance)
{
    assert(instance->_type == type);

    assert(GlobalWSD == nullptr);
    assert(GlobalKit == nullptr);
    assert(GlobalTool == nullptr);

    switch (type)
    {
    case UnitType::Wsd:
        GlobalWSD = static_cast<UnitWSD*>(instance);
        break;
    case UnitType::Kit:
        GlobalKit = static_cast<UnitKit*>(instance);
        break;
    case UnitType::Tool:
        GlobalTool = static_cast<UnitTool*>(instance);
        break;
    default:
        assert(false);
        break;
    }
}

int UnitBase::uninit()
{
    // Only in debug builds do we support tests.
#if ENABLE_DEBUG
    TST_LOG_NAME("UnitBase", "Uninitializing unit-tests: "
                                 << (GlobalResult == TestResult::Ok ? "SUCCESS" : "FAILED"));

    if (GlobalArray)
    {
        // By default, this will check _setRetValue and copy _retValue to the arg.
        // But we call it to trigger overrides and to perform cleanups.
        int retValue = GlobalResult == TestResult::Ok ? EX_OK : EX_SOFTWARE;
        if (GlobalArray[GlobalIndex] != nullptr)
            GlobalArray[GlobalIndex]->returnValue(retValue);
        if (retValue)
            GlobalResult = TestResult::Failed;

        for (int i = 0; GlobalArray[i] != nullptr; ++i)
        {
            delete GlobalArray[i];
        }

        delete[] GlobalArray;
        GlobalArray = nullptr;
    }

    GlobalIndex = -1;

    free(UnitBase::UnitLibPath);
    UnitBase::UnitLibPath = nullptr;

    GlobalKit = nullptr;
    GlobalWSD = nullptr;
    GlobalTool = nullptr;

    // Close the DLL last, after deleting the test instances.
    closeUnit();

    return GlobalResult == TestResult::Ok ? EX_OK : EX_SOFTWARE;
#else // ENABLE_DEBUG
    return 0; // Always success in release.
#endif // !ENABLE_DEBUG
}

std::shared_ptr<SocketPoll> UnitBase::socketPoll()
{
    // We could be called from either a UnitWSD::DocBrokerDestroy (prisoner_poll)
    // or from UnitWSD::invokeTest() (loolwsd main).
    std::lock_guard<std::mutex> guard(_lockSocketPoll);
    if (!_socketPoll)
        _socketPoll = std::make_shared<SocketPoll>(getTestname());
    return _socketPoll;
}

void UnitKit::postFork()
{
    // Don't drag wakeup pipes into the new process.
    std::shared_ptr<SocketPoll> socketPoll = getSocketPoll();
    if (socketPoll)
        socketPoll->closeAllSockets();
}

void UnitBase::initialize()
{
    assert(DlHandle != nullptr && "Invalid handle to set");
    TST_LOG("==================== Starting [" << getTestname() << "] ====================");
    socketPoll()->startThread();
}

void UnitBase::setTimeout(std::chrono::milliseconds timeoutMilliSeconds)
{
    assert(!TimeoutThread.joinable() && "setTimeout must be called before starting a test");
    _timeoutMilliSeconds = timeoutMilliSeconds;
    TST_LOG(getTestname() << ": setTimeout: " << _timeoutMilliSeconds);
}

UnitBase::~UnitBase()
{
    TST_LOG(getTestname() << ": ~UnitBase: " << (failed() ? "FAILED" : "SUCCESS"));

    std::shared_ptr<SocketPoll> socketPoll = getSocketPoll();
    if (socketPoll)
        socketPoll->joinThread();
}

bool UnitBase::filterLOKitMessage(const std::shared_ptr<Message>& message)
{
    return onFilterLOKitMessage(message);
}

bool UnitBase::filterSendWebSocketMessage(const char* data, const std::size_t len,
                                          const WSOpCode code, const bool flush, int& unitReturn)
{
    const std::string message(data, len);
    if (message.starts_with("unocommandresult:"))
    {
        const std::size_t index = message.find_first_of('{');
        if (index != std::string::npos)
        {
            try
            {
                const std::string stringJSON = message.substr(index);
                Poco::JSON::Parser parser;
                const Poco::Dynamic::Var parsedJSON = parser.parse(stringJSON);
                const auto& object = parsedJSON.extract<Poco::JSON::Object::Ptr>();
                if (object->get("commandName").toString() == ".uno:Save")
                {
                    const bool success = object->get("success").toString() == "true";
                    std::string result;
                    if (object->has("result"))
                    {
                        const Poco::Dynamic::Var parsedResultJSON = object->get("result");
                        const auto& resultObj = parsedResultJSON.extract<Poco::JSON::Object::Ptr>();
                        if (resultObj->get("type").toString() == "string")
                            result = resultObj->get("value").toString();
                    }

                    if (onDocumentSaved(message, success, result))
                        return false;
                }
            }
            catch (const std::exception& exception)
            {
                TST_LOG("unocommandresult parsing failure: " << exception.what());
            }
        }
        else
        {
            TST_LOG("Expected json unocommandresult. Ignoring: " << message);
        }
    }
    else if (message.starts_with("loaded:"))
    {
        if (message.find("isfirst=true") != std::string::npos)
        {
            // The Document loaded.
            if (onDocumentLoaded(message))
                return false;
        }

        // A view loaded.
        if (onViewLoaded(message))
            return false;
    }
    else if (message.starts_with("unloaded:"))
    {
        // A view unloaded.
        if (onViewUnloaded(message))
            return false;
    }
    else if (message == "statechanged: .uno:ModifiedStatus=true")
    {
        if (onDocumentModified(message))
            return false;
    }
    else if (message == "statechanged: .uno:ModifiedStatus=false")
    {
        if (onDocumentUnmodified(message))
            return false;
    }
    else if (message.starts_with("statechanged:"))
    {
        if (onDocumentStateChanged(message))
            return false;
    }
    else if (message.starts_with("error:"))
    {
        if (onDocumentError(message))
            return false;
    }

    return onFilterSendWebSocketMessage(data, len, code, flush, unitReturn);
}

void UnitBase::exitTest(TestResult result, const std::string& reason)
{
    // We could be called from either a SocketPoll (websrv_poll)
    // or from invokeTest (loolwsd main).
    std::lock_guard<std::mutex> guard(_lock);

    if (isFinished())
    {
        if (result != _result)
        {
            TST_LOG("exitTest got " << name(result) << " but is already finished with "
                                    << name(_result));
        }

        return;
    }

    if (result == TestResult::Ok)
    {
        TST_LOG("SUCCESS: exitTest: " << name(result) << (reason.empty() ? "" : ": " + reason));
    }
    else
    {
        TST_LOG("ERROR: FAILURE: exitTest: " << name(result)
                                             << (reason.empty() ? "" : ": " + reason));

        if (GlobalResult == TestResult::Ok)
            GlobalResult = result;
        SigUtil::triggerDumpState(__func__);
    }

    _result = result;
    _reason = reason;
    _setRetValue = true;

    // the kit needs to send a 'unitresult:' message to wsd to exit there.
    if (_type == UnitType::Kit)
        SocketPoll::wakeupWorld();

    else // otherwise exit.
    {
        endTest(reason);

        // Notify inheritors.
        onExitTest(result, reason);
    }
}

std::string UnitBase::getReason() const
{
    std::lock_guard<std::mutex> guard(_lock);
    return _reason;
}

std::string UnitKit::getResultMessage() const
{
    assert(isFinished());
    return std::string("unitresult: ") + std::string(nameShort(_result)) + " " + getReason();
}

void UnitWSD::processUnitResult(const StringVector &tokens)
{
    UnitBase::TestResult result = UnitBase::TestResult::TimedOut;
    TST_LOG("Received " << tokens[0] << " from kit:" << tokens[1] << " " << tokens[2]);
    assert (tokens[0] == "unitresult:");
    if (tokens[1] == "Ok")
        result = UnitBase::TestResult::Ok;
    else if (tokens[1] == "Failed")
        result = UnitBase::TestResult::Failed;
    exitTest(result, tokens[2]);
}

void UnitBase::timeout()
{
    // Don't timeout if we had already finished.
    if (isUnitTesting() && !isFinished())
    {
        TST_LOG("ERROR: Timed out waiting for unit test to complete within "
                << _timeoutMilliSeconds);
        exitTest(TestResult::TimedOut);
    }
}

void UnitBase::returnValue(int& retValue)
{
    if (_setRetValue)
        retValue = (_result == TestResult::Ok ? EX_OK : EX_SOFTWARE);
}

void UnitBase::endTest([[maybe_unused]] const std::string& reason)
{
    TST_LOG("Ending test by stopping SocketPoll [" << getTestname() << "]: " << reason);
    std::shared_ptr<SocketPoll> socketPoll = getSocketPoll();
    if (socketPoll)
        socketPoll->joinThread();

    // tell the timeout thread that the work has finished
    KitWorkFinished = true;
    TimeoutConditionVariable.notify_all();
    if (TimeoutThread.joinable())
        TimeoutThread.join();

    TST_LOG("==================== Finished [" << getTestname() << "] ====================");
}

UnitWSD::UnitWSD(const std::string& name)
    : UnitBase(name, UnitType::Wsd)
    , _wsd(nullptr)
    , _hasKitHooks(false)
{
}

UnitWSD::~UnitWSD()
{
}

void UnitWSD::defaultConfigure(Poco::Util::LayeredConfiguration& config)
{
    // Force HTTP - helps stracing.
    config.setBool("ssl.enable", false);
    // Use http:// everywhere.
    config.setBool("ssl.termination", false);
    // Force console output - easier to debug.
    config.setBool("logging.file[@enable]", false);
}

void UnitWSD::lookupTile(int part, int mode, int width, int height, int tilePosX, int tilePosY,
                         int tileWidth, int tileHeight,
                         std::shared_ptr<TileData> &tile)
{
    if (isUnitTesting())
    {
        if (tile)
            onTileCacheHit(part, mode, width, height, tilePosX, tilePosY, tileWidth, tileHeight);
        else
            onTileCacheMiss(part, mode, width, height, tilePosX, tilePosY, tileWidth, tileHeight);
    }
}

void UnitWSD::DocBrokerDestroy(const std::string& key)
{
    if (isUnitTesting())
    {
        onDocBrokerDestroy(key);
        if (!isFinished())
        {
            // Not yet finished; don't start the next test just yet.
            return;
        }

        // We could be called from either a SocketPoll (websrv_poll)
        // or from invokeTest (loolwsd main).
        std::lock_guard<std::mutex> guard(_lock);

        // Check if we have more tests, but keep the current index if it's the last.
        if (haveMoreTests())
        {
            // We have more tests.
            ++GlobalIndex;
            filter();

            // Clear the shortcuts.
            GlobalKit = nullptr;
            GlobalWSD = nullptr;
            GlobalTool = nullptr;

            if (GlobalArray[GlobalIndex] != nullptr && !SigUtil::getShutdownRequestFlag() &&
                (_result == TestResult::Ok || GlobalTestOptions.getKeepgoing()))
            {
                rememberInstance(_type, GlobalArray[GlobalIndex]);

                TST_LOG("Starting test #" << GlobalIndex + 1 << ": "
                                          << GlobalArray[GlobalIndex]->getTestname());
                UnitWSD *globalWSD = GlobalWSD;
                if (globalWSD)
                    globalWSD->configure(Poco::Util::Application::instance().config());
                GlobalArray[GlobalIndex]->initialize();
            }

            // Wake-up so the previous test stops.
            SocketPoll::wakeupWorld();
        }
    }
}

UnitWSD& UnitWSD::get()
{
    UnitWSD *globalWSD = GlobalWSD;
    assert(globalWSD);
    return *globalWSD;
}

void UnitWSD::onExitTest(TestResult result, const std::string&)
{
    if (haveMoreTests())
    {
        if (result != TestResult::Ok && !GlobalTestOptions.getKeepgoing())
        {
            TST_LOG("Failing fast per options, even though there are more tests");
            if constexpr (!Util::isMobileApp())
            {
                TST_LOG("Setting TerminationFlag as the Test Suite failed");
                SigUtil::setTerminationFlag(); // and wake-up world.
            }
            else
                SocketPoll::wakeupWorld();
            return;
        }

        TST_LOG("Have more tests. Waiting for the DocBroker to destroy before starting them");
        return;
    }

    // We are done with all the tests.
    TST_LOG_NAME("UnitBase", getTestname()
                                 << " was the last test. Finishing "
                                 << (GlobalResult == TestResult::Ok ? "SUCCESS" : "FAILED"));

    if constexpr (!Util::isMobileApp())
    {
        TST_LOG("Setting TerminationFlag as there are no more tests");
        SigUtil::setTerminationFlag(); // and wake-up world.
    }
    else
        SocketPoll::wakeupWorld();
}

UnitKit::UnitKit(const std::string& name)
    : UnitBase(name, UnitType::Kit)
{
}

UnitKit::~UnitKit() {}

UnitKit& UnitKit::get()
{
    if (Util::isKitInProcess() && !GlobalKit)
        GlobalKit = new UnitKit("UnitKit");

    UnitKit *globalKit = GlobalKit;
    assert(globalKit);
    return *globalKit;
}

void UnitKit::onExitTest(TestResult, const std::string&)
{
    // loolforkit doesn't link with CPPUnit.
    // LOK_ASSERT_MESSAGE("UnitKit doesn't yet support multiple tests", !haveMoreTests());

    // // We are done with all the tests.
    // TST_LOG_NAME("UnitBase", getTestname()
    //                              << " was the last test. Finishing "
    //                              << (GlobalResult == TestResult::Ok ? "SUCCESS" : "FAILED"));

    if constexpr (!Util::isMobileApp())
    {
        // TST_LOG("Setting TerminationFlag as there are no more tests");
        SigUtil::setTerminationFlag(); // and wake-up world.
    }
    else
        SocketPoll::wakeupWorld();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
