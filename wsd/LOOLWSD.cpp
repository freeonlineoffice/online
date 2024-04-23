/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "LOOLWSD.hpp"
#if ENABLE_FEATURE_LOCK
#include "CommandControl.hpp"
#endif
#if !MOBILEAPP
#include <HostUtil.hpp>
#endif // !MOBILEAPP

/* Default host used in the start test URI */
#define LOOLWSD_TEST_HOST "localhost"

/* Default lool UI used in the admin console URI */
#define LOOLWSD_TEST_ADMIN_CONSOLE "/browser/dist/admin/admin.html"

/* Default lool UI used in for monitoring URI */
#define LOOLWSD_TEST_METRICS "/lool/getMetrics"

/* Default lool UI used in the start test URI */
#define LOOLWSD_TEST_LOOL_UI "/browser/" LOOLWSD_VERSION_HASH "/debug.html"

/* Default document used in the start test URI */
#define LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_WRITER  "test/data/hello-world.odt"
#define LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_CALC    "test/data/hello-world.ods"
#define LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_IMPRESS "test/data/hello-world.odp"
#define LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_DRAW    "test/data/hello-world.odg"

/* Default ciphers used, when not specified otherwise */
#define DEFAULT_CIPHER_SET "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"

// This is the main source for the loolwsd program. LOOL uses several loolwsd processes: one main
// parent process that listens on the TCP port and accepts connections from LOOL clients, and a
// number of child processes, each which handles a viewing (editing) session for one document.

#include <unistd.h>
#include <stdlib.h>
#include <sysexits.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include <cassert>
#include <clocale>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

#if !MOBILEAPP

#include <Poco/Net/Context.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/IPAddress.h>
#include <Poco/Net/MessageHeader.h>
#include <Poco/Net/NameValueCollection.h>
#include <Poco/Net/Net.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/PartHandler.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/SSLManager.h>

using Poco::Net::PartHandler;

#include <cerrno>
#include <stdexcept>
#include <fstream>
#include <unordered_map>

#include "Admin.hpp"
#include "Auth.hpp"
#include "FileServer.hpp"
#include "UserMessages.hpp"

#endif

#include <Poco/DateTimeFormatter.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/Exception.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/MemoryStream.h>
#include <Poco/Net/HostEntry.h>
#include <Poco/Path.h>
#include <Poco/TemporaryFile.h>
#include <Poco/URI.h>
#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/MapConfiguration.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionException.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/XMLConfiguration.h>

#include "ClientSession.hpp"
#include <ClientRequestDispatcher.hpp>
#include <Common.hpp>
#include <Clipboard.hpp>
#include <Crypto.hpp>
#include <DelaySocket.hpp>
#include "DocumentBroker.hpp"
#include <common/JsonUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/JailUtil.hpp>
#include <common/Watchdog.hpp>
#if MOBILEAPP
#  include <Kit.hpp>
#endif
#include <Log.hpp>
#include <MobileApp.hpp>
#include <Protocol.hpp>
#include <Session.hpp>
#if ENABLE_SSL
#  include <SslSocket.hpp>
#endif
#include "Storage.hpp"
#include "TraceFile.hpp"
#include <Unit.hpp>
#include <Util.hpp>
#include <common/ConfigUtil.hpp>
#include <common/TraceEvent.hpp>

#include <common/SigUtil.hpp>

#include <RequestVettingStation.hpp>
#include <ServerSocket.hpp>

#if MOBILEAPP
#ifdef IOS
#include "ios.h"
#elif defined(GTKAPP)
#include "gtk.hpp"
#elif defined(__ANDROID__)
#include "androidapp.hpp"
#elif WASMAPP
#include "wasmapp.hpp"
#endif
#endif // MOBILEAPP

#ifdef __linux__
#if !MOBILEAPP
#include <sys/inotify.h>
#endif
#endif

using namespace LOOLProtocol;

using Poco::DirectoryIterator;
using Poco::Exception;
using Poco::File;
using Poco::Path;
using Poco::URI;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::MessageHeader;
using Poco::Net::NameValueCollection;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::LayeredConfiguration;
using Poco::Util::MissingOptionException;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;
using Poco::Util::XMLConfiguration;

/// Port for external clients to connect to
int ClientPortNumber = 0;
/// Protocols to listen on
Socket::Type ClientPortProto = Socket::Type::All;

/// INET address to listen on
ServerSocket::Type ClientListenAddr = ServerSocket::Type::Public;

#if !MOBILEAPP
/// UDS address for kits to connect to.
std::string MasterLocation;

std::string LOOLWSD::LatestVersion;
std::mutex LOOLWSD::FetchUpdateMutex;
#endif

// Tracks the set of prisoners / children waiting to be used.
static std::mutex NewChildrenMutex;
static std::condition_variable NewChildrenCV;
static std::vector<std::shared_ptr<ChildProcess> > NewChildren;

static std::chrono::steady_clock::time_point LastForkRequestTime = std::chrono::steady_clock::now();
static std::atomic<int> OutstandingForks(0);
std::map<std::string, std::shared_ptr<DocumentBroker>> DocBrokers;
std::mutex DocBrokersMutex;
static Poco::AutoPtr<Poco::Util::XMLConfiguration> KitXmlConfig;

extern "C"
{
    void dump_state(void); /* easy for gdb */
    void forwardSigUsr2();
}

#if ENABLE_DEBUG && !MOBILEAPP
static std::chrono::milliseconds careerSpanMs(std::chrono::milliseconds::zero());
#endif

/// The timeout for a child to spawn, initially high, then reset to the default.
int ChildSpawnTimeoutMs = CHILD_TIMEOUT_MS * 4;
std::atomic<unsigned> LOOLWSD::NumConnections;
std::unordered_set<std::string> LOOLWSD::EditFileExtensions;
std::unordered_set<std::string> LOOLWSD::ViewWithCommentsFileExtensions;

#if MOBILEAPP

// Or can this be retrieved in some other way?
int LOOLWSD::prisonerServerSocketFD;

#else

/// Funky latency simulation basic delay (ms)
static std::size_t SimulatedLatencyMs = 0;

#endif

void LOOLWSD::appendAllowedHostsFrom(LayeredConfiguration& conf, const std::string& root, std::vector<std::string>& allowed)
{
    for (size_t i = 0; ; ++i)
    {
        const std::string path = root + ".host[" + std::to_string(i) + ']';
        if (!conf.has(path))
        {
            break;
        }
        const std::string host = getConfigValue<std::string>(conf, path, "");
        if (!host.empty())
        {
            LOG_INF_S("Adding trusted LOK_ALLOW host: [" << host << ']');
            allowed.push_back(host);
        }
    }
}

namespace {
std::string removeProtocolAndPort(const std::string& host)
{
    std::string result;

    // protocol
    size_t nPos = host.find("//");
    if (nPos != std::string::npos)
        result = host.substr(nPos + 2);
    else
        result = host;

    // port
    nPos = result.find(":");
    if (nPos != std::string::npos)
    {
        if (nPos == 0)
            return "";

        result = result.substr(0, nPos);
    }

    return result;
}

bool isValidRegex(const std::string& expression)
{
    try
    {
        std::regex regex(expression);
        return true;
    }
    catch (const std::regex_error& e) {}

    return false;
}
}

void LOOLWSD::appendAllowedAliasGroups(LayeredConfiguration& conf, std::vector<std::string>& allowed)
{
    for (size_t i = 0;; i++)
    {
        const std::string path = "storage.wopi.alias_groups.group[" + std::to_string(i) + ']';
        if (!conf.has(path + ".host"))
        {
            break;
        }

        std::string host = conf.getString(path + ".host", "");
        bool allow = conf.getBool(path + ".host[@allow]", false);
        if (!allow)
        {
            break;
        }

        host = removeProtocolAndPort(host);

        if (!host.empty())
        {
            LOG_INF_S("Adding trusted LOK_ALLOW host: [" << host << ']');
            allowed.push_back(host);
        }

        for (size_t j = 0;; j++)
        {
            const std::string aliasPath = path + ".alias[" + std::to_string(j) + ']';
            if (!conf.has(aliasPath))
            {
                break;
            }

            std::string alias = getConfigValue<std::string>(conf, aliasPath, "");

            alias = removeProtocolAndPort(alias);
            if (!alias.empty())
            {
                LOG_INF_S("Adding trusted LOK_ALLOW alias: [" << alias << ']');
                allowed.push_back(alias);
            }
        }
    }
}

/// Internal implementation to alert all clients
/// connected to any document.
void LOOLWSD::alertAllUsersInternal(const std::string& msg)
{
    if (Util::isMobileApp())
        return;
    std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);

    LOG_INF("Alerting all users: [" << msg << ']');

    if (UnitWSD::get().filterAlertAllusers(msg))
        return;

    for (auto& brokerIt : DocBrokers)
    {
        std::shared_ptr<DocumentBroker> docBroker = brokerIt.second;
        docBroker->addCallback([msg, docBroker](){ docBroker->alertAllUsers(msg); });
    }
}

void LOOLWSD::alertUserInternal(const std::string& dockey, const std::string& msg)
{
    if (Util::isMobileApp())
        return;
    std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);

    LOG_INF("Alerting document users with dockey: [" << dockey << ']' << " msg: [" << msg << ']');

    for (auto& brokerIt : DocBrokers)
    {
        std::shared_ptr<DocumentBroker> docBroker = brokerIt.second;
        if (docBroker->getDocKey() == dockey)
            docBroker->addCallback([msg, docBroker](){ docBroker->alertAllUsers(msg); });
    }
}

void LOOLWSD::writeTraceEventRecording(const char *data, std::size_t nbytes)
{
    static std::mutex traceEventFileMutex;

    std::unique_lock<std::mutex> lock(traceEventFileMutex);

    fwrite(data, nbytes, 1, LOOLWSD::TraceEventFile);
}

void LOOLWSD::writeTraceEventRecording(const std::string &recording)
{
    writeTraceEventRecording(recording.data(), recording.length());
}

void LOOLWSD::checkSessionLimitsAndWarnClients()
{
#if !MOBILEAPP
    if (config::isSupportKeyEnabled())
        return;

    ssize_t docBrokerCount = DocBrokers.size() - ConvertToBroker::getInstanceCount();
    if (LOOLWSD::MaxDocuments < 10000 &&
        (docBrokerCount > static_cast<ssize_t>(LOOLWSD::MaxDocuments) || LOOLWSD::NumConnections >= LOOLWSD::MaxConnections))
    {
        const std::string info = Poco::format(PAYLOAD_INFO_LIMIT_REACHED, LOOLWSD::MaxDocuments, LOOLWSD::MaxConnections);
        LOG_INF("Sending client 'limitreached' message: " << info);

        try
        {
            Util::alertAllUsers(info);
        }
        catch (const std::exception& ex)
        {
            LOG_ERR("Error while shutting down socket on reaching limit: " << ex.what());
        }
    }
#endif
}

void LOOLWSD::checkDiskSpaceAndWarnClients(const bool cacheLastCheck)
{
#if !MOBILEAPP
    try
    {
        const std::string fs = FileUtil::checkDiskSpaceOnRegisteredFileSystems(cacheLastCheck);
        if (!fs.empty())
        {
            LOG_WRN("File system of [" << fs << "] is dangerously low on disk space.");
            LOOLWSD::alertAllUsersInternal("error: cmd=internal kind=diskfull");
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Exception while checking disk-space and warning clients: " << exc.what());
    }
#else
    (void) cacheLastCheck;
#endif
}

/// Remove dead and idle DocBrokers.
/// The client of idle document should've greyed-out long ago.
void cleanupDocBrokers()
{
    Util::assertIsLocked(DocBrokersMutex);

    const size_t count = DocBrokers.size();
    for (auto it = DocBrokers.begin(); it != DocBrokers.end(); )
    {
        std::shared_ptr<DocumentBroker> docBroker = it->second;

        // Remove only when not alive.
        if (!docBroker->isAlive())
        {
            LOG_INF("Removing DocumentBroker for docKey [" << it->first << "].");
            docBroker->dispose();
            it = DocBrokers.erase(it);
            continue;
        } else {
            ++it;
        }
    }

    if (count != DocBrokers.size())
    {
        LOG_TRC("Have " << DocBrokers.size() << " DocBrokers after cleanup.\n"
                        <<
                [&](auto& log)
                {
                    for (auto& pair : DocBrokers)
                    {
                        log << "DocumentBroker [" << pair.first << "].\n";
                    }
                });

#if !MOBILEAPP && ENABLE_DEBUG
        if (LOOLWSD::SingleKit && DocBrokers.empty())
        {
            LOG_DBG("Setting ShutdownRequestFlag: No more docs left in single-kit mode.");
            SigUtil::requestShutdown();
        }
#endif
    }
}

#if !MOBILEAPP

/// Forks as many children as requested.
/// Returns the number of children requested to spawn,
/// -1 for error.
static int forkChildren(const int number)
{
    if (Util::isKitInProcess())
        return 0;

    LOG_TRC("Request forkit to spawn " << number << " new child(ren)");
    Util::assertIsLocked(NewChildrenMutex);

    if (number > 0)
    {
        LOOLWSD::checkDiskSpaceAndWarnClients(false);

        const std::string aMessage = "spawn " + std::to_string(number) + '\n';
        LOG_DBG("MasterToForKit: " << aMessage.substr(0, aMessage.length() - 1));
        LOOLWSD::sendMessageToForKit(aMessage);
        OutstandingForks += number;
        LastForkRequestTime = std::chrono::steady_clock::now();
        return number;
    }

    return 0;
}

/// Cleans up dead children.
/// Returns true if removed at least one.
static bool cleanupChildren()
{
    if (Util::isKitInProcess())
        return 0;

    Util::assertIsLocked(NewChildrenMutex);

    const int count = NewChildren.size();
    for (int i = count - 1; i >= 0; --i)
    {
        if (!NewChildren[i]->isAlive())
        {
            LOG_WRN("Removing dead spare child [" << NewChildren[i]->getPid() << "].");
            NewChildren.erase(NewChildren.begin() + i);
        }
    }

    return static_cast<int>(NewChildren.size()) != count;
}

/// Decides how many children need spawning and spawns.
/// Returns the number of children requested to spawn,
/// -1 for error.
static int rebalanceChildren(int balance)
{
    Util::assertIsLocked(NewChildrenMutex);

    const size_t available = NewChildren.size();
    LOG_TRC("Rebalance children to " << balance << ", have " << available << " and "
                                     << OutstandingForks << " outstanding requests");

    // Do the cleanup first.
    const bool rebalance = cleanupChildren();

    const auto duration = (std::chrono::steady_clock::now() - LastForkRequestTime);
    const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    if (OutstandingForks != 0 && durationMs >= std::chrono::milliseconds(ChildSpawnTimeoutMs))
    {
        // Children taking too long to spawn.
        // Forget we had requested any, and request anew.
        LOG_WRN("ForKit not responsive for " << durationMs << " forking " << OutstandingForks
                                             << " children. Resetting.");
        OutstandingForks = 0;
    }

    balance -= available;
    balance -= OutstandingForks;

    if (balance > 0 && (rebalance || OutstandingForks == 0))
    {
        LOG_DBG("prespawnChildren: Have " << available << " spare "
                                          << (available == 1 ? "child" : "children") << ", and "
                                          << OutstandingForks << " outstanding, forking " << balance
                                          << " more. Time since last request: " << durationMs);
        return forkChildren(balance);
    }

    return 0;
}

/// Proactively spawn children processes
/// to load documents with alacrity.
/// Returns true only if at least one child was requested to spawn.
static bool prespawnChildren()
{
    // Rebalance if not forking already.
    std::unique_lock<std::mutex> lock(NewChildrenMutex, std::defer_lock);
    return lock.try_lock() && (rebalanceChildren(LOOLWSD::NumPreSpawnedChildren) > 0);
}

#endif

static size_t addNewChild(std::shared_ptr<ChildProcess> child)
{
    assert(child && "Adding null child");
    const auto pid = child->getPid();

    std::unique_lock<std::mutex> lock(NewChildrenMutex);

    --OutstandingForks;
    // Prevent from going -ve if we have unexpected children.
    if (OutstandingForks < 0)
        ++OutstandingForks;

    if (LOOLWSD::IsBindMountingEnabled)
    {
        // Reset the child-spawn timeout to the default, now that we're set.
        // But only when mounting is enabled. Otherwise, copying is always slow.
        ChildSpawnTimeoutMs = CHILD_TIMEOUT_MS;
    }

    LOG_TRC("Adding a new child " << pid << " to NewChildren, have " << OutstandingForks
                                  << " outstanding requests");
    NewChildren.emplace_back(std::move(child));
    const size_t count = NewChildren.size();
    lock.unlock();

    LOG_INF("Have " << count << " spare " << (count == 1 ? "child" : "children")
                    << " after adding [" << pid << "]. Notifying.");

    NewChildrenCV.notify_one();
    return count;
}

#if !MOBILEAPP

namespace
{

#if ENABLE_DEBUG
inline std::string getLaunchBase(bool asAdmin = false)
{
    std::ostringstream oss;
    oss << "    ";
    oss << ((LOOLWSD::isSSLEnabled() || LOOLWSD::isSSLTermination()) ? "https://" : "http://");

    if (asAdmin)
    {
        auto user = LOOLWSD::getConfigValue<std::string>("admin_console.username", "");
        auto passwd = LOOLWSD::getConfigValue<std::string>("admin_console.password", "");

        if (user.empty() || passwd.empty())
            return "";

        oss << user << ':' << passwd << '@';
    }

    oss << LOOLWSD_TEST_HOST ":";
    oss << ClientPortNumber;

    return oss.str();
}

inline std::string getLaunchURI(const std::string &document, bool readonly = false)
{
    std::ostringstream oss;

    oss << getLaunchBase();
    oss << LOOLWSD::ServiceRoot;
    oss << LOOLWSD_TEST_LOOL_UI;
    oss << "?file_path=";
    oss << DEBUG_ABSSRCDIR "/";
    oss << document;
    if (readonly)
        oss << "&permission=readonly";

    return oss.str();
}

inline std::string getServiceURI(const std::string &sub, bool asAdmin = false)
{
    std::ostringstream oss;

    oss << getLaunchBase(asAdmin);
    oss << LOOLWSD::ServiceRoot;
    oss << sub;

    return oss.str();
}

#endif

} // anonymous namespace

#endif // MOBILEAPP

std::atomic<uint64_t> LOOLWSD::NextConnectionId(1);

#if !MOBILEAPP
std::atomic<int> LOOLWSD::ForKitProcId(-1);
std::shared_ptr<ForKitProcess> LOOLWSD::ForKitProc;
bool LOOLWSD::NoCapsForKit = false;
bool LOOLWSD::NoSeccomp = false;
bool LOOLWSD::AdminEnabled = true;
bool LOOLWSD::UnattendedRun = false;
bool LOOLWSD::SignalParent = false;
bool LOOLWSD::UseEnvVarOptions = false;
std::string LOOLWSD::RouteToken;
#if ENABLE_DEBUG
bool LOOLWSD::SingleKit = false;
bool LOOLWSD::ForceCaching = false;
#endif
LOOLWSD::WASMActivationState LOOLWSD::WASMState = LOOLWSD::WASMActivationState::Disabled;
std::unordered_map<std::string, std::chrono::steady_clock::time_point> LOOLWSD::Uri2WasmModeMap;
#endif
std::string LOOLWSD::SysTemplate;
std::string LOOLWSD::LoTemplate = LO_PATH;
std::string LOOLWSD::CleanupChildRoot;
std::string LOOLWSD::ChildRoot;
std::string LOOLWSD::ServerName;
std::string LOOLWSD::FileServerRoot;
std::string LOOLWSD::ServiceRoot;
std::string LOOLWSD::TmpFontDir;
std::string LOOLWSD::LOKitVersion;
std::string LOOLWSD::ConfigFile = LOOLWSD_CONFIGDIR "/loolwsd.xml";
std::string LOOLWSD::ConfigDir = LOOLWSD_CONFIGDIR "/conf.d";
bool LOOLWSD::EnableTraceEventLogging = false;
bool LOOLWSD::EnableAccessibility = false;
FILE *LOOLWSD::TraceEventFile = NULL;
std::string LOOLWSD::LogLevel = "trace";
std::string LOOLWSD::LogLevelStartup = "trace";
std::string LOOLWSD::LogToken;
std::string LOOLWSD::MostVerboseLogLevelSettableFromClient = "notice";
std::string LOOLWSD::LeastVerboseLogLevelSettableFromClient = "fatal";
std::string LOOLWSD::UserInterface = "default";
bool LOOLWSD::AnonymizeUserData = false;
bool LOOLWSD::CheckLoolUser = true;
bool LOOLWSD::CleanupOnly = false; //< If we should cleanup and exit.
bool LOOLWSD::IsProxyPrefixEnabled = false;
#if ENABLE_SSL
Util::RuntimeConstant<bool> LOOLWSD::SSLEnabled;
Util::RuntimeConstant<bool> LOOLWSD::SSLTermination;
#endif
unsigned LOOLWSD::MaxConnections;
unsigned LOOLWSD::MaxDocuments;
std::string LOOLWSD::OverrideWatermark;
std::set<const Poco::Util::AbstractConfiguration*> LOOLWSD::PluginConfigurations;
std::chrono::steady_clock::time_point LOOLWSD::StartTime;
bool LOOLWSD::IsBindMountingEnabled = true;

// If you add global state please update dumpState below too

static std::string UnitTestLibrary;

unsigned int LOOLWSD::NumPreSpawnedChildren = 0;
std::unique_ptr<TraceFileWriter> LOOLWSD::TraceDumper;
#if !MOBILEAPP
std::unique_ptr<ClipboardCache> LOOLWSD::SavedClipboards;

/// The file request handler used for file-serving.
std::unique_ptr<FileServerRequestHandler> LOOLWSD::FileRequestHandler;
#endif

/// This thread polls basic web serving, and handling of
/// websockets before upgrade: when upgraded they go to the
/// relevant DocumentBroker poll instead.
static std::shared_ptr<TerminatingPoll> WebServerPoll;

class PrisonPoll : public TerminatingPoll
{
public:
    PrisonPoll() : TerminatingPoll("prisoner_poll") {}

    /// Check prisoners are still alive and balanced.
    void wakeupHook() override;

#if !MOBILEAPP
    // Resets the forkit process object
    void setForKitProcess(const std::weak_ptr<ForKitProcess>& forKitProc)
    {
        assertCorrectThread(__FILE__, __LINE__);
        _forKitProc = forKitProc;
    }

    void sendMessageToForKit(const std::string& msg)
    {
        if (std::this_thread::get_id() == getThreadOwner())
        {
            // Speed up sending the message if the request comes from owner thread
            std::shared_ptr<ForKitProcess> forKitProc = _forKitProc.lock();
            if (forKitProc)
            {
                forKitProc->sendTextFrame(msg);
            }
        }
        else
        {
            // Put the message in the owner's thread queue to be send later
            // because WebSocketHandler is not thread safe and otherwise we
            // should synchronize inside WebSocketHandler.
            addCallback([this, msg]{
                std::shared_ptr<ForKitProcess> forKitProc = _forKitProc.lock();
                if (forKitProc)
                {
                    forKitProc->sendTextFrame(msg);
                }
            });
        }
    }

private:
    std::weak_ptr<ForKitProcess> _forKitProc;
#endif
};

/// This thread listens for and accepts prisoner kit processes.
/// And also cleans up and balances the correct number of children.
static std::shared_ptr<PrisonPoll> PrisonerPoll;

#if MOBILEAPP
#ifndef IOS
std::mutex LOOLWSD::lokit_main_mutex;
#endif
#endif

std::shared_ptr<ChildProcess> getNewChild_Blocks(SocketPoll &destPoll, unsigned mobileAppDocId)
{
    (void)mobileAppDocId;
    const auto startTime = std::chrono::steady_clock::now();

    std::unique_lock<std::mutex> lock(NewChildrenMutex);

#if !MOBILEAPP
    assert(mobileAppDocId == 0 && "Unexpected to have mobileAppDocId in the non-mobile build");

    int numPreSpawn = LOOLWSD::NumPreSpawnedChildren;
    ++numPreSpawn; // Replace the one we'll dispatch just now.
    LOG_DBG("getNewChild: Rebalancing children to " << numPreSpawn);
    if (rebalanceChildren(numPreSpawn) < 0)
    {
        LOG_DBG("getNewChild: rebalancing of children failed. Scheduling housekeeping to recover.");

        LOOLWSD::doHousekeeping();

        // Let the caller retry after a while.
        return nullptr;
    }

    const auto timeout = std::chrono::milliseconds(ChildSpawnTimeoutMs / 2);
    LOG_TRC("Waiting for a new child for a max of " << timeout);
#else // MOBILEAPP
    const auto timeout = std::chrono::hours(100);

#ifdef IOS
    assert(mobileAppDocId > 0 && "Unexpected to have no mobileAppDocId in the iOS build");
#endif

    std::thread([&]
                {
#ifndef IOS
                    std::lock_guard<std::mutex> lock(LOOLWSD::lokit_main_mutex);
                    Util::setThreadName("lokit_main");
#else
                    Util::setThreadName("lokit_main_" + Util::encodeId(mobileAppDocId, 3));
#endif
                    // Ugly to have that static global LOOLWSD::prisonerServerSocketFD, Otoh we know
                    // there is just one LOOLWSD object. (Even in real Online.)
                    lokit_main(LOOLWSD::prisonerServerSocketFD, LOOLWSD::UserInterface, mobileAppDocId);
                }).detach();
#endif // MOBILEAPP

    // FIXME: blocks ...
    // Unfortunately we need to wait after spawning children to avoid bombing the system.
    // If we fail fast and return, the next document will spawn more children without knowing
    // there are some on the way already. And if the system is slow already, that wouldn't help.
    LOG_TRC("Waiting for NewChildrenCV");
    if (NewChildrenCV.wait_for(lock, timeout, []()
                               {
                                   LOG_TRC("Predicate for NewChildrenCV wait: NewChildren.size()=" << NewChildren.size());
                                   return !NewChildren.empty();
                               }))
    {
        LOG_TRC("NewChildrenCV wait successful");
        std::shared_ptr<ChildProcess> child = NewChildren.back();
        NewChildren.pop_back();
        const size_t available = NewChildren.size();

        // Release early before moving sockets.
        lock.unlock();

        // Validate before returning.
        if (child && child->isAlive())
        {
            LOG_DBG("getNewChild: Have "
                    << available << " spare " << (available == 1 ? "child" : "children")
                    << " after popping [" << child->getPid() << "] to return in "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - startTime));

            // Change ownership now.
            child->moveSocketFromTo(PrisonerPoll, destPoll);

            return child;
        }

        LOG_WRN("getNewChild: popped dead child, need to find another.");
    }
    else
    {
        LOG_TRC("NewChildrenCV wait failed");
        LOG_WRN("getNewChild: No child available. Sending spawn request to forkit and failing.");
    }

    LOG_DBG("getNewChild: Timed out while waiting for new child.");
    return nullptr;
}

#ifdef __linux__
#if !MOBILEAPP
class InotifySocket : public Socket
{
public:
    InotifySocket():
        Socket(inotify_init1(IN_NONBLOCK), Socket::Type::Unix)
        , m_stopOnConfigChange(true)
    {
        if (getFD() == -1)
        {
            LOG_WRN("Inotify - Failed to start a watcher for the configuration, disabling "
                    "stop_on_config_change");
            m_stopOnConfigChange = false;
            return;
        }

        watch(LOOLWSD_CONFIGDIR);
    }

    /// Check for file changes, stop the server if we find any
    void handlePoll(SocketDisposition &disposition, std::chrono::steady_clock::time_point now, int events) override;

    int getPollEvents(std::chrono::steady_clock::time_point /* now */,
                      int64_t & /* timeoutMaxMicroS */) override
    {
        return POLLIN;
    }

    bool watch(std::string configFile);

private:
    bool m_stopOnConfigChange;
    int m_watchedCount = 0;
};

bool InotifySocket::watch(const std::string configFile)
{
    LOG_TRC("Inotify - Attempting to watch " << configFile << ", in addition to current "
                                             << m_watchedCount << " watched files");

    if (getFD() == -1)
    {
        LOG_WRN("Inotify - Trying to watch config file " << configFile
                                                         << " without an inotify file descriptor");
        return false;
    }

    int watchedStatus;
    watchedStatus = inotify_add_watch(getFD(), configFile.c_str(), IN_MODIFY);

    if (watchedStatus == -1)
        LOG_WRN("Inotify - Failed to watch config file " << configFile);
    else
        m_watchedCount++;

    return watchedStatus != -1;
}

void InotifySocket::handlePoll(SocketDisposition & /* disposition */, std::chrono::steady_clock::time_point /* now */, int /* events */)
{
    LOG_TRC("InotifyPoll - woken up. Reload on config change: "
            << m_stopOnConfigChange << ", Watching " << m_watchedCount << " files");
    if (!m_stopOnConfigChange)
        return;

    char buf[4096];

    static_assert(sizeof(buf) >= sizeof(struct inotify_event) + NAME_MAX + 1, "see man 7 inotify");

    const struct inotify_event* event;

    LOG_TRC("InotifyPoll - Checking for config changes...");

    while (true)
    {
        ssize_t len = read(getFD(), buf, sizeof(buf));

        if (len == -1 && errno != EAGAIN)
        {
            // Some read error, EAGAIN is when there is no data so let's not warn for it
            LOG_WRN("InotifyPoll - Read error " << std::strerror(errno)
                                                << " when trying to get events");
        }
        else if (len == -1)
        {
            LOG_TRC("InotifyPoll - Got to end of data when reading inotify");
        }

        if (len <= 0)
            break;

        assert(buf[len - 1] == 0 && "see man 7 inotify");

        for (char* ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len)
        {
            event = (const struct inotify_event*)ptr;

            LOG_WRN("InotifyPoll - Config file " << event->name << " was modified, stopping LOOLWSD");
            SigUtil::requestShutdown();
        }
    }
}

#endif // if !MOBILEAPP
#endif // #ifdef __linux__

/// The Web Server instance with the accept socket poll thread.
class LOOLWSDServer;
static std::unique_ptr<LOOLWSDServer> Server;

/// Helper class to hold default configuration entries.
class AppConfigMap final : public Poco::Util::MapConfiguration
{
public:
    AppConfigMap(const std::map<std::string, std::string>& map)
    {
        for (const auto& pair : map)
        {
            setRaw(pair.first, pair.second);
        }
    }

    void reset(const std::map<std::string, std::string>& map)
    {
        clear();
        for (const auto& pair : map)
        {
            setRaw(pair.first, pair.second);
        }
    }
};

#if !MOBILEAPP

void ForKitProcWSHandler::handleMessage(const std::vector<char> &data)
{
    LOG_TRC("ForKitProcWSHandler: handling incoming [" << LOOLProtocol::getAbbreviatedMessage(&data[0], data.size()) << "].");
    const std::string firstLine = LOOLProtocol::getFirstLine(&data[0], data.size());
    const StringVector tokens = StringVector::tokenize(firstLine.data(), firstLine.size());

    if (tokens.equals(0, "segfaultcount"))
    {
        int count = std::stoi(tokens[1]);
        if (count >= 0)
        {
            Admin::instance().addSegFaultCount(count);
            LOG_INF(count << " loolkit processes crashed with segmentation fault.");
        }
        else
        {
            LOG_WRN("Invalid 'segfaultcount' message received.");
        }
    }
    else
    {
        LOG_ERR("ForKitProcWSHandler: unknown command: " << tokens[0]);
    }
}

#endif

LOOLWSD::LOOLWSD()
{
}

LOOLWSD::~LOOLWSD()
{
}

#if !MOBILEAPP

/**
  A custom socket poll to fetch remote config every 60 seconds.
  If config changes it applies the new config using LayeredConfiguration.

  The URI to fetch is read from the configuration too - it is either the
  remote config itself, or the font configuration.  It si passed via the
  uriConfigKey param.
*/
class RemoteJSONPoll : public SocketPoll
{
public:
    RemoteJSONPoll(LayeredConfiguration& config, const std::string& uriConfigKey, const std::string& name, const std::string& kind)
        : SocketPoll(name)
        , conf(config)
        , configKey(uriConfigKey)
        , expectedKind(kind)
    { }

    virtual ~RemoteJSONPoll() { }

    virtual void handleJSON(const Poco::JSON::Object::Ptr& json) = 0;

    virtual void handleUnchangedJSON()
    { }

    void start()
    {
        Poco::URI remoteServerURI(conf.getString(configKey));

        if (expectedKind == "configuration")
        {
            if (remoteServerURI.empty())
            {
                LOG_INF("Remote " << expectedKind << " is not specified in loolwsd.xml");
                return; // no remote config server setup.
            }
#if !ENABLE_DEBUG
            if (Util::iequal(remoteServerURI.getScheme(), "http"))
            {
                LOG_ERR("Remote config url should only use HTTPS protocol: " << remoteServerURI.toString());
                return;
            }
#endif
        }

        startThread();
    }

    void pollingThread()
    {
        while (!isStop() && !SigUtil::getShutdownRequestFlag())
        {
            Poco::URI remoteServerURI(conf.getString(configKey));

            // don't try to fetch from an empty URI
            bool valid = !remoteServerURI.empty();

#if !ENABLE_DEBUG
            if (Util::iequal(remoteServerURI.getScheme(), "http"))
            {
                LOG_ERR("Remote config url should only use HTTPS protocol: " << remoteServerURI.toString());
                valid = false;
            }
#endif

            if (valid)
            {
                try
                {
                    std::shared_ptr<http::Session> httpSession(
                            StorageBase::getHttpSession(remoteServerURI));
                    http::Request request(remoteServerURI.getPathAndQuery());

                    //we use ETag header to check whether JSON is modified or not
                    if (!eTagValue.empty())
                    {
                        request.set("If-None-Match", eTagValue);
                    }

                    const std::shared_ptr<const http::Response> httpResponse =
                        httpSession->syncRequest(request);

                    const http::StatusCode statusCode = httpResponse->statusLine().statusCode();

                    if (statusCode == http::StatusCode::OK)
                    {
                        eTagValue = httpResponse->get("ETag");

                        std::string body = httpResponse->getBody();

                        LOG_DBG("Got " << body.size() << " bytes for " << remoteServerURI.toString());

                        Poco::JSON::Object::Ptr remoteJson;
                        if (JsonUtil::parseJSON(body, remoteJson))
                        {
                            std::string kind;
                            JsonUtil::findJSONValue(remoteJson, "kind", kind);
                            if (kind == expectedKind)
                            {
                                handleJSON(remoteJson);
                            }
                            else
                            {
                                LOG_ERR("Make sure that " << remoteServerURI.toString() << " contains a property 'kind' with "
                                        "value '" << expectedKind << "'");
                            }
                        }
                        else
                        {
                            LOG_ERR("Could not parse the remote config JSON");
                        }
                    }
                    else if (statusCode == http::StatusCode::NotModified)
                    {
                        LOG_DBG("Not modified since last time: " << remoteServerURI.toString());
                        handleUnchangedJSON();
                    }
                    else
                    {
                        LOG_ERR("Remote config server has response status code: " << statusCode);
                    }
                }
                catch (...)
                {
                    LOG_ERR("Failed to fetch remote config JSON, Please check JSON format");
                }
            }
            poll(std::chrono::seconds(60));
        }
    }

protected:
    LayeredConfiguration& conf;
    std::string eTagValue;

private:
    std::string configKey;
    std::string expectedKind;
};

class RemoteConfigPoll : public RemoteJSONPoll
{
public:
    RemoteConfigPoll(LayeredConfiguration& config) :
        RemoteJSONPoll(config, "remote_config.remote_url", "remoteconfig_poll", "configuration")
    {
        constexpr int PRIO_JSON = -200; // highest priority
        _persistConfig = new AppConfigMap(std::map<std::string, std::string>{});
        conf.addWriteable(_persistConfig, PRIO_JSON);
    }

    virtual ~RemoteConfigPoll() { }

    void handleJSON(const Poco::JSON::Object::Ptr& remoteJson) override
    {
        std::map<std::string, std::string> newAppConfig;

        fetchAliasGroups(newAppConfig, remoteJson);

#if ENABLE_FEATURE_LOCK
        fetchLockedHostPatterns(newAppConfig, remoteJson);
        fetchLockedTranslations(newAppConfig, remoteJson);
        fetchUnlockImageUrl(newAppConfig, remoteJson);
#endif

        fetchIndirectionEndpoint(newAppConfig, remoteJson);

        fetchMonitors(newAppConfig, remoteJson);

        fetchRemoteFontConfig(newAppConfig, remoteJson);

        // before resetting get monitors list
        std::vector<std::pair<std::string,int>> oldMonitors = Admin::instance().getMonitorList();

        _persistConfig->reset(newAppConfig);

#if ENABLE_FEATURE_LOCK
        CommandControl::LockManager::parseLockedHost(conf);
#endif
        Admin::instance().updateMonitors(oldMonitors);

        HostUtil::parseAliases(conf);
    }

    void fetchLockedHostPatterns(std::map<std::string, std::string>& newAppConfig,
                                 Poco::JSON::Object::Ptr remoteJson)
    {
        try
        {
            Poco::JSON::Object::Ptr lockedHost;
            Poco::JSON::Array::Ptr lockedHostPatterns;
            try
            {
                lockedHost = remoteJson->getObject("feature_locking")->getObject("locked_hosts");
                lockedHostPatterns = lockedHost->getArray("hosts");
            }
            catch (const Poco::NullPointerException&)
            {
                LOG_INF("Overriding locked_hosts failed because feature_locking->locked_hosts->hosts array does not exist");
                return;
            }

            if (lockedHostPatterns.isNull() || lockedHostPatterns->size() == 0)
            {
                LOG_INF(
                    "Overriding locked_hosts failed because locked_hosts->hosts array is empty or null");
                return;
            }

            //use feature_lock.locked_hosts[@allow] entry from loolwsd.xml if feature_lock.locked_hosts.allow key doesnot exist in json
            Poco::Dynamic::Var allow = !lockedHost->has("allow") ? Poco::Dynamic::Var(conf.getBool("feature_lock.locked_hosts[@allow]"))
                                                                 : lockedHost->get("allow");
            newAppConfig.insert(std::make_pair("feature_lock.locked_hosts[@allow]", booleanToString(allow)));

            if (booleanToString(allow) == "false")
            {
                LOG_INF("locked_hosts feature is disabled, set feature_lock->locked_hosts->allow to true to enable");
                return;
            }

            std::size_t i;
            for (i = 0; i < lockedHostPatterns->size(); i++)
            {
                std::string host;
                Poco::JSON::Object::Ptr subObject = lockedHostPatterns->getObject(i);
                JsonUtil::findJSONValue(subObject, "host", host);
                Poco::Dynamic::Var readOnly = subObject->get("read_only");
                Poco::Dynamic::Var disabledCommands = subObject->get("disabled_commands");

                const std::string path =
                    "feature_lock.locked_hosts.host[" + std::to_string(i) + "]";
                newAppConfig.insert(std::make_pair(path, host));
                newAppConfig.insert(std::make_pair(path + "[@read_only]", booleanToString(readOnly)));
                newAppConfig.insert(std::make_pair(path + "[@disabled_commands]",
                                                   booleanToString(disabledCommands)));
            }

            //if number of locked wopi host patterns defined in loolwsd.xml are greater than number of host
            //fetched from json, overwrite the remaining host from config file to empty strings and
            //set read_only and disabled_commands to false
            for (;; ++i)
            {
                const std::string path =
                    "feature_lock.locked_hosts.host[" + std::to_string(i) + "]";
                if (!conf.has(path))
                {
                    break;
                }
                newAppConfig.insert(std::make_pair(path, ""));
                newAppConfig.insert(std::make_pair(path + "[@read_only]", "false"));
                newAppConfig.insert(std::make_pair(path + "[@disabled_commands]", "false"));
            }
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Failed to fetch locked_hosts, please check JSON format: " << exc.what());
        }
    }

    void fetchAliasGroups(std::map<std::string, std::string>& newAppConfig,
                          const Poco::JSON::Object::Ptr& remoteJson)
    {
        try
        {
            Poco::JSON::Object::Ptr aliasGroups;
            Poco::JSON::Array::Ptr groups;

            try
            {
                aliasGroups = remoteJson->getObject("storage")->getObject("wopi")->getObject("alias_groups");
                groups = aliasGroups->getArray("groups");
            }
            catch (const Poco::NullPointerException&)
            {
                LOG_INF("Overriding alias_groups failed because storage->wopi->alias_groups->groups array does not exist");
                return;
            }

            if (groups.isNull() || groups->size() == 0)
            {
                LOG_INF("Overriding alias_groups failed because alias_groups->groups array is empty or null");
                return;
            }

            std::string mode = "first";
            JsonUtil::findJSONValue(aliasGroups, "mode", mode);
            newAppConfig.insert(std::make_pair("storage.wopi.alias_groups[@mode]", mode));

            std::size_t i;
            for (i = 0; i < groups->size(); i++)
            {
                Poco::JSON::Object::Ptr group = groups->getObject(i);
                std::string host;
                JsonUtil::findJSONValue(group, "host", host);
                Poco::Dynamic::Var allow = group->get("allow");
                const std::string path =
                    "storage.wopi.alias_groups.group[" + std::to_string(i) + ']';

                newAppConfig.insert(std::make_pair(path + ".host", host));
                newAppConfig.insert(std::make_pair(path + ".host[@allow]", booleanToString(allow)));
#if ENABLE_FEATURE_LOCK
                std::string unlockLink;
                JsonUtil::findJSONValue(group, "unlock_link", unlockLink);
                newAppConfig.insert(std::make_pair(path + ".unlock_link", unlockLink));
#endif
                Poco::JSON::Array::Ptr aliases = group->getArray("aliases");

                size_t j = 0;
                if (aliases)
                {
                    auto it = aliases->begin();
                    for (; j < aliases->size(); j++)
                    {
                        const std::string aliasPath = path + ".alias[" + std::to_string(j) + ']';
                        newAppConfig.insert(std::make_pair(aliasPath, it->toString()));
                        it++;
                    }
                }
                for (;; j++)
                {
                    const std::string aliasPath = path + ".alias[" + std::to_string(j) + ']';
                    if (!conf.has(aliasPath))
                    {
                        break;
                    }
                    newAppConfig.insert(std::make_pair(aliasPath, ""));
                }
            }

            //if number of alias_groups defined in configuration are greater than number of alias_group
            //fetched from json, overwrite the remaining alias_groups from config file to empty strings and
            for (;; i++)
            {
                const std::string path =
                    "storage.wopi.alias_groups.group[" + std::to_string(i) + "].host";
                if (!conf.has(path))
                {
                    break;
                }
                newAppConfig.insert(std::make_pair(path, ""));
                newAppConfig.insert(std::make_pair(path + "[@allowed]", "false"));
            }
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Fetching of alias groups failed with error: " << exc.what()
                                                                   << ", please check JSON format");
        }
    }

    void fetchRemoteFontConfig(std::map<std::string, std::string>& newAppConfig,
                               const Poco::JSON::Object::Ptr& remoteJson)
    {
        try
        {
            Poco::JSON::Object::Ptr remoteFontConfig = remoteJson->getObject("remote_font_config");

            std::string url;
            if (JsonUtil::findJSONValue(remoteFontConfig, "url", url))
                newAppConfig.insert(std::make_pair("remote_font_config.url", url));
        }
        catch (const Poco::NullPointerException&)
        {
            LOG_INF("Overriding the remote font config URL failed because the remove_font_config entry does not exist");
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Failed to fetch remote_font_config, please check JSON format: " << exc.what());
        }
    }

    void fetchLockedTranslations(std::map<std::string, std::string>& newAppConfig,
                                 const Poco::JSON::Object::Ptr& remoteJson)
    {
        try
        {
            Poco::JSON::Array::Ptr lockedTranslations;
            try
            {
                lockedTranslations =
                    remoteJson->getObject("feature_locking")->getArray("translations");
            }
            catch (const Poco::NullPointerException&)
            {
                LOG_INF(
                    "Overriding translations failed because feature_locking->translations array "
                    "does not exist");
                return;
            }

            if (lockedTranslations.isNull() || lockedTranslations->size() == 0)
            {
                LOG_INF("Overriding feature_locking->translations failed because array is empty or "
                        "null");
                return;
            }

            std::size_t i;
            for (i = 0; i < lockedTranslations->size(); i++)
            {
                Poco::JSON::Object::Ptr translation = lockedTranslations->getObject(i);
                std::string language;
                //default values if the one of the entry is missing in json
                std::string title = conf.getString("feature_lock.unlock_title", "");
                std::string description = conf.getString("feature_lock.unlock_description", "");
                std::string writerHighlights =
                    conf.getString("feature_lock.writer_unlock_highlights", "");
                std::string impressHighlights =
                    conf.getString("feature_lock.impress_unlock_highlights", "");
                std::string calcHighlights =
                    conf.getString("feature_lock.calc_unlock_highlights", "");
                std::string drawHighlights =
                    conf.getString("feature_lock.draw_unlock_highlights", "");

                JsonUtil::findJSONValue(translation, "language", language);
                JsonUtil::findJSONValue(translation, "unlock_title", title);
                JsonUtil::findJSONValue(translation, "unlock_description", description);
                JsonUtil::findJSONValue(translation, "writer_unlock_highlights", writerHighlights);
                JsonUtil::findJSONValue(translation, "calc_unlock_highlights", calcHighlights);
                JsonUtil::findJSONValue(translation, "impress_unlock_highlights",
                                        impressHighlights);
                JsonUtil::findJSONValue(translation, "draw_unlock_highlights", drawHighlights);

                const std::string path =
                    "feature_lock.translations.language[" + std::to_string(i) + ']';

                newAppConfig.insert(std::make_pair(path + "[@name]", language));
                newAppConfig.insert(std::make_pair(path + ".unlock_title", title));
                newAppConfig.insert(std::make_pair(path + ".unlock_description", description));
                newAppConfig.insert(
                    std::make_pair(path + ".writer_unlock_highlights", writerHighlights));
                newAppConfig.insert(
                    std::make_pair(path + ".calc_unlock_highlights", calcHighlights));
                newAppConfig.insert(
                    std::make_pair(path + ".impress_unlock_highlights", impressHighlights));
                newAppConfig.insert(
                    std::make_pair(path + ".draw_unlock_highlights", drawHighlights));
            }

            //if number of translations defined in configuration are greater than number of translation
            //fetched from json, overwrite the remaining translations from config file to empty strings
            for (;; i++)
            {
                const std::string path =
                    "feature_lock.translations.language[" + std::to_string(i) + "][@name]";
                if (!conf.has(path))
                {
                    break;
                }
                newAppConfig.insert(std::make_pair(path, ""));
            }
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Failed to fetch feature_locking->translations, please check JSON format: " << exc.what());
        }
    }

    void fetchUnlockImageUrl(std::map<std::string, std::string>& newAppConfig,
                             const Poco::JSON::Object::Ptr& remoteJson)
    {
        try
        {
            Poco::JSON::Object::Ptr featureLocking = remoteJson->getObject("feature_locking");

            std::string unlockImage;
            if (JsonUtil::findJSONValue(featureLocking, "unlock_image", unlockImage))
            {
                newAppConfig.insert(std::make_pair("feature_lock.unlock_image", unlockImage));
            }
        }
        catch (const Poco::NullPointerException&)
        {
            LOG_INF("Overriding unlock_image URL failed because the unlock_image entry does not "
                    "exist");
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Failed to fetch unlock_image, please check JSON format: " << exc.what());
        }
    }

    void fetchIndirectionEndpoint(std::map<std::string, std::string>& newAppConfig,
                                  const Poco::JSON::Object::Ptr& remoteJson)
    {
        try
        {
            Poco::JSON::Object::Ptr indirectionEndpoint =
                remoteJson->getObject("indirection_endpoint");

            std::string url;
            if (JsonUtil::findJSONValue(indirectionEndpoint, "url", url))
            {
                newAppConfig.insert(std::make_pair("indirection_endpoint.url", url));
            }
        }
        catch (const Poco::NullPointerException&)
        {
            LOG_INF("Overriding indirection_endpoint.url failed because the indirection_endpoint.url "
                    "entry does not "
                    "exist");
        }
        catch (const std::exception& exc)
        {
            LOG_ERR(
                "Failed to fetch indirection_endpoint, please check JSON format: " << exc.what());
        }
    }

    void fetchMonitors(std::map<std::string, std::string>& newAppConfig,
                       const Poco::JSON::Object::Ptr& remoteJson)
    {
        Poco::JSON::Array::Ptr monitors;
        try
        {
            monitors = remoteJson->getArray("monitors");
        }
        catch (const Poco::NullPointerException&)
        {
            LOG_INF("Overriding monitor failed because array "
                    "does not exist");
            return;
        }

        if (monitors.isNull() || monitors->size() == 0)
        {
            LOG_INF("Overriding monitors failed because array is empty or "
                    "null");
            return;
        }
        std::size_t i;
        for (i = 0; i < monitors->size(); i++)
            newAppConfig.insert(
                std::make_pair("monitors.monitor[" + std::to_string(i) + ']', monitors->get(i).toString()));

        //if number of monitors defined in configuration are greater than number of monitors
        //fetched from json or if the number of monitors shrinks with new json,
        //overwrite the remaining monitors from config file to empty strings
        for (;; i++)
        {
            const std::string path =
                "monitors.monitor[" + std::to_string(i) + ']';
            if (!conf.has(path))
            {
                break;
            }
            newAppConfig.insert(std::make_pair(path, ""));
        }
    }


    //sets property to false if it is missing from JSON
    //and returns std::string
    std::string booleanToString(Poco::Dynamic::Var& booleanFlag)
    {
        if (booleanFlag.isEmpty())
        {
            booleanFlag = "false";
        }
        return booleanFlag.toString();
    }

private:
    // keeps track of remote config layer
    Poco::AutoPtr<AppConfigMap> _persistConfig = nullptr;
};

class RemoteFontConfigPoll : public RemoteJSONPoll
{
public:
    RemoteFontConfigPoll(LayeredConfiguration& config)
        : RemoteJSONPoll(config, "remote_font_config.url", "remotefontconfig_poll", "fontconfiguration")
    {
    }

    virtual ~RemoteFontConfigPoll() { }

    void handleJSON(const Poco::JSON::Object::Ptr& remoteJson) override
    {
        // First mark all fonts we have downloaded previously as "inactive" to be able to check if
        // some font gets deleted from the list in the JSON file.
        for (auto& it : fonts)
            it.second.active = false;

        // Just pick up new fonts.
        auto fontsPtr = remoteJson->getArray("fonts");
        if (!fontsPtr)
        {
            LOG_WRN("The 'fonts' property does not exist or is not an array");
            return;
        }

        for (std::size_t i = 0; i < fontsPtr->size(); i++)
        {
            if (!fontsPtr->isObject(i))
                LOG_WRN("Element " << i << " in fonts array is not an object");
            else
            {
                const auto fontPtr = fontsPtr->getObject(i);
                const auto uriPtr = fontPtr->get("uri");
                if (uriPtr.isEmpty() || !uriPtr.isString())
                    LOG_WRN("Element in fonts array does not have an 'uri' property or it is not a string");
                else
                {
                    const std::string uri = uriPtr.toString();
                    const auto stampPtr = fontPtr->get("stamp");

                    if (!stampPtr.isEmpty() && !stampPtr.isString())
                        LOG_WRN("Element in fonts array with uri '" << uri << "' has a stamp property that is not a string, ignored");
                    else if (fonts.count(uri) == 0)
                    {
                        // First case: This font has not been downloaded.
                        if (!stampPtr.isEmpty())
                        {
                            if (downloadPlain(uri))
                            {
                                fonts[uri].stamp = stampPtr.toString();
                                fonts[uri].active = true;
                            }
                        }
                        else
                        {
                            if (downloadWithETag(uri, ""))
                            {
                                fonts[uri].active = true;
                            }
                        }
                    }
                    else if (!stampPtr.isEmpty() && stampPtr.toString() != fonts[uri].stamp)
                    {
                        // Second case: Font has been downloaded already, has a "stamp" property,
                        // and that has been changed in the JSON since it was downloaded.
                        restartForKitAndReDownloadConfigFile();
                        break;
                    }
                    else if (!stampPtr.isEmpty())
                    {
                        // Third case: Font has been downloaded already, has a "stamp" property, and
                        // that has *not* changed in the JSON since it was downloaded.
                        fonts[uri].active = true;
                    }
                    else
                    {
                        // Last case: Font has been downloaded but does not have a "stamp" property.
                        // Use ETag.
                        if (!eTagUnchanged(uri, fonts[uri].eTag))
                        {
                            restartForKitAndReDownloadConfigFile();
                            break;
                        }
                        fonts[uri].active = true;
                    }
                }
            }
        }

        // Any font that has been deleted from the JSON needs to be removed on this side, too.
        for (const auto &it : fonts)
        {
            if (!it.second.active)
            {
                LOG_DBG("Font no longer mentioned in the remote font config: " << it.first);
                restartForKitAndReDownloadConfigFile();
                break;
            }
        }
    }

    void handleUnchangedJSON() override
    {
        // Iterate over the fonts that were mentioned in the JSON file when it was last downloaded.
        for (auto& it : fonts)
        {
            // If the JSON has a "stamp" for the font, and we have already downloaded it, by
            // definition we don't need to do anything when the JSON file has not changed.
            if (it.second.stamp != "" && it.second.pathName != "")
                continue;

            // If the JSON has a "stamp" it must have been downloaded already. Should we even
            // assert() that?
            if (it.second.stamp != "" && it.second.pathName == "")
            {
                LOG_WRN("Font at " << it.first << " was not downloaded, should have been");
                continue;
            }

            // Otherwise use the ETag to check if the font file needs re-downloading.
            if (!eTagUnchanged(it.first, it.second.eTag))
            {
                restartForKitAndReDownloadConfigFile();
                break;
            }
        }
    }

private:
    bool downloadPlain(const std::string& uri)
    {
        const Poco::URI fontUri{uri};
        std::shared_ptr<http::Session> httpSession(StorageBase::getHttpSession(fontUri));
        http::Request request(fontUri.getPathAndQuery());

        request.set("User-Agent", WOPI_AGENT_STRING);

        const std::shared_ptr<const http::Response> httpResponse
            = httpSession->syncRequest(request);

        return finishDownload(uri, httpResponse);
    }

    bool eTagUnchanged(const std::string& uri, const std::string& oldETag)
    {
        const Poco::URI fontUri{uri};
        std::shared_ptr<http::Session> httpSession(StorageBase::getHttpSession(fontUri));
        http::Request request(fontUri.getPathAndQuery());

        if (!oldETag.empty())
        {
            request.set("If-None-Match", oldETag);
        }

        request.set("User-Agent", WOPI_AGENT_STRING);

        const std::shared_ptr<const http::Response> httpResponse
            = httpSession->syncRequest(request);

        if (httpResponse->statusLine().statusCode() == http::StatusCode::NotModified)
        {
            LOG_DBG("Not modified since last time: " << uri);
            return true;
        }

        return false;
    }

    bool downloadWithETag(const std::string& uri, const std::string& oldETag)
    {
        const Poco::URI fontUri{uri};
        std::shared_ptr<http::Session> httpSession(StorageBase::getHttpSession(fontUri));
        http::Request request(fontUri.getPathAndQuery());

        if (!oldETag.empty())
        {
            request.set("If-None-Match", oldETag);
        }

        request.set("User-Agent", WOPI_AGENT_STRING);

        const std::shared_ptr<const http::Response> httpResponse
            = httpSession->syncRequest(request);

        if (httpResponse->statusLine().statusCode() == http::StatusCode::NotModified)
        {
            LOG_DBG("Not modified since last time: " << uri);
            return true;
        }

        if (!finishDownload(uri, httpResponse))
            return false;

        fonts[uri].eTag = httpResponse->get("ETag");
        return true;
    }

    bool finishDownload(const std::string& uri, const std::shared_ptr<const http::Response> httpResponse)
    {
        if (httpResponse->statusLine().statusCode() != http::StatusCode::OK)
        {
            LOG_WRN("Could not fetch " << uri);
            return false;
        }

        const std::string body = httpResponse->getBody();

        std::string fontFile;

        // We intentionally use a new file name also when an updated version of a font is
        // downloaded. It causes trouble to rewrite the same file, in case it is in use in some Kit
        // process at the moment.

        // We don't remove the old file either as that also causes problems.

        // And in reality, it is a bit unclear how likely it even is that fonts downloaded through
        // this mechanism even will be updated.
        fontFile = LOOLWSD::TmpFontDir + "/" + Util::encodeId(Util::rng::getNext()) + ".ttf";

        std::ofstream fontStream(fontFile);
        fontStream.write(body.data(), body.size());
        if (!fontStream.good())
        {
                LOG_ERR("Could not write to " << fontFile);
                return false;
        }

        LOG_DBG("Got " << body.size() << " bytes for " << uri << " and wrote to " << fontFile);
        fonts[uri].pathName = fontFile;

        LOOLWSD::sendMessageToForKit("addfont " + fontFile);

        return true;
    }

    void restartForKitAndReDownloadConfigFile()
    {
        LOG_DBG("Downloaded font has been updated or a font has been removed. ForKit must be restarted.");
        fonts.clear();
        // Clear the saved ETag of the remote font configuration file so that it will be
        // re-downloaded, and all fonts mentioned in it re-downloaded and fed to ForKit.
        eTagValue.clear();
        LOOLWSD::sendMessageToForKit("exit");
    }

    struct FontData
    {
        // Each font can have a "stamp" in the JSON that we treat just as a string. In practice it
        // can be some timestamp, but we don't parse it. If the stamp is changed, we re-download the
        // font file.
        std::string stamp;

        // If the font has no "stamp" property, we use the ETag mechanism to see if the font file
        // needs to be re-downloaded.
        std::string eTag;

        // Where the font has been stored
        std::string pathName;

        // Flag that tells whether the font is mentioned in the JSON file that is being handled.
        // Used only in handleJSON() when the JSON has been (re-)downloaded, not when the JSON was
        // unchanged in handleUnchangedJSON().
        bool active;
    };

    // The key of this map is the download URI of the font.
    std::map<std::string, FontData> fonts;
};
#endif

void LOOLWSD::innerInitialize(Application& self)
{
    if (!Util::isMobileApp() && geteuid() == 0 && CheckLoolUser)
    {
        throw std::runtime_error("Do not run as root. Please run as lool user.");
    }

    Util::setApplicationPath(Poco::Path(Application::instance().commandPath()).parent().toString());

    StartTime = std::chrono::steady_clock::now();

    LayeredConfiguration& conf = config();

    // Add default values of new entries here, so there is a sensible default in case
    // the setting is missing from the config file. It is possible that users do not
    // update their config files, and we are backward compatible.
    // These defaults should be the same
    // 1) here
    // 2) in the 'default' attribute in loolwsd.xml, which is for documentation
    // 3) the default parameter of getConfigValue() call. That is used when the
    //    setting is present in loolwsd.xml, but empty (i.e. use the default).
    static const std::map<std::string, std::string> DefAppConfig = {
        { "accessibility.enable", "false" },
        { "allowed_languages", "de_DE en_GB en_US es_ES fr_FR it nl pt_BR pt_PT ru" },
        { "admin_console.enable_pam", "false" },
        { "child_root_path", "jails" },
        { "file_server_root_path", "browser/.." },
        { "enable_websocket_urp", "false" },
        { "hexify_embedded_urls", "false" },
        { "experimental_features", "false" },
        { "logging.protocol", "false" },
        // { "logging.anonymize.anonymize_user_data", "false" }, // Do not set to fallback on filename/username.
        { "logging.color", "true" },
        { "logging.file.property[0]", "loolwsd.log" },
        { "logging.file.property[0][@name]", "path" },
        { "logging.file.property[1]", "never" },
        { "logging.file.property[1][@name]", "rotation" },
        { "logging.file.property[2]", "true" },
        { "logging.file.property[2][@name]", "compress" },
        { "logging.file.property[3]", "false" },
        { "logging.file.property[3][@name]", "flush" },
        { "logging.file.property[4]", "10 days" },
        { "logging.file.property[4][@name]", "purgeAge" },
        { "logging.file.property[5]", "10" },
        { "logging.file.property[5][@name]", "purgeCount" },
        { "logging.file.property[6]", "true" },
        { "logging.file.property[6][@name]", "rotateOnOpen" },
        { "logging.file.property[7]", "false" },
        { "logging.file.property[7][@name]", "archive" },
        { "logging.file[@enable]", "false" },
        { "logging.level", "trace" },
        { "logging.level_startup", "trace" },
        { "logging.lokit_sal_log", "-INFO-WARN" },
        { "logging.docstats", "false" },
        { "logging.userstats", "false" },
        { "browser_logging", "false" },
        { "mount_jail_tree", "true" },
        { "net.connection_timeout_secs", "30" },
        { "net.listen", "any" },
        { "net.proto", "all" },
        { "net.service_root", "" },
        { "net.proxy_prefix", "false" },
        { "net.content_security_policy", "" },
        { "net.frame_ancestors", "" },
        { "num_prespawn_children", "1" },
        { "per_document.always_save_on_exit", "false" },
        { "per_document.autosave_duration_secs", "300" },
        { "per_document.cleanup.cleanup_interval_ms", "10000" },
        { "per_document.cleanup.bad_behavior_period_secs", "60" },
        { "per_document.cleanup.idle_time_secs", "300" },
        { "per_document.cleanup.limit_dirty_mem_mb", "3072" },
        { "per_document.cleanup.limit_cpu_per", "85" },
        { "per_document.cleanup.lost_kit_grace_period_secs", "120" },
        { "per_document.cleanup[@enable]", "false" },
        { "per_document.idle_timeout_secs", "3600" },
        { "per_document.idlesave_duration_secs", "30" },
        { "per_document.limit_file_size_mb", "0" },
        { "per_document.limit_num_open_files", "0" },
        { "per_document.limit_load_secs", "100" },
        { "per_document.limit_store_failures", "5" },
        { "per_document.limit_convert_secs", "100" },
        { "per_document.limit_stack_mem_kb", "8000" },
        { "per_document.limit_virt_mem_mb", "0" },
        { "per_document.max_concurrency", "4" },
        { "per_document.min_time_between_saves_ms", "500" },
        { "per_document.min_time_between_uploads_ms", "5000" },
        { "per_document.batch_priority", "5" },
        { "per_document.pdf_resolution_dpi", "96" },
        { "per_document.redlining_as_comments", "false" },
        { "per_view.idle_timeout_secs", "900" },
        { "per_view.out_of_focus_timeout_secs", "120" },
        { "per_view.custom_os_info", "" },
        { "security.capabilities", "true" },
        { "security.seccomp", "true" },
        { "security.jwt_expiry_secs", "1800" },
        { "security.enable_metrics_unauthenticated", "false" },
        { "certificates.database_path", "" },
        { "server_name", "" },
        { "ssl.ca_file_path", LOOLWSD_CONFIGDIR "/ca-chain.cert.pem" },
        { "ssl.cert_file_path", LOOLWSD_CONFIGDIR "/cert.pem" },
        { "ssl.enable", "true" },
        { "ssl.hpkp.max_age[@enable]", "true" },
        { "ssl.hpkp.report_uri[@enable]", "false" },
        { "ssl.hpkp[@enable]", "false" },
        { "ssl.hpkp[@report_only]", "false" },
        { "ssl.sts.enabled", "false" },
        { "ssl.sts.max_age", "31536000" },
        { "ssl.key_file_path", LOOLWSD_CONFIGDIR "/key.pem" },
        { "ssl.termination", "true" },
        { "stop_on_config_change", "false" },
        { "storage.filesystem[@allow]", "false" },
        // "storage.ssl.enable" - deliberately not set; for back-compat
        { "storage.wopi.max_file_size", "0" },
        { "storage.wopi[@allow]", "true" },
        { "storage.wopi.locking.refresh", "900" },
        { "sys_template_path", "systemplate" },
        { "trace_event[@enable]", "false" },
        { "trace.path[@compress]", "true" },
        { "trace.path[@snapshot]", "false" },
        { "trace[@enable]", "false" },
        { "welcome.enable", "false" },
        { "home_mode.enable", "false" },
        { "feedback.show", "true" },
        { "overwrite_mode.enable", "true" },
#if ENABLE_FEATURE_LOCK
        { "feature_lock.locked_hosts[@allow]", "false" },
        { "feature_lock.locked_hosts.fallback[@read_only]", "false" },
        { "feature_lock.locked_hosts.fallback[@disabled_commands]", "false" },
        { "feature_lock.locked_hosts.host[0]", "localhost" },
        { "feature_lock.locked_hosts.host[0][@read_only]", "false" },
        { "feature_lock.locked_hosts.host[0][@disabled_commands]", "false" },
        { "feature_lock.is_lock_readonly", "false" },
        { "feature_lock.locked_commands", LOCKED_COMMANDS },
        { "feature_lock.unlock_title", UNLOCK_TITLE },
        { "feature_lock.unlock_link", UNLOCK_LINK },
        { "feature_lock.unlock_description", UNLOCK_DESCRIPTION },
        { "feature_lock.writer_unlock_highlights", WRITER_UNLOCK_HIGHLIGHTS },
        { "feature_lock.calc_unlock_highlights", CALC_UNLOCK_HIGHLIGHTS },
        { "feature_lock.impress_unlock_highlights", IMPRESS_UNLOCK_HIGHLIGHTS },
        { "feature_lock.draw_unlock_highlights", DRAW_UNLOCK_HIGHLIGHTS },
#endif
#if ENABLE_FEATURE_RESTRICTION
        { "restricted_commands", "" },
#endif
        { "user_interface.mode", "default" },
        { "user_interface.use_integration_theme", "true" },
        { "quarantine_files[@enable]", "false" },
        { "quarantine_files.limit_dir_size_mb", "250" },
        { "quarantine_files.max_versions_to_maintain", "2" },
        { "quarantine_files.path", "quarantine" },
        { "quarantine_files.expiry_min", "30" },
        { "remote_config.remote_url", "" },
        { "storage.wopi.alias_groups[@mode]", "first" },
        { "languagetool.base_url", "" },
        { "languagetool.api_key", "" },
        { "languagetool.user_name", "" },
        { "languagetool.enabled", "false" },
        { "languagetool.ssl_verification", "true" },
        { "languagetool.rest_protocol", "" },
        { "deepl.api_url", "" },
        { "deepl.auth_key", "" },
        { "deepl.enabled", "false" },
        { "zotero.enable", "true" },
        { "indirection_endpoint.url", "" },
#if !MOBILEAPP
        { "help_url", HELP_URL },
#endif
        { "product_name", APP_NAME },
        { "admin_console.logging.admin_login", "true" },
        { "admin_console.logging.metrics_fetch", "true" },
        { "admin_console.logging.monitor_connect", "true" },
        { "admin_console.logging.admin_action", "true" },
        { "wasm.enable", "false" },
        { "wasm.force", "false" },
    };

    // Set default values, in case they are missing from the config file.
    Poco::AutoPtr<AppConfigMap> defConfig(new AppConfigMap(DefAppConfig));
    conf.addWriteable(defConfig, PRIO_SYSTEM); // Lowest priority

#if !MOBILEAPP

    // Load default configuration files, with name independent
    // of Poco's view of app-name, from local file if present.
    Poco::Path configPath("loolwsd.xml");
    if (Application::findFile(configPath))
        loadConfiguration(configPath.toString(), PRIO_DEFAULT);
    else
    {
        // Fallback to the LOOLWSD_CONFIGDIR or --config-file path.
        loadConfiguration(ConfigFile, PRIO_DEFAULT);
    }

    // Load extra ("plug-in") configuration files, if present
    File dir(ConfigDir);
    if (dir.exists() && dir.isDirectory())
    {
        for (auto configFileIterator = DirectoryIterator(dir); configFileIterator != DirectoryIterator(); ++configFileIterator)
        {
            // Only accept configuration files ending in .xml
            const std::string configFile = configFileIterator.path().getFileName();
            if (configFile.length() > 4 && strcasecmp(configFile.substr(configFile.length() - 4).data(), ".xml") == 0)
            {
                const std::string fullFileName = dir.path() + "/" + configFile;
                PluginConfigurations.insert(new XMLConfiguration(fullFileName));
            }
        }
    }

    // Override any settings passed on the command-line or via environment variables
    if (UseEnvVarOptions)
        initializeEnvOptions();
    Poco::AutoPtr<AppConfigMap> overrideConfig(new AppConfigMap(_overrideSettings));
    conf.addWriteable(overrideConfig, PRIO_APPLICATION); // Highest priority

    if (!UnitTestLibrary.empty())
    {
        UnitWSD::defaultConfigure(conf);
    }

    // Experimental features.
    EnableExperimental = getConfigValue<bool>(conf, "experimental_features", false);

    EnableAccessibility = getConfigValue<bool>(conf, "accessibility.enable", false);

    // Setup user interface mode
    UserInterface = getConfigValue<std::string>(conf, "user_interface.mode", "default");

    if (UserInterface == "compact")
        UserInterface = "classic";

    if (UserInterface == "tabbed")
        UserInterface = "notebookbar";

    if (EnableAccessibility)
        UserInterface = "notebookbar";

    // Set the log-level after complete initialization to force maximum details at startup.
    LogLevel = getConfigValue<std::string>(conf, "logging.level", "trace");
    MostVerboseLogLevelSettableFromClient = getConfigValue<std::string>(conf, "logging.most_verbose_level_settable_from_client", "notice");
    LeastVerboseLogLevelSettableFromClient = getConfigValue<std::string>(conf, "logging.least_verbose_level_settable_from_client", "fatal");

    setenv("LOOL_LOGLEVEL", LogLevel.c_str(), true);

#if !ENABLE_DEBUG
    const std::string salLog = getConfigValue<std::string>(conf, "logging.lokit_sal_log", "-INFO-WARN");
    setenv("SAL_LOG", salLog.c_str(), 0);
#endif

#if WASMAPP
    // In WASM, we want to log to the Log Console.
    // Disable logging to file to log to stdout and
    // disable color since this isn't going to the terminal.
    constexpr bool withColor = false;
    constexpr bool logToFile = false;
#else
    const bool withColor = getConfigValue<bool>(conf, "logging.color", true) && isatty(fileno(stderr));
    if (withColor)
    {
        setenv("LOOL_LOGCOLOR", "1", true);
    }

    const auto logToFile = getConfigValue<bool>(conf, "logging.file[@enable]", false);
    std::map<std::string, std::string> logProperties;
    for (std::size_t i = 0; ; ++i)
    {
        const std::string confPath = "logging.file.property[" + std::to_string(i) + ']';
        const std::string confName = config().getString(confPath + "[@name]", "");
        if (!confName.empty())
        {
            const std::string value = config().getString(confPath, "");
            logProperties.emplace(confName, value);
        }
        else if (!config().has(confPath))
        {
            break;
        }
    }

    // Setup the logfile envar for the kit processes.
    if (logToFile)
    {
        const auto it = logProperties.find("path");
        if (it != logProperties.end())
        {
            setenv("LOOL_LOGFILE", "1", true);
            setenv("LOOL_LOGFILENAME", it->second.c_str(), true);
            std::cerr << "\nLogging at " << LogLevel << " level to file: " << it->second.c_str()
                      << std::endl;
        }
    }
#endif

    // Log at trace level until we complete the initialization.
    LogLevelStartup = getConfigValue<std::string>(conf, "logging.level_startup", "trace");
    setenv("LOOL_LOGLEVEL_STARTUP", LogLevelStartup.c_str(), true);

    Log::initialize("wsd", LogLevelStartup, withColor, logToFile, logProperties);
    if (LogLevel != LogLevelStartup)
    {
        LOG_INF("Setting log-level to [" << LogLevelStartup << "] and delaying setting to ["
                << LogLevel << "] until after WSD initialization.");
    }

    if (getConfigValue<bool>(conf, "browser_logging", false))
    {
        LogToken = Util::rng::getHexString(16);
    }

    // First log entry.
    ServerName = config().getString("server_name");
    LOG_INF("Initializing loolwsd server [" << ServerName << "]. Experimental features are "
                                            << (EnableExperimental ? "enabled." : "disabled."));


    // Initialize the UnitTest subsystem.
    if (!UnitWSD::init(UnitWSD::UnitType::Wsd, UnitTestLibrary))
    {
        throw std::runtime_error("Failed to load wsd unit test library.");
    }

    // Allow UT to manipulate before using configuration values.
    UnitWSD::get().configure(conf);

    // Trace Event Logging.
    EnableTraceEventLogging = getConfigValue<bool>(conf, "trace_event[@enable]", false);

    if (EnableTraceEventLogging)
    {
        const auto traceEventFile = getConfigValue<std::string>(conf, "trace_event.path", LOOLWSD_TRACEEVENTFILE);
        LOG_INF("Trace Event file is " << traceEventFile << ".");
        TraceEventFile = fopen(traceEventFile.c_str(), "w");
        if (TraceEventFile != NULL)
        {
            if (fcntl(fileno(TraceEventFile), F_SETFD, FD_CLOEXEC) == -1)
            {
                fclose(TraceEventFile);
                TraceEventFile = NULL;
            }
            else
            {
                fprintf(TraceEventFile, "[\n");
                // Output a metadata event that tells that this is the WSD process
                fprintf(TraceEventFile, "{\"name\":\"process_name\",\"ph\":\"M\",\"args\":{\"name\":\"WSD\"},\"pid\":%d,\"tid\":%ld},\n",
                        getpid(), (long) Util::getThreadId());
                fprintf(TraceEventFile, "{\"name\":\"thread_name\",\"ph\":\"M\",\"args\":{\"name\":\"Main\"},\"pid\":%d,\"tid\":%ld},\n",
                        getpid(), (long) Util::getThreadId());
            }
        }
    }

    // Check deprecated settings.
    bool reuseCookies = false;
    if (getSafeConfig(conf, "storage.wopi.reuse_cookies", reuseCookies))
        LOG_WRN("NOTE: Deprecated config option storage.wopi.reuse_cookies - no longer supported.");

#if !MOBILEAPP
    LOOLWSD::WASMState = getConfigValue<bool>(conf, "wasm.enable", false)
                             ? LOOLWSD::WASMActivationState::Enabled
                             : LOOLWSD::WASMActivationState::Disabled;

    if (getConfigValue<bool>(conf, "wasm.force", false))
    {
        if (LOOLWSD::WASMState != LOOLWSD::WASMActivationState::Enabled)
        {
            LOG_FTL(
                "WASM is not enabled; cannot force serving WASM. Please set wasm.enabled to true "
                "in loolwsd.xml first");
            Util::forcedExit(EX_SOFTWARE);
        }

        LOG_INF("WASM is force-enabled. All documents will be loaded through WASM");
        LOOLWSD::WASMState = LOOLWSD::WASMActivationState::Forced;
    }
#endif // !MOBILEAPP

    // Get anonymization settings.
#if LOOLWSD_ANONYMIZE_USER_DATA
    AnonymizeUserData = true;
    LOG_INF("Anonymization of user-data is permanently enabled.");
#else
    LOG_INF("Anonymization of user-data is configurable.");
    bool haveAnonymizeUserDataConfig = false;
    if (getSafeConfig(conf, "logging.anonymize.anonymize_user_data", AnonymizeUserData))
        haveAnonymizeUserDataConfig = true;

    bool anonymizeFilenames = false;
    bool anonymizeUsernames = false;
    if (getSafeConfig(conf, "logging.anonymize.usernames", anonymizeFilenames) ||
        getSafeConfig(conf, "logging.anonymize.filenames", anonymizeUsernames))
    {
        LOG_WRN("NOTE: both logging.anonymize.usernames and logging.anonymize.filenames are deprecated and superseded by "
                "logging.anonymize.anonymize_user_data. Please remove username and filename entries from the config and use only anonymize_user_data.");

        if (haveAnonymizeUserDataConfig)
            LOG_WRN("Since logging.anonymize.anonymize_user_data is provided (" << AnonymizeUserData << ") in the config, it will be used.");
        else
        {
            AnonymizeUserData = (anonymizeFilenames || anonymizeUsernames);
        }
    }
#endif

    if (AnonymizeUserData && LogLevel == "trace" && !CleanupOnly)
    {
        if (getConfigValue<bool>(conf, "logging.anonymize.allow_logging_user_data", false))
        {
            LOG_WRN("Enabling trace logging while anonymization is enabled due to logging.anonymize.allow_logging_user_data setting. "
                    "This will leak user-data!");

            // Disable anonymization as it's useless now.
            AnonymizeUserData = false;
        }
        else
        {
            static const char failure[] = "Anonymization and trace-level logging are incompatible. "
                "Please reduce logging level to debug or lower in loolwsd.xml to prevent leaking sensitive user data.";
            LOG_FTL(failure);
            std::cerr << '\n' << failure << std::endl;
#if ENABLE_DEBUG
            std::cerr << "\nIf you have used 'make run', edit loolwsd.xml and make sure you have removed "
                         "'--o:logging.level=trace' from the command line in Makefile.am.\n" << std::endl;
#endif
            Util::forcedExit(EX_SOFTWARE);
        }
    }

    std::uint64_t anonymizationSalt = 82589933;
    LOG_INF("Anonymization of user-data is " << (AnonymizeUserData ? "enabled." : "disabled."));
    if (AnonymizeUserData)
    {
        // Get the salt, if set, otherwise default, and set as envar, so the kits inherit it.
        anonymizationSalt = getConfigValue<std::uint64_t>(conf, "logging.anonymize.anonymization_salt", 82589933);
        const std::string anonymizationSaltStr = std::to_string(anonymizationSalt);
        setenv("LOOL_ANONYMIZATION_SALT", anonymizationSaltStr.c_str(), true);
    }
    FileUtil::setUrlAnonymization(AnonymizeUserData, anonymizationSalt);

    {
        bool enableWebsocketURP =
            LOOLWSD::getConfigValue<bool>("security.enable_websocket_urp", false);
        setenv("ENABLE_WEBSOCKET_URP", enableWebsocketURP ? "true" : "false", 1);
    }

    {
        std::string proto = getConfigValue<std::string>(conf, "net.proto", "");
        if (Util::iequal(proto, "ipv4"))
            ClientPortProto = Socket::Type::IPv4;
        else if (Util::iequal(proto, "ipv6"))
            ClientPortProto = Socket::Type::IPv6;
        else if (Util::iequal(proto, "all"))
            ClientPortProto = Socket::Type::All;
        else
            LOG_WRN("Invalid protocol: " << proto);
    }

    {
        std::string listen = getConfigValue<std::string>(conf, "net.listen", "");
        if (Util::iequal(listen, "any"))
            ClientListenAddr = ServerSocket::Type::Public;
        else if (Util::iequal(listen, "loopback"))
            ClientListenAddr = ServerSocket::Type::Local;
        else
            LOG_WRN("Invalid listen address: " << listen << ". Falling back to default: 'any'" );
    }

    // Prefix for the loolwsd pages; should not end with a '/'
    ServiceRoot = getPathFromConfig("net.service_root");
    while (ServiceRoot.length() > 0 && ServiceRoot[ServiceRoot.length() - 1] == '/')
        ServiceRoot.pop_back();

    IsProxyPrefixEnabled = getConfigValue<bool>(conf, "net.proxy_prefix", false);

#if ENABLE_SSL
    LOOLWSD::SSLEnabled.set(getConfigValue<bool>(conf, "ssl.enable", true));
    LOOLWSD::SSLTermination.set(getConfigValue<bool>(conf, "ssl.termination", true));
#endif

    LOG_INF("SSL support: SSL is " << (LOOLWSD::isSSLEnabled() ? "enabled." : "disabled."));
    LOG_INF("SSL support: termination is " << (LOOLWSD::isSSLTermination() ? "enabled." : "disabled."));

    std::string allowedLanguages(config().getString("allowed_languages"));
    // Core <= 7.0.
    setenv("LOK_WHITELIST_LANGUAGES", allowedLanguages.c_str(), 1);
    // Core >= 7.1.
    setenv("LOK_ALLOWLIST_LANGUAGES", allowedLanguages.c_str(), 1);

#endif

    int pdfResolution = getConfigValue<int>(conf, "per_document.pdf_resolution_dpi", 96);
    if (pdfResolution > 0)
    {
        constexpr int MaxPdfResolutionDpi = 384;
        if (pdfResolution > MaxPdfResolutionDpi)
        {
            // Avoid excessive memory consumption.
            LOG_WRN("The PDF resolution specified in per_document.pdf_resolution_dpi ("
                    << pdfResolution << ") is larger than the maximum (" << MaxPdfResolutionDpi
                    << "). Using " << MaxPdfResolutionDpi << " instead.");

            pdfResolution = MaxPdfResolutionDpi;
        }

        const std::string pdfResolutionStr = std::to_string(pdfResolution);
        LOG_DBG("Setting envar PDFIMPORT_RESOLUTION_DPI="
                << pdfResolutionStr << " per config per_document.pdf_resolution_dpi");
        ::setenv("PDFIMPORT_RESOLUTION_DPI", pdfResolutionStr.c_str(), 1);
    }

    SysTemplate = getPathFromConfig("sys_template_path");
    if (SysTemplate.empty())
    {
        LOG_FTL("Missing sys_template_path config entry.");
        throw MissingOptionException("systemplate");
    }

    ChildRoot = getPathFromConfig("child_root_path");
    if (ChildRoot.empty())
    {
        LOG_FTL("Missing child_root_path config entry.");
        throw MissingOptionException("childroot");
    }
    else
    {
#if !MOBILEAPP
        if (CleanupOnly)
        {
            // Cleanup and exit.
            JailUtil::cleanupJails(ChildRoot);
            Util::forcedExit(EX_OK);
        }
#endif
        if (ChildRoot[ChildRoot.size() - 1] != '/')
            ChildRoot += '/';

#if CODE_COVERAGE
        ::setenv("BASE_CHILD_ROOT", Poco::Path(ChildRoot).absolute().toString().c_str(), 1);
#endif

        // We need to cleanup other people's expired jails
        CleanupChildRoot = ChildRoot;

        // Encode the process id into the path for parallel re-use of jails/
        ChildRoot += std::to_string(getpid()) + '-' + Util::rng::getHexString(8) + '/';

        LOG_INF("Creating childroot: " + ChildRoot);
    }

#if !MOBILEAPP

    // Copy and serialize the config into XML to pass to forkit.
    KitXmlConfig.reset(new Poco::Util::XMLConfiguration);
    for (const auto& pair : DefAppConfig)
    {
        try
        {
            KitXmlConfig->setString(pair.first, config().getRawString(pair.first));
        }
        catch (const std::exception&)
        {
            // Nothing to do.
        }
    }

    // Fixup some config entries to match out decisions/overrides.
    KitXmlConfig->setBool("ssl.enable", isSSLEnabled());
    KitXmlConfig->setBool("ssl.termination", isSSLTermination());

    // We don't pass the config via command-line
    // to avoid dealing with escaping and other traps.
    std::ostringstream oss;
    KitXmlConfig->save(oss);
    setenv("LOOL_CONFIG", oss.str().c_str(), true);

    // Initialize the config subsystem too.
    config::initialize(&config());

    // For some reason I can't get at this setting in ChildSession::loKitCallback().
    std::string fontsMissingHandling = config::getString("fonts_missing.handling", "log");
    setenv("FONTS_MISSING_HANDLING", fontsMissingHandling.c_str(), 1);

    IsBindMountingEnabled = getConfigValue<bool>(conf, "mount_jail_tree", true);
#if CODE_COVERAGE
    // Code coverage is not supported with bind-mounting.
    if (IsBindMountingEnabled)
    {
        LOG_WRN("Mounting is not compatible with code-coverage. Disabling.");
        IsBindMountingEnabled = false;
    }
#endif // CODE_COVERAGE

    // Setup the jails.
    JailUtil::cleanupJails(CleanupChildRoot);
    JailUtil::setupChildRoot(IsBindMountingEnabled, ChildRoot, SysTemplate);

    LOG_DBG("FileServerRoot before config: " << FileServerRoot);
    FileServerRoot = getPathFromConfig("file_server_root_path");
    LOG_DBG("FileServerRoot after config: " << FileServerRoot);

    //creating quarantine directory
    if (getConfigValue<bool>(conf, "quarantine_files[@enable]", false))
    {
        std::string path = Util::trimmed(getPathFromConfig("quarantine_files.path"));
        LOG_INF("Quarantine path is set to [" << path << "] in config");
        if (path.empty())
        {
            LOG_WRN("Quarantining is enabled via quarantine_files config, but no path is set in "
                    "quarantine_files.path. Disabling quarantine");
        }
        else
        {
            if (path[path.size() - 1] != '/')
                path += '/';

            if (path[0] != '/')
                LOG_WRN("Quarantine path is relative. Please use an absolute path for better "
                        "reliability");

            Poco::File p(path);
            try
            {
                LOG_TRC("Creating quarantine directory [" + path << ']');
                p.createDirectories();

                LOG_DBG("Created quarantine directory [" + path << ']');
            }
            catch (const std::exception& ex)
            {
                LOG_WRN("Failed to create quarantine directory [" << path
                                                                  << "]. Disabling quaratine");
            }

            if (FileUtil::Stat(path).exists())
            {
                LOG_INF("Initializing quarantine at [" + path << ']');
                Quarantine::initialize(path);
            }
        }
    }
    else
    {
        LOG_INF("Quarantine is disabled in config");
    }

    NumPreSpawnedChildren = getConfigValue<int>(conf, "num_prespawn_children", 1);
    if (NumPreSpawnedChildren < 1)
    {
        LOG_WRN("Invalid num_prespawn_children in config (" << NumPreSpawnedChildren << "). Resetting to 1.");
        NumPreSpawnedChildren = 1;
    }
    LOG_INF("NumPreSpawnedChildren set to " << NumPreSpawnedChildren << '.');

    FileUtil::registerFileSystemForDiskSpaceChecks(ChildRoot);

    int nThreads = std::max<int>(std::thread::hardware_concurrency(), 1);
    int maxConcurrency = getConfigValue<int>(conf, "per_document.max_concurrency", 4);

    if (maxConcurrency > 16)
    {
        LOG_WRN("Using a large number of threads for every document puts pressure on "
                "the scheduler, and consumes memory, while providing marginal gains "
                "consider lowering max_concurrency from " << maxConcurrency);
    }
    if (maxConcurrency > nThreads)
    {
        LOG_ERR("Setting concurrency above the number of physical "
                "threads yields extra latency and memory usage for no benefit. "
                "Clamping " << maxConcurrency << " to " << nThreads << " threads.");
        maxConcurrency = nThreads;
    }
    if (maxConcurrency > 0)
    {
        setenv("MAX_CONCURRENCY", std::to_string(maxConcurrency).c_str(), 1);
    }
    LOG_INF("MAX_CONCURRENCY set to " << maxConcurrency << '.');
#elif defined(__EMSCRIPTEN__)
    // disable threaded image scaling for wasm for now
    setenv("VCL_NO_THREAD_SCALE", "1", 1);
#endif

    const auto redlining = getConfigValue<bool>(conf, "per_document.redlining_as_comments", false);
    if (!redlining)
    {
        setenv("DISABLE_REDLINE", "1", 1);
        LOG_INF("DISABLE_REDLINE set");
    }

    // Otherwise we profile the soft-device at jail creation time.
    setenv("SAL_DISABLE_OPENCL", "true", 1);
    // Disable getting the OS print queue and default printer
    setenv("SAL_DISABLE_PRINTERLIST", "true", 1);
    setenv("SAL_DISABLE_DEFAULTPRINTER", "true", 1);

    // Log the connection and document limits.
#if ENABLE_WELCOME_MESSAGE
    if (getConfigValue<bool>(conf, "home_mode.enable", false))
    {
        LOOLWSD::MaxConnections = 20;
        LOOLWSD::MaxDocuments = 10;
    }
    else
    {
        conf.setString("feedback.show", "true");
        conf.setString("welcome.enable", "true");
        LOOLWSD::MaxConnections = MAX_CONNECTIONS;
        LOOLWSD::MaxDocuments = MAX_DOCUMENTS;
    }
#else
    {
        LOOLWSD::MaxConnections = MAX_CONNECTIONS;
        LOOLWSD::MaxDocuments = MAX_DOCUMENTS;
    }
#endif

#if !MOBILEAPP
    NoSeccomp = Util::isKitInProcess() || !getConfigValue<bool>(conf, "security.seccomp", true);
    NoCapsForKit =
        Util::isKitInProcess() || !getConfigValue<bool>(conf, "security.capabilities", true);
    AdminEnabled = getConfigValue<bool>(conf, "admin_console.enable", true);
#if ENABLE_DEBUG
    if (Util::isKitInProcess())
        SingleKit = true;
#endif
#endif

    // LanguageTool configuration
    bool enableLanguageTool = getConfigValue<bool>(conf, "languagetool.enabled", false);
    setenv("LANGUAGETOOL_ENABLED", enableLanguageTool ? "true" : "false", 1);
    const std::string baseAPIUrl = getConfigValue<std::string>(conf, "languagetool.base_url", "");
    setenv("LANGUAGETOOL_BASEURL", baseAPIUrl.c_str(), 1);
    const std::string userName = getConfigValue<std::string>(conf, "languagetool.user_name", "");
    setenv("LANGUAGETOOL_USERNAME", userName.c_str(), 1);
    const std::string apiKey = getConfigValue<std::string>(conf, "languagetool.api_key", "");
    setenv("LANGUAGETOOL_APIKEY", apiKey.c_str(), 1);
    bool sslVerification = getConfigValue<bool>(conf, "languagetool.ssl_verification", true);
    setenv("LANGUAGETOOL_SSL_VERIFICATION", sslVerification ? "true" : "false", 1);
    const std::string restProtocol = getConfigValue<std::string>(conf, "languagetool.rest_protocol", "");
    setenv("LANGUAGETOOL_RESTPROTOCOL", restProtocol.c_str(), 1);

    // DeepL configuration
    const std::string apiURL = getConfigValue<std::string>(conf, "deepl.api_url", "");
    const std::string authKey = getConfigValue<std::string>(conf, "deepl.auth_key", "");
    setenv("DEEPL_API_URL", apiURL.c_str(), 1);
    setenv("DEEPL_AUTH_KEY", authKey.c_str(), 1);

#if !MOBILEAPP
    const std::string helpUrl = getConfigValue<std::string>(conf, "help_url", HELP_URL);
    setenv("LOK_HELP_URL", helpUrl.c_str(), 1);
#else
    // On mobile UI there should be no tunnelled dialogs. But if there are some, by mistake,
    // at least they should not have a non-working Help button.
    setenv("LOK_HELP_URL", "", 1);
#endif

    if (config::isSupportKeyEnabled())
    {
        const std::string supportKeyString = getConfigValue<std::string>(conf, "support_key", "");

        if (supportKeyString.empty())
        {
            LOG_WRN("Support key not set, please use 'loolconfig set-support-key'.");
            std::cerr << "Support key not set, please use 'loolconfig set-support-key'." << std::endl;
            LOOLWSD::OverrideWatermark = "Unsupported, the support key is missing.";
        }
        else
        {
            SupportKey key(supportKeyString);

            if (!key.verify())
            {
                LOG_WRN("Invalid support key, please use 'loolconfig set-support-key'.");
                std::cerr << "Invalid support key, please use 'loolconfig set-support-key'." << std::endl;
                LOOLWSD::OverrideWatermark = "Unsupported, the support key is invalid.";
            }
            else
            {
                int validDays =  key.validDaysRemaining();
                if (validDays <= 0)
                {
                    LOG_WRN("Your support key has expired, please ask for a new one, and use 'loolconfig set-support-key'.");
                    std::cerr << "Your support key has expired, please ask for a new one, and use 'loolconfig set-support-key'." << std::endl;
                    LOOLWSD::OverrideWatermark = "Unsupported, the support key has expired.";
                }
                else
                {
                    LOG_INF("Your support key is valid for " << validDays << " days");
                    LOOLWSD::MaxConnections = 1000;
                    LOOLWSD::MaxDocuments = 200;
                    LOOLWSD::OverrideWatermark.clear();
                }
            }
        }
    }

    if (LOOLWSD::MaxConnections < 3)
    {
        LOG_ERR("MAX_CONNECTIONS must be at least 3");
        LOOLWSD::MaxConnections = 3;
    }

    if (LOOLWSD::MaxDocuments > LOOLWSD::MaxConnections)
    {
        LOG_ERR("MAX_DOCUMENTS cannot be bigger than MAX_CONNECTIONS");
        LOOLWSD::MaxDocuments = LOOLWSD::MaxConnections;
    }

#if !WASMAPP
    struct rlimit rlim;
    ::getrlimit(RLIMIT_NOFILE, &rlim);
    LOG_INF("Maximum file descriptor supported by the system: " << rlim.rlim_cur - 1);
    // 4 fds per document are used for client connection, Kit process communication, and
    // a wakeup pipe with 2 fds. 32 fds (i.e. 8 documents) are reserved.
    LOG_INF("Maximum number of open documents supported by the system: " << rlim.rlim_cur / 4 - 8);
#endif

    LOG_INF("Maximum concurrent open Documents limit: " << LOOLWSD::MaxDocuments);
    LOG_INF("Maximum concurrent client Connections limit: " << LOOLWSD::MaxConnections);

    LOOLWSD::NumConnections = 0;

    // Command Tracing.
    if (getConfigValue<bool>(conf, "trace[@enable]", false))
    {
        const auto& path = getConfigValue<std::string>(conf, "trace.path", "");
        const auto recordOutgoing = getConfigValue<bool>(conf, "trace.outgoing.record", false);
        std::vector<std::string> filters;
        for (size_t i = 0; ; ++i)
        {
            const std::string confPath = "trace.filter.message[" + std::to_string(i) + ']';
            const std::string regex = config().getString(confPath, "");
            if (!regex.empty())
            {
                filters.push_back(regex);
            }
            else if (!config().has(confPath))
            {
                break;
            }
        }

        const auto compress = getConfigValue<bool>(conf, "trace.path[@compress]", false);
        const auto takeSnapshot = getConfigValue<bool>(conf, "trace.path[@snapshot]", false);
        TraceDumper = std::make_unique<TraceFileWriter>(path, recordOutgoing, compress,
                                                        takeSnapshot, filters);
    }

    // Allowed hosts for being external data source in the documents
    std::vector<std::string> lokAllowedHosts;
    appendAllowedHostsFrom(conf, "net.lok_allow", lokAllowedHosts);
    // For backward compatibility post_allow hosts are also allowed
    bool postAllowed = conf.getBool("net.post_allow[@allow]", false);
    if (postAllowed)
        appendAllowedHostsFrom(conf, "net.post_allow", lokAllowedHosts);
    // For backward compatibility wopi hosts are also allowed
    bool wopiAllowed = conf.getBool("storage.wopi[@allow]", false);
    if (wopiAllowed)
    {
        appendAllowedHostsFrom(conf, "storage.wopi", lokAllowedHosts);
        appendAllowedAliasGroups(conf, lokAllowedHosts);
    }

    if (lokAllowedHosts.size())
    {
        std::string allowedRegex;
        for (size_t i = 0; i < lokAllowedHosts.size(); i++)
        {
            if (isValidRegex(lokAllowedHosts[i]))
                allowedRegex += (i != 0 ? "|" : "") + lokAllowedHosts[i];
            else
                LOG_ERR("Invalid regular expression for allowed host: \"" << lokAllowedHosts[i] << "\"");
        }

        setenv("LOK_HOST_ALLOWLIST", allowedRegex.c_str(), true);
    }

#if !MOBILEAPP
    SavedClipboards = std::make_unique<ClipboardCache>();

    LOG_TRC("Initialize FileServerRequestHandler");
    LOOLWSD::FileRequestHandler =
        std::make_unique<FileServerRequestHandler>(LOOLWSD::FileServerRoot);
#endif

    WebServerPoll = std::make_unique<TerminatingPoll>("websrv_poll");

    PrisonerPoll = std::make_unique<PrisonPoll>();

    Server = std::make_unique<LOOLWSDServer>();

    LOG_TRC("Initialize StorageBase");
    StorageBase::initialize();

#if !MOBILEAPP
    // Check for smaps_rollup bug where rewinding and rereading gives
    // bogus doubled results
    if (FILE* fp = fopen("/proc/self/smaps_rollup", "r"))
    {
        std::size_t memoryDirty1 = Util::getPssAndDirtyFromSMaps(fp).second;
        (void)Util::getPssAndDirtyFromSMaps(fp); // interleave another rewind+read to margin
        std::size_t memoryDirty2 = Util::getPssAndDirtyFromSMaps(fp).second;
        LOG_TRC("Comparing smaps_rollup read and rewind+read: " << memoryDirty1 << " vs " << memoryDirty2);
        if (memoryDirty2 >= memoryDirty1 * 2)
        {
            // Believed to be fixed in >= v4.19, bug seen in 4.15.0 and not in 6.5.10
            // https://github.com/torvalds/linux/commit/258f669e7e88c18edbc23fe5ce00a476b924551f
            LOG_WRN("Reading smaps_rollup twice reports Private_Dirty doubled, smaps_rollup is unreliable on this kernel");
            setenv("LOOL_DISABLE_SMAPS_ROLLUP", "1", true);
        }
        fclose(fp);
    }

    ServerApplication::initialize(self);

    DocProcSettings docProcSettings;
    docProcSettings.setLimitVirtMemMb(getConfigValue<int>("per_document.limit_virt_mem_mb", 0));
    docProcSettings.setLimitStackMemKb(getConfigValue<int>("per_document.limit_stack_mem_kb", 0));
    docProcSettings.setLimitFileSizeMb(getConfigValue<int>("per_document.limit_file_size_mb", 0));
    docProcSettings.setLimitNumberOpenFiles(getConfigValue<int>("per_document.limit_num_open_files", 0));

    DocCleanupSettings &docCleanupSettings = docProcSettings.getCleanupSettings();
    docCleanupSettings.setEnable(getConfigValue<bool>("per_document.cleanup[@enable]", false));
    docCleanupSettings.setCleanupInterval(getConfigValue<int>("per_document.cleanup.cleanup_interval_ms", 10000));
    docCleanupSettings.setBadBehaviorPeriod(getConfigValue<int>("per_document.cleanup.bad_behavior_period_secs", 60));
    docCleanupSettings.setIdleTime(getConfigValue<int>("per_document.cleanup.idle_time_secs", 300));
    docCleanupSettings.setLimitDirtyMem(getConfigValue<int>("per_document.cleanup.limit_dirty_mem_mb", 3072));
    docCleanupSettings.setLimitCpu(getConfigValue<int>("per_document.cleanup.limit_cpu_per", 85));
    docCleanupSettings.setLostKitGracePeriod(getConfigValue<int>("per_document.cleanup.lost_kit_grace_period_secs", 120));

    Admin::instance().setDefDocProcSettings(docProcSettings, false);

#else
    (void) self;
#endif
}

void LOOLWSD::initializeSSL()
{
#if ENABLE_SSL
    if (!LOOLWSD::isSSLEnabled())
        return;

    const std::string ssl_cert_file_path = getPathFromConfig("ssl.cert_file_path");
    LOG_INF("SSL Cert file: " << ssl_cert_file_path);

    const std::string ssl_key_file_path = getPathFromConfig("ssl.key_file_path");
    LOG_INF("SSL Key file: " << ssl_key_file_path);

    const std::string ssl_ca_file_path = getPathFromConfig("ssl.ca_file_path");
    LOG_INF("SSL CA file: " << ssl_ca_file_path);

    std::string ssl_cipher_list = config().getString("ssl.cipher_list", "");
    if (ssl_cipher_list.empty())
            ssl_cipher_list = DEFAULT_CIPHER_SET;
    LOG_INF("SSL Cipher list: " << ssl_cipher_list);

    // Initialize the non-blocking server socket SSL context.
    ssl::Manager::initializeServerContext(ssl_cert_file_path, ssl_key_file_path, ssl_ca_file_path,
                                          ssl_cipher_list, ssl::CertificateVerification::Disabled);

    if (!ssl::Manager::isServerContextInitialized())
        LOG_ERR("Failed to initialize Server SSL.");
    else
        LOG_INF("Initialized Server SSL.");
#else
    LOG_INF("SSL is unavailable in this build.");
#endif
}

void LOOLWSD::dumpNewSessionTrace(const std::string& id, const std::string& sessionId, const std::string& uri, const std::string& path)
{
    if (TraceDumper)
    {
        try
        {
            TraceDumper->newSession(id, sessionId, uri, path);
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Exception in tracer newSession: " << exc.what());
        }
    }
}

void LOOLWSD::dumpEndSessionTrace(const std::string& id, const std::string& sessionId, const std::string& uri)
{
    if (TraceDumper)
    {
        try
        {
            TraceDumper->endSession(id, sessionId, uri);
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Exception in tracer newSession: " << exc.what());
        }
    }
}

void LOOLWSD::dumpEventTrace(const std::string& id, const std::string& sessionId, const std::string& data)
{
    if (TraceDumper)
    {
        TraceDumper->writeEvent(id, sessionId, data);
    }
}

void LOOLWSD::dumpIncomingTrace(const std::string& id, const std::string& sessionId, const std::string& data)
{
    if (TraceDumper)
    {
        TraceDumper->writeIncoming(id, sessionId, data);
    }
}

void LOOLWSD::dumpOutgoingTrace(const std::string& id, const std::string& sessionId, const std::string& data)
{
    if (TraceDumper)
    {
        TraceDumper->writeOutgoing(id, sessionId, data);
    }
}

void LOOLWSD::defineOptions(OptionSet& optionSet)
{
    if (Util::isMobileApp())
        return;
    ServerApplication::defineOptions(optionSet);

    optionSet.addOption(Option("help", "", "Display help information on command line arguments.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("version-hash", "", "Display product version-hash information and exit.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("version", "", "Display version and hash information.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("cleanup", "", "Cleanup jails and other temporary data and exit.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("port", "", "Port number to listen to (default: " +
                               std::to_string(DEFAULT_CLIENT_PORT_NUMBER) + "),")
                        .required(false)
                        .repeatable(false)
                        .argument("port_number"));

    optionSet.addOption(Option("disable-ssl", "", "Disable SSL security layer.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("disable-lool-user-checking", "", "Don't check whether loolwsd is running under the user 'lool'.  NOTE: This is insecure, use only when you know what you are doing!")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("override", "o", "Override any setting by providing full xmlpath=value.")
                        .required(false)
                        .repeatable(true)
                        .argument("xmlpath"));

    optionSet.addOption(Option("config-file", "", "Override configuration file path.")
                        .required(false)
                        .repeatable(false)
                        .argument("path"));

    optionSet.addOption(Option("config-dir", "", "Override extra configuration directory path.")
                        .required(false)
                        .repeatable(false)
                        .argument("path"));

    optionSet.addOption(Option("lo-template-path", "", "Override the LOK core installation directory path.")
                        .required(false)
                        .repeatable(false)
                        .argument("path"));

    optionSet.addOption(Option("unattended", "", "Unattended run, won't wait for a debugger on faulting.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("signal", "", "Send signal SIGUSR2 to parent process when server is ready to accept connections")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("use-env-vars", "",
                               "Use the environment variables defined on "
                               "https://sdk.collaboraonline.com/docs/installation/"
                               "CODE_Docker_image.html#setting-the-application-configuration-"
                               "dynamically-via-environment-variables to set options. "
                               "'DONT_GEN_SSL_CERT' is forcibly enabled and 'extra_params' is "
                               "ignored even when using this option.")
                            .required(false)
                            .repeatable(false));

#if ENABLE_DEBUG
    optionSet.addOption(Option("unitlib", "", "Unit testing library path.")
                        .required(false)
                        .repeatable(false)
                        .argument("unitlib"));

    optionSet.addOption(Option("careerspan", "", "How many seconds to run.")
                        .required(false)
                        .repeatable(false)
                        .argument("seconds"));

    optionSet.addOption(Option("singlekit", "", "Spawn one libreoffice kit.")
                        .required(false)
                        .repeatable(false));

    optionSet.addOption(Option("forcecaching", "", "Force HTML & asset caching even in debug mode: accelerates cypress.")
                        .required(false)
                        .repeatable(false));
#endif
}

void LOOLWSD::handleOption(const std::string& optionName,
                           const std::string& value)
{
#if !MOBILEAPP
    ServerApplication::handleOption(optionName, value);

    if (optionName == "help")
    {
        displayHelp();
        Util::forcedExit(EX_OK);
    }
    else if (optionName == "version-hash")
    {
        std::string version, hash;
        Util::getVersionInfo(version, hash);
        std::cout << hash << std::endl;
        Util::forcedExit(EX_OK);
    }
    else if (optionName == "version")
        ; // ignore for compatibility
    else if (optionName == "cleanup")
        CleanupOnly = true; // Flag for later as we need the config.
    else if (optionName == "port")
        ClientPortNumber = std::stoi(value);
    else if (optionName == "disable-ssl")
        _overrideSettings["ssl.enable"] = "false";
    else if (optionName == "disable-lool-user-checking")
        CheckLoolUser = false;
    else if (optionName == "override")
    {
        std::string optName;
        std::string optValue;
        LOOLProtocol::parseNameValuePair(value, optName, optValue);
        _overrideSettings[optName] = optValue;
    }
    else if (optionName == "config-file")
        ConfigFile = value;
    else if (optionName == "config-dir")
        ConfigDir = value;
    else if (optionName == "lo-template-path")
        LoTemplate = value;
    else if (optionName == "signal")
        SignalParent = true;
    else if (optionName == "use-env-vars")
        UseEnvVarOptions = true;

#if ENABLE_DEBUG
    else if (optionName == "unitlib")
        UnitTestLibrary = value;
    else if (optionName == "unattended")
    {
        UnattendedRun = true;
        SigUtil::setUnattended();
    }
    else if (optionName == "careerspan")
        careerSpanMs = std::chrono::seconds(std::stoi(value)); // Convert second to ms
    else if (optionName == "singlekit")
    {
        SingleKit = true;
        NumPreSpawnedChildren = 1;
    }
    else if (optionName == "forcecaching")
        ForceCaching = true;

    static const char* latencyMs = std::getenv("LOOL_DELAY_SOCKET_MS");
    if (latencyMs)
        SimulatedLatencyMs = std::stoi(latencyMs);
#endif

#else
    (void) optionName;
    (void) value;
#endif
}

void LOOLWSD::initializeEnvOptions()
{
    int n = 0;
    char* aliasGroup;
    while ((aliasGroup = std::getenv(("aliasgroup" + std::to_string(n + 1)).c_str())) != nullptr)
    {
        bool first = true;
        std::istringstream aliasGroupStream;
        aliasGroupStream.str(aliasGroup);
        for (std::string alias; std::getline(aliasGroupStream, alias, ',');)
        {
            if (first)
            {
                const std::string path = "storage.wopi.alias_groups.group[" + std::to_string(n) + "].host";
                _overrideSettings[path] = alias;
                _overrideSettings[path + "[@allow]"] = "true";
                first = false;
            }
            else
            {
                _overrideSettings["storage.wopi.alias_groups.group[" + std::to_string(n) +
                                  "].alias"] = alias;
            }
        }

        n++;
    }
    if (n >= 1)
    {
        _overrideSettings["alias_groups[@mode]"] = "groups";
    }

    char* optionValue;
    if ((optionValue = std::getenv("username")) != nullptr) _overrideSettings["admin_console.username"] = optionValue;
    if ((optionValue = std::getenv("password")) != nullptr) _overrideSettings["admin_console.password"] = optionValue;
    if ((optionValue = std::getenv("server_name")) != nullptr) _overrideSettings["server_name"] = optionValue;
    if ((optionValue = std::getenv("dictionaries")) != nullptr) _overrideSettings["allowed_languages"] = optionValue;
    if ((optionValue = std::getenv("remoteconfigurl")) != nullptr) _overrideSettings["remote_config.remote_url"] = optionValue;
}

#if !MOBILEAPP

void LOOLWSD::displayHelp()
{
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("Free Online Office WebSocket server.");
    helpFormatter.format(std::cout);
}

bool LOOLWSD::checkAndRestoreForKit()
{
// clang issues warning for WIF*() macro usages below:
// "equality comparison with extraneous parentheses [-Werror,-Wparentheses-equality]"
// https://bugs.llvm.org/show_bug.cgi?id=22949

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wparentheses-equality"
#endif

    if (ForKitProcId == -1)
    {
        // Fire the ForKit process for the first time.
        if (!SigUtil::getShutdownRequestFlag() && !createForKit())
        {
            // Should never fail.
            LOG_FTL("Setting ShutdownRequestFlag: Failed to spawn loolforkit.");
            SigUtil::requestShutdown();
        }
    }

    if (Util::isKitInProcess())
        return true;

    int status;
    const pid_t pid = waitpid(ForKitProcId, &status, WUNTRACED | WNOHANG);
    if (pid > 0)
    {
        if (pid == ForKitProcId)
        {
            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                if (WIFEXITED(status))
                {
                    LOG_INF("Forkit process [" << pid << "] exited with code: " <<
                            WEXITSTATUS(status) << '.');
                }
                else
                {
                    LOG_ERR("Forkit process [" << pid << "] " <<
                            (WCOREDUMP(status) ? "core-dumped" : "died") <<
                            " with " << SigUtil::signalName(WTERMSIG(status)));
                }

                // Spawn a new forkit and try to dust it off and resume.
                if (!SigUtil::getShutdownRequestFlag() && !createForKit())
                {
                    LOG_FTL("Setting ShutdownRequestFlag: Failed to spawn forkit instance.");
                    SigUtil::requestShutdown();
                }
            }
            else if (WIFSTOPPED(status))
            {
                LOG_INF("Forkit process [" << pid << "] stopped with " <<
                        SigUtil::signalName(WSTOPSIG(status)));
            }
            else if (WIFCONTINUED(status))
            {
                LOG_INF("Forkit process [" << pid << "] resumed with SIGCONT.");
            }
            else
            {
                LOG_WRN("Unknown status returned by waitpid: " << std::hex << status << std::dec);
            }

            return true;
        }
        else
        {
            LOG_ERR("An unknown child process [" << pid << "] died.");
        }
    }
    else if (pid < 0)
    {
        LOG_SYS("Forkit waitpid failed");
        if (errno == ECHILD)
        {
            // No child processes.
            // Spawn a new forkit and try to dust it off and resume.
            if (!SigUtil::getShutdownRequestFlag()  && !createForKit())
            {
                LOG_FTL("Setting ShutdownRequestFlag: Failed to spawn forkit instance.");
                SigUtil::requestShutdown();
            }
        }

        return true;
    }

    return false;

#if defined __clang__
#pragma clang diagnostic pop
#endif
}

#endif

void LOOLWSD::doHousekeeping()
{
    if (PrisonerPoll)
    {
        PrisonerPoll->wakeup();
    }
}

void LOOLWSD::closeDocument(const std::string& docKey, const std::string& message)
{
    std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
    auto docBrokerIt = DocBrokers.find(docKey);
    if (docBrokerIt != DocBrokers.end())
    {
        std::shared_ptr<DocumentBroker> docBroker = docBrokerIt->second;
        docBroker->addCallback([docBroker, message]() {
                docBroker->closeDocument(message);
            });
    }
}

void LOOLWSD::autoSave(const std::string& docKey)
{
    std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
    auto docBrokerIt = DocBrokers.find(docKey);
    if (docBrokerIt != DocBrokers.end())
    {
        std::shared_ptr<DocumentBroker> docBroker = docBrokerIt->second;
        docBroker->addCallback(
            [docBroker]() { docBroker->autoSave(/*force=*/true, /*dontSaveIfUnmodified=*/true); });
    }
}

void LOOLWSD::setLogLevelsOfKits(const std::string& level)
{
    std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);

    LOG_INF("Changing kits' log levels: [" << level << ']');

    for (const auto& brokerIt : DocBrokers)
    {
        std::shared_ptr<DocumentBroker> docBroker = brokerIt.second;
        docBroker->addCallback([docBroker, level]() {
            docBroker->setKitLogLevel(level);
        });
    }
}

/// Really do the house-keeping
void PrisonPoll::wakeupHook()
{
#if !MOBILEAPP
    LOG_TRC("PrisonerPoll - wakes up with " << NewChildren.size() <<
            " new children and " << DocBrokers.size() << " brokers and " <<
            OutstandingForks << " kits forking");

    if (!LOOLWSD::checkAndRestoreForKit())
    {
        // No children have died.
        // Make sure we have sufficient reserves.
        prespawnChildren();
    }
#endif
    std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex, std::defer_lock);
    if (docBrokersLock.try_lock())
    {
        cleanupDocBrokers();
        SigUtil::checkForwardSigUsr2(forwardSigUsr2);
    }
}

#if !MOBILEAPP

bool LOOLWSD::createForKit()
{
    LOG_INF("Creating new forkit process.");

    // Creating a new forkit is always a slow process.
    ChildSpawnTimeoutMs = CHILD_TIMEOUT_MS * 4;

    std::unique_lock<std::mutex> newChildrenLock(NewChildrenMutex);

    StringVector args;
    std::string parentPath = Path(Application::instance().commandPath()).parent().toString();

#if STRACE_LOOLFORKIT
    // if you want to use this, you need to sudo setcap cap_fowner,cap_chown,cap_mknod,cap_sys_chroot=ep /usr/bin/strace
    args.push_back("-o");
    args.push_back("strace.log");
    args.push_back("-f");
    args.push_back("-tt");
    args.push_back("-s");
    args.push_back("256");
    args.push_back(parentPath + "loolforkit");
#elif VALGRIND_LOOLFORKIT
    NoCapsForKit = true;
    NoSeccomp = true;
//    args.push_back("--log-file=valgrind.log");
//    args.push_back("--track-fds=all");

//  for massif: can connect with (gdb) target remote | vgdb
//  and then monitor snapshot <filename> before kit exit
//    args.push_back("--tool=massif");
//    args.push_back("--vgdb=yes");
//    args.push_back("--vgdb-error=0");

    args.push_back("--trace-children=yes");
    args.push_back("--error-limit=no");
    args.push_back("--num-callers=128");
    std::string nocapsCopy = parentPath + "loolforkit-nocaps";
    FileUtil::copy(parentPath + "loolforkit", nocapsCopy, true, true);
    args.push_back(nocapsCopy);
#endif
    args.push_back("--systemplate=" + SysTemplate);
    args.push_back("--lotemplate=" + LoTemplate);
    args.push_back("--childroot=" + ChildRoot);
    args.push_back("--clientport=" + std::to_string(ClientPortNumber));
    args.push_back("--masterport=" + MasterLocation);

    const DocProcSettings& docProcSettings = Admin::instance().getDefDocProcSettings();
    std::ostringstream ossRLimits;
    ossRLimits << "limit_virt_mem_mb:" << docProcSettings.getLimitVirtMemMb();
    ossRLimits << ";limit_stack_mem_kb:" << docProcSettings.getLimitStackMemKb();
    ossRLimits << ";limit_file_size_mb:" << docProcSettings.getLimitFileSizeMb();
    ossRLimits << ";limit_num_open_files:" << docProcSettings.getLimitNumberOpenFiles();
    args.push_back("--rlimits=" + ossRLimits.str());

    if (UnitWSD::get().hasKitHooks())
        args.push_back("--unitlib=" + UnitTestLibrary);

    args.push_back("--version");

    if (NoCapsForKit)
        args.push_back("--nocaps");

    if (NoSeccomp)
        args.push_back("--noseccomp");

    args.push_back("--ui=" + UserInterface);

    if (!CheckLoolUser)
        args.push_back("--disable-lool-user-checking");

    if (UnattendedRun)
        args.push_back("--unattended");

#if ENABLE_DEBUG
    if (SingleKit)
        args.push_back("--singlekit");
#endif

#if STRACE_LOOLFORKIT
    std::string forKitPath = "/usr/bin/strace";
#elif VALGRIND_LOOLFORKIT
    std::string forKitPath = "/usr/bin/valgrind";
#else
    std::string forKitPath = parentPath + "loolforkit";
#endif

    // Always reap first, in case we haven't done so yet.
    if (ForKitProcId != -1)
    {
        if (Util::isKitInProcess())
            return true;
        int status;
        waitpid(ForKitProcId, &status, WUNTRACED | WNOHANG);
        ForKitProcId = -1;
        Admin::instance().setForKitPid(ForKitProcId);
    }

    // Below line will be executed by PrisonerPoll thread.
    ForKitProc = nullptr;
    PrisonerPoll->setForKitProcess(ForKitProc);

    // ForKit always spawns one.
    ++OutstandingForks;

    LOG_INF("Launching forkit process: " << forKitPath << ' ' << args.cat(' ', 0));

    LastForkRequestTime = std::chrono::steady_clock::now();
    int child = createForkit(forKitPath, args);
    ForKitProcId = child;

    LOG_INF("Forkit process launched: " << ForKitProcId);

    // Init the Admin manager
    Admin::instance().setForKitPid(ForKitProcId);

    const int balance = LOOLWSD::NumPreSpawnedChildren - OutstandingForks;
    if (balance > 0)
        rebalanceChildren(balance);

    return ForKitProcId != -1;
}

void LOOLWSD::sendMessageToForKit(const std::string& message)
{
    if (PrisonerPoll)
    {
        PrisonerPoll->sendMessageToForKit(message);
    }
}

#endif // !MOBILEAPP


/// Handles the socket that the prisoner kit connected to WSD on.
class PrisonerRequestDispatcher final : public WebSocketHandler
{
    std::weak_ptr<ChildProcess> _childProcess;
    int _pid; //< The Kit's PID (for logging).
    int _socketFD; //< The socket FD to the Kit (for logging).
    bool _associatedWithDoc; //< True when/if we get a DocBroker.

public:
    PrisonerRequestDispatcher()
        : WebSocketHandler(/* isClient = */ false, /* isMasking = */ true)
        , _pid(0)
        , _socketFD(0)
        , _associatedWithDoc(false)
    {
        LOG_TRC_S("PrisonerRequestDispatcher");
    }
    ~PrisonerRequestDispatcher()
    {
        LOG_TRC("~PrisonerRequestDispatcher");

        // Notify the broker that we're done.
        // Note: since this class is the default WebScoketHandler
        // for all incoming connections, for ForKit we have to
        // replace it (once we receive 'GET /loolws/forkit') with
        // ForKitProcWSHandler (see ForKitProcess) and nothing to disconnect.
        if (_childProcess.lock())
            onDisconnect();
    }

private:
    /// Keep our socket around ...
    void onConnect(const std::shared_ptr<StreamSocket>& socket) override
    {
        WebSocketHandler::onConnect(socket);
        LOG_TRC("Prisoner connected");
    }

    void onDisconnect() override
    {
        LOG_DBG("Prisoner connection disconnected");

        // Notify the broker that we're done.
        std::shared_ptr<ChildProcess> child = _childProcess.lock();
        std::shared_ptr<DocumentBroker> docBroker =
            child && child->getPid() > 0 ? child->getDocumentBroker() : nullptr;
        if (docBroker)
        {
            assert(child->getPid() == _pid && "Child PID changed unexpectedly");
            const bool unexpected = !docBroker->isUnloading() && !SigUtil::getShutdownRequestFlag();
            if (unexpected)
            {
                LOG_WRN("DocBroker [" << docBroker->getDocKey()
                                      << "] got disconnected from its Kit (" << child->getPid()
                                      << ") unexpectedly. Closing");
            }
            else
            {
                LOG_DBG("DocBroker [" << docBroker->getDocKey() << "] disconnected from its Kit ("
                                      << child->getPid() << ") as expected");
            }

            docBroker->disconnectedFromKit(unexpected);
        }
        else if (!_associatedWithDoc && !SigUtil::getShutdownRequestFlag())
        {
            LOG_WRN("Unassociated Kit (" << _pid << ") disconnected unexpectedly");

            std::unique_lock<std::mutex> lock(NewChildrenMutex);
            auto it = std::find(NewChildren.begin(), NewChildren.end(), child);
            if (it != NewChildren.end())
                NewChildren.erase(it);
            else
                LOG_WRN("Unknown Kit process closed with pid " << child ? child->getPid() : -1);
#if !MOBILEAPP
            rebalanceChildren(LOOLWSD::NumPreSpawnedChildren);
#endif
        }
    }

    /// Called after successful socket reads.
    void handleIncomingMessage(SocketDisposition &disposition) override
    {
        if (_childProcess.lock())
        {
            // FIXME: inelegant etc. - derogate to websocket code
            WebSocketHandler::handleIncomingMessage(disposition);
            return;
        }

        std::shared_ptr<StreamSocket> socket = getSocket().lock();
        if (!socket)
        {
            LOG_ERR("Invalid socket while reading incoming message");
            return;
        }

        Buffer& data = socket->getInBuffer();
        if (data.empty())
        {
            LOG_DBG("No data to process from the socket");
            return;
        }

#ifdef LOG_SOCKET_DATA
        LOG_TRC("HandleIncomingMessage: buffer has:\n"
                << Util::dumpHex(std::string(data.data(), std::min(data.size(), 256UL))));
#endif

        // Consume the incoming data by parsing and processing the body.
        http::Request request;
#if !MOBILEAPP
        const int64_t read = request.readData(data.data(), data.size());
        if (read < 0)
        {
            LOG_ERR("Error parsing prisoner socket data");
            return;
        }

        if (read == 0)
        {
            // Not enough data.
            return;
        }

        assert(read > 0 && "Must have read some data!");

        // Remove consumed data.
        data.eraseFirst(read);
#endif

        try
        {
#if !MOBILEAPP
            LOG_TRC("Child connection with URI [" << LOOLWSD::anonymizeUrl(request.getUrl())
                                                  << ']');
            Poco::URI requestURI(request.getUrl());
            if (requestURI.getPath() == FORKIT_URI)
            {
                if (socket->getPid() != LOOLWSD::ForKitProcId)
                {
                    LOG_WRN("Connection request received on "
                            << FORKIT_URI << " endpoint from unexpected ForKit process. Skipped");
                    return;
                }
                LOOLWSD::ForKitProc = std::make_shared<ForKitProcess>(LOOLWSD::ForKitProcId, socket, request);
                LOG_ASSERT_MSG(socket->getInBuffer().empty(), "Unexpected data in prisoner socket");
                socket->getInBuffer().clear();
                PrisonerPoll->setForKitProcess(LOOLWSD::ForKitProc);
                return;
            }
            if (requestURI.getPath() != NEW_CHILD_URI)
            {
                LOG_ERR("Invalid incoming child URI [" << requestURI.getPath() << ']');
                return;
            }

            const auto duration = (std::chrono::steady_clock::now() - LastForkRequestTime);
            const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            LOG_TRC("New child spawned after " << durationMs << " of requesting");

            // New Child is spawned.
            const Poco::URI::QueryParameters params = requestURI.getQueryParameters();
            const int pid = socket->getPid();
            std::string jailId;
            for (const auto& param : params)
            {
                if (param.first == "jailid")
                    jailId = param.second;

                else if (param.first == "version")
                    LOOLWSD::LOKitVersion = param.second;
            }

            if (pid <= 0)
            {
                LOG_ERR("Invalid PID in child URI [" << LOOLWSD::anonymizeUrl(request.getUrl())
                                                     << ']');
                return;
            }

            if (jailId.empty())
            {
                LOG_ERR("Invalid JailId in child URI [" << LOOLWSD::anonymizeUrl(request.getUrl())
                                                        << ']');
                return;
            }

            LOG_ASSERT_MSG(socket->getInBuffer().empty(), "Unexpected data in prisoner socket");
            socket->getInBuffer().clear();

            LOG_INF("New child [" << pid << "], jailId: " << jailId);
#else
            pid_t pid = 100;
            std::string jailId = "jail";
            socket->getInBuffer().clear();
#endif
            LOG_TRC("Calling make_shared<ChildProcess>, for NewChildren?");

            auto child = std::make_shared<ChildProcess>(pid, jailId, socket, request);

            if (!Util::isMobileApp())
                UnitWSD::get().newChild(child);

            _pid = pid;
            _socketFD = socket->getFD();
            child->setSMapsFD(socket->getIncomingFD(SMAPS));
            _childProcess = child; // weak

            addNewChild(child);
        }
        catch (const std::bad_weak_ptr&)
        {
            // Using shared_from_this() from a constructor is not good.
            assert(!"Got std::bad_weak_ptr. Are we using shared_from_this() from a constructor?");
        }
        catch (const std::exception& exc)
        {
            // Probably don't have enough data just yet.
            // TODO: timeout if we never get enough.
        }
    }

    /// Prisoner websocket fun ... (for now)
    virtual void handleMessage(const std::vector<char> &data) override
    {
        if (UnitWSD::isUnitTesting() && UnitWSD::get().filterChildMessage(data))
            return;

        auto message = std::make_shared<Message>(data.data(), data.size(), Message::Dir::Out);
        std::shared_ptr<StreamSocket> socket = getSocket().lock();
        if (socket)
        {
            assert(socket->getFD() == _socketFD && "Socket FD changed unexpectedly");
            LOG_TRC("Prisoner message [" << message->abbr() << ']');
        }
        else
            LOG_WRN("Message handler called but without valid socket.");

        std::shared_ptr<ChildProcess> child = _childProcess.lock();
        std::shared_ptr<DocumentBroker> docBroker =
            child && child->getPid() > 0 ? child->getDocumentBroker() : nullptr;
        if (docBroker)
        {
            assert(child->getPid() == _pid && "Child PID changed unexpectedly");
            _associatedWithDoc = true;
            docBroker->handleInput(message);
        }
        else if (child && child->getPid() > 0)
        {
            const std::string abbreviatedMessage = LOOLWSD::AnonymizeUserData ? "..." : message->abbr();
            LOG_WRN("Child " << child->getPid() << " has no DocBroker to handle message: ["
                             << abbreviatedMessage << ']');
        }
        else
        {
            const std::string abbreviatedMessage = LOOLWSD::AnonymizeUserData ? "..." : message->abbr();
            LOG_ERR("Cannot handle message with unassociated Kit (PID " << _pid << "): ["
                                                                        << abbreviatedMessage);
        }
    }

    int getPollEvents(std::chrono::steady_clock::time_point /* now */,
                      int64_t & /* timeoutMaxMs */) override
    {
        return POLLIN;
    }

    void performWrites(std::size_t /*capacity*/) override {}
};

class PlainSocketFactory final : public SocketFactory
{
    std::shared_ptr<Socket> create(const int physicalFd, Socket::Type type) override
    {
        int fd = physicalFd;
#if !MOBILEAPP
        if (SimulatedLatencyMs > 0)
        {
            int delayfd = Delay::create(SimulatedLatencyMs, physicalFd);
            if (delayfd == -1)
                LOG_ERR("DelaySocket creation failed, using physicalFd " << physicalFd << " instead.");
            else
                fd = delayfd;
        }
#endif
        return StreamSocket::create<StreamSocket>(
            std::string(), fd, type, false,
            std::make_shared<ClientRequestDispatcher>());
    }
};

#if ENABLE_SSL
class SslSocketFactory final : public SocketFactory
{
    std::shared_ptr<Socket> create(const int physicalFd, Socket::Type type) override
    {
        int fd = physicalFd;

#if !MOBILEAPP
        if (SimulatedLatencyMs > 0)
            fd = Delay::create(SimulatedLatencyMs, physicalFd);
#endif

        return StreamSocket::create<SslStreamSocket>(std::string(), fd, type, false,
                                                     std::make_shared<ClientRequestDispatcher>());
    }
};
#endif

class PrisonerSocketFactory final : public SocketFactory
{
    std::shared_ptr<Socket> create(const int fd, Socket::Type type) override
    {
        // No local delay.
        return StreamSocket::create<StreamSocket>(std::string(), fd, type, false,
                                                  std::make_shared<PrisonerRequestDispatcher>(),
                                                  StreamSocket::ReadType::UseRecvmsgExpectFD);
    }
};

/// The main server thread.
///
/// Waits for the connections from the lools, and creates the
/// websockethandlers accordingly.
class LOOLWSDServer
{
    LOOLWSDServer(LOOLWSDServer&& other) = delete;
    const LOOLWSDServer& operator=(LOOLWSDServer&& other) = delete;
    // allocate port & hold temporarily.
    std::shared_ptr<ServerSocket> _serverSocket;
public:
    LOOLWSDServer()
        : _acceptPoll("accept_poll")
#if !MOBILEAPP
        , _admin(Admin::instance())
#endif
    {
    }

    ~LOOLWSDServer()
    {
        stop();
    }

    void findClientPort()
    {
        _serverSocket = findServerPort();
    }

    void startPrisoners()
    {
        PrisonerPoll->startThread();
        PrisonerPoll->insertNewSocket(findPrisonerServerPort());
    }

    static void stopPrisoners()
    {
        PrisonerPoll->joinThread();
    }

    void start()
    {
        _acceptPoll.startThread();
        _acceptPoll.insertNewSocket(_serverSocket);

#if MOBILEAPP
        loolwsd_server_socket_fd = _serverSocket->getFD();
#endif

        _serverSocket.reset();
        WebServerPoll->startThread();

#if !MOBILEAPP
        _admin.start();
#endif
    }

    void stop()
    {
        _acceptPoll.joinThread();
        if (WebServerPoll)
            WebServerPoll->joinThread();
#if !MOBILEAPP
        _admin.stop();
#endif
    }

    void dumpState(std::ostream& os) const
    {
        // FIXME: add some stop-world magic before doing the dump(?)
        Socket::InhibitThreadChecks = true;
        SocketPoll::InhibitThreadChecks = true;

        std::string version, hash;
        Util::getVersionInfo(version, hash);

        os << "LOOLWSDServer: " << version << " - " << hash
#if !MOBILEAPP
           << "\n  Kit version: " << LOOLWSD::LOKitVersion
           << "\n  Ports: server " << ClientPortNumber << " prisoner " << MasterLocation
           << "\n  SSL: " << (LOOLWSD::isSSLEnabled() ? "https" : "http")
           << "\n  SSL-Termination: " << (LOOLWSD::isSSLTermination() ? "yes" : "no")
           << "\n  Security " << (LOOLWSD::NoCapsForKit ? "no" : "") << " chroot, "
           << (LOOLWSD::NoSeccomp ? "no" : "") << " api lockdown"
           << "\n  Admin: " << (LOOLWSD::AdminEnabled ? "enabled" : "disabled")
           << "\n  RouteToken: " << LOOLWSD::RouteToken
#endif
           << "\n  TerminationFlag: " << SigUtil::getTerminationFlag()
           << "\n  isShuttingDown: " << SigUtil::getShutdownRequestFlag()
           << "\n  NewChildren: " << NewChildren.size()
           << "\n  OutstandingForks: " << OutstandingForks
           << "\n  NumPreSpawnedChildren: " << LOOLWSD::NumPreSpawnedChildren
           << "\n  ChildSpawnTimeoutMs: " << ChildSpawnTimeoutMs
           << "\n  Document Brokers: " << DocBrokers.size()
#if !MOBILEAPP
           << "\n  of which ConvertTo: " << ConvertToBroker::getInstanceCount()
#endif
           << "\n  vs. MaxDocuments: " << LOOLWSD::MaxDocuments
           << "\n  NumConnections: " << LOOLWSD::NumConnections
           << "\n  vs. MaxConnections: " << LOOLWSD::MaxConnections
           << "\n  SysTemplate: " << LOOLWSD::SysTemplate
           << "\n  LoTemplate: " << LOOLWSD::LoTemplate
           << "\n  ChildRoot: " << LOOLWSD::ChildRoot
           << "\n  FileServerRoot: " << LOOLWSD::FileServerRoot
           << "\n  ServiceRoot: " << LOOLWSD::ServiceRoot
           << "\n  LOKitVersion: " << LOOLWSD::LOKitVersion
           << "\n  HostIdentifier: " << Util::getProcessIdentifier()
           << "\n  ConfigFile: " << LOOLWSD::ConfigFile
           << "\n  ConfigDir: " << LOOLWSD::ConfigDir
           << "\n  LogLevel: " << LOOLWSD::LogLevel
           << "\n  AnonymizeUserData: " << (LOOLWSD::AnonymizeUserData ? "yes" : "no")
           << "\n  CheckLoolUser: " << (LOOLWSD::CheckLoolUser ? "yes" : "no")
           << "\n  IsProxyPrefixEnabled: " << (LOOLWSD::IsProxyPrefixEnabled ? "yes" : "no")
           << "\n  OverrideWatermark: " << LOOLWSD::OverrideWatermark
           << "\n  UserInterface: " << LOOLWSD::UserInterface
            ;

        os << "\nServer poll:\n";
        _acceptPoll.dumpState(os);

        os << "Web Server poll:\n";
        WebServerPoll->dumpState(os);

        os << "Prisoner poll:\n";
        PrisonerPoll->dumpState(os);

#if !MOBILEAPP
        os << "Admin poll:\n";
        _admin.dumpState(os);

        // If we have any delaying work going on.
        Delay::dumpState(os);

        LOOLWSD::SavedClipboards->dumpState(os);
#endif

        os << "Document Broker polls "
                  << "[ " << DocBrokers.size() << " ]:\n";
        for (auto &i : DocBrokers)
            i.second->dumpState(os);

#if !MOBILEAPP
        os << "Converter count: " << ConvertToBroker::getInstanceCount() << '\n';
#endif

        Socket::InhibitThreadChecks = false;
        SocketPoll::InhibitThreadChecks = false;
    }

private:
    class AcceptPoll : public TerminatingPoll {
    public:
        AcceptPoll(const std::string &threadName) :
            TerminatingPoll(threadName) {}

        void wakeupHook() override
        {
            SigUtil::checkDumpGlobalState(dump_state);
        }
    };
    /// This thread & poll accepts incoming connections.
    AcceptPoll _acceptPoll;

#if !MOBILEAPP
    Admin& _admin;
#endif

    /// Create the internal only, local socket for forkit / kits prisoners to talk to.
    std::shared_ptr<ServerSocket> findPrisonerServerPort()
    {
        std::shared_ptr<SocketFactory> factory = std::make_shared<PrisonerSocketFactory>();
#if !MOBILEAPP
        auto socket = std::make_shared<LocalServerSocket>(*PrisonerPoll, factory);

        const std::string location = socket->bind();
        if (!location.length())
        {
            LOG_FTL("Failed to create local unix domain socket. Exiting.");
            Util::forcedExit(EX_SOFTWARE);
            return nullptr;
        }

        if (!socket->listen())
        {
            LOG_FTL("Failed to listen on local unix domain socket at " << location << ". Exiting.");
            Util::forcedExit(EX_SOFTWARE);
        }

        LOG_INF("Listening to prisoner connections on " << location);
        MasterLocation = location;
#ifndef HAVE_ABSTRACT_UNIX_SOCKETS
        if(!socket->link(LOOLWSD::SysTemplate + "/0" + MasterLocation))
        {
            LOG_FTL("Failed to hardlink local unix domain socket into a jail. Exiting.");
            Util::forcedExit(EX_SOFTWARE);
        }
#endif
#else
        constexpr int DEFAULT_MASTER_PORT_NUMBER = 9981;
        std::shared_ptr<ServerSocket> socket
            = ServerSocket::create(ServerSocket::Type::Public, DEFAULT_MASTER_PORT_NUMBER,
                                   ClientPortProto, *PrisonerPoll, factory);

        LOOLWSD::prisonerServerSocketFD = socket->getFD();
        LOG_INF("Listening to prisoner connections on #" << LOOLWSD::prisonerServerSocketFD);
#endif
        return socket;
    }

    /// Create the externally listening public socket
    std::shared_ptr<ServerSocket> findServerPort()
    {
        std::shared_ptr<SocketFactory> factory;

        if (ClientPortNumber <= 0)
        {
            // Avoid using the default port for unit-tests altogether.
            // This avoids interfering with a running test instance.
            ClientPortNumber = DEFAULT_CLIENT_PORT_NUMBER + (UnitWSD::isUnitTesting() ? 1 : 0);
        }

#if ENABLE_SSL
        if (LOOLWSD::isSSLEnabled())
            factory = std::make_shared<SslSocketFactory>();
        else
#endif
            factory = std::make_shared<PlainSocketFactory>();

        std::shared_ptr<ServerSocket> socket = ServerSocket::create(
            ClientListenAddr, ClientPortNumber, ClientPortProto, *WebServerPoll, factory);

        const int firstPortNumber = ClientPortNumber;
        while (!socket &&
#ifdef BUILDING_TESTS
               true
#else
               UnitWSD::isUnitTesting()
#endif
            )
        {
            ++ClientPortNumber;
            LOG_INF("Client port " << (ClientPortNumber - 1) << " is busy, trying "
                                   << ClientPortNumber);
            socket = ServerSocket::create(ClientListenAddr, ClientPortNumber, ClientPortProto,
                                          *WebServerPoll, factory);
        }

        if (!socket)
        {
            LOG_FTL("Failed to listen on Server port(s) (" << firstPortNumber << '-'
                                                           << ClientPortNumber << "). Exiting");
            Util::forcedExit(EX_SOFTWARE);
        }

#if !MOBILEAPP
        LOG_INF('#' << socket->getFD() << " Listening to client connections on port "
                    << ClientPortNumber);
#else
        LOG_INF("Listening to client connections on #" << socket->getFD());
#endif
        return socket;
    }
};

#if !MOBILEAPP
void LOOLWSD::processFetchUpdate()
{
    try
    {
        const std::string url(INFOBAR_URL);
        if (url.empty())
            return; // No url, nothing to do.

        Poco::URI uriFetch(url);
        uriFetch.addQueryParameter("product", APP_NAME);
        uriFetch.addQueryParameter("version", LOOLWSD_VERSION);
        LOG_TRC("Infobar update request from " << uriFetch.toString());
        std::shared_ptr<http::Session> sessionFetch = StorageBase::getHttpSession(uriFetch);
        if (!sessionFetch)
            return;

        http::Request request(uriFetch.getPathAndQuery());
        request.add("Accept", "application/json");

        const std::shared_ptr<const http::Response> httpResponse =
            sessionFetch->syncRequest(request);
        if (httpResponse->statusLine().statusCode() == http::StatusCode::OK)
        {
            LOG_DBG("Infobar update returned: " << httpResponse->getBody());

            std::lock_guard<std::mutex> lock(LOOLWSD::FetchUpdateMutex);
            LOOLWSD::LatestVersion = httpResponse->getBody();
        }
        else
            LOG_WRN("Failed to update the infobar. Got: "
                    << httpResponse->statusLine().statusCode() << ' '
                    << httpResponse->statusLine().reasonPhrase());
    }
    catch(const Poco::Exception& exc)
    {
        LOG_DBG("FetchUpdate: " << exc.displayText());
    }
    catch(std::exception& exc)
    {
        LOG_DBG("FetchUpdate: " << exc.what());
    }
    catch(...)
    {
        LOG_DBG("FetchUpdate: Unknown exception");
    }
}

#if ENABLE_DEBUG
std::string LOOLWSD::getServerURL()
{
    return getServiceURI(LOOLWSD_TEST_LOOL_UI);
}
#endif
#endif

int LOOLWSD::innerMain()
{
#if !MOBILEAPP
#  ifdef __linux__
    // down-pay all the forkit linking cost once & early.
    setenv("LD_BIND_NOW", "1", 1);
#  endif

    std::string version, hash;
    Util::getVersionInfo(version, hash);
    LOG_INF("Loolwsd version details: " << version << " - " << hash << " - id " << Util::getProcessIdentifier() << " - on " << Util::getLinuxVersion());
#endif

    initializeSSL();

#if !MOBILEAPP
    // Fetch remote settings from server if configured
    RemoteConfigPoll remoteConfigThread(config());
    remoteConfigThread.start();
#endif

#ifndef IOS
    // We can open files with non-ASCII names just fine on iOS without this, and this code is
    // heavily Linux-specific anyway.

    // Force a uniform UTF-8 locale for ourselves & our children.
    char* locale = std::setlocale(LC_ALL, "C.UTF-8");
    if (!locale)
    {
        // rhbz#1590680 - C.UTF-8 is unsupported on RH7
        LOG_WRN("Could not set locale to C.UTF-8, will try en_US.UTF-8");
        locale = std::setlocale(LC_ALL, "en_US.UTF-8");
        if (!locale)
            LOG_WRN("Could not set locale to en_US.UTF-8. Without UTF-8 support documents with non-ASCII file names cannot be opened.");
    }
    if (locale)
    {
        LOG_INF("Locale is set to " + std::string(locale));
        ::setenv("LC_ALL", locale, 1);
    }
#endif

#if !MOBILEAPP
    // We use the same option set for both parent and child loolwsd,
    // so must check options required in the parent (but not in the
    // child) separately now. Also check for options that are
    // meaningless for the parent.
    if (LoTemplate.empty())
    {
        LOG_FTL("Missing --lo-template-path option");
        throw MissingOptionException("lotemplate");
    }

    if (FileServerRoot.empty())
        FileServerRoot = Util::getApplicationPath();
    FileServerRoot = Poco::Path(FileServerRoot).absolute().toString();
    LOG_DBG("FileServerRoot: " << FileServerRoot);

    LOG_DBG("Initializing DelaySocket with " << SimulatedLatencyMs << "ms.");
    Delay delay(SimulatedLatencyMs);

    const auto fetchUpdateCheck = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::hours(std::max(getConfigValue<int>("fetch_update_check", 10), 0)));
#endif

    ClientRequestDispatcher::InitStaticFileContentCache();

    // Allocate our port - passed to prisoners.
    assert(Server && "The LOOLWSDServer instance does not exist.");
    Server->findClientPort();

    TmpFontDir = ChildRoot + JailUtil::CHILDROOT_TMP_INCOMING_PATH;

    // Start the internal prisoner server and spawn forkit,
    // which in turn forks first child.
    Server->startPrisoners();

// No need to "have at least one child" beforehand on mobile
#if !MOBILEAPP

    if (!Util::isKitInProcess())
    {
        // Make sure we have at least one child before moving forward.
        std::unique_lock<std::mutex> lock(NewChildrenMutex);
        // If we are debugging, it's not uncommon to wait for several minutes before first
        // child is born. Don't use an expiry timeout in that case.
        const bool debugging = std::getenv("SLEEPFORDEBUGGER") || std::getenv("SLEEPKITFORDEBUGGER");
        if (debugging)
        {
            LOG_DBG("Waiting for new child without timeout.");
            NewChildrenCV.wait(lock, []() { return !NewChildren.empty(); });
        }
        else
        {
            int retry = (LOOLWSD::NoCapsForKit ? 150 : 50);
            const auto timeout = std::chrono::milliseconds(ChildSpawnTimeoutMs);
            while (retry-- > 0 && !SigUtil::getShutdownRequestFlag())
            {
                LOG_INF("Waiting for a new child for a max of " << timeout);
                if (NewChildrenCV.wait_for(lock, timeout, []() { return !NewChildren.empty(); }))
                {
                    break;
                }
            }
        }

        // Check we have at least one.
        LOG_TRC("Have " << NewChildren.size() << " new children.");
        if (NewChildren.empty())
        {
            if (SigUtil::getShutdownRequestFlag())
                LOG_FTL("Shutdown requested while starting up. Exiting.");
            else
                LOG_FTL("No child process could be created. Exiting.");
            Util::forcedExit(EX_SOFTWARE);
        }

        assert(NewChildren.size() > 0);
    }

    if (LogLevel != "trace")
    {
        LOG_INF("WSD initialization complete: setting log-level to [" << LogLevel << "] as configured.");
        Log::logger().setLevel(LogLevel);
    }

    if (Log::logger().getLevel() >= Poco::Message::Priority::PRIO_INFORMATION)
        LOG_ERR("Log level is set very high to '" << LogLevel << "' this will have a "
                "significant performance impact. Do not use this in production.");

    // Start the remote font downloading polling thread.
    std::unique_ptr<RemoteFontConfigPoll> remoteFontConfigThread;
    try
    {
        // Fetch font settings from server if configured
        remoteFontConfigThread = std::make_unique<RemoteFontConfigPoll>(config());
        remoteFontConfigThread->start();
    }
    catch (const Poco::Exception&)
    {
        LOG_DBG("No remote_font_config");
    }
#endif

    // URI with /contents are public and we don't need to anonymize them.
    Util::mapAnonymized("contents", "contents");

    // Start the server.
    Server->start();

#if WASMAPP
    // It is not at all obvious that this is the ideal place to do the HULLO thing and call onopen
    // on TheFakeWebSocket. But it seems to work.
    handle_lool_message("HULLO");
    MAIN_THREAD_EM_ASM(window.TheFakeWebSocket.onopen());
#endif

    /// The main-poll does next to nothing:
    SocketPoll mainWait("main");

#if !MOBILEAPP
    std::cerr << "Ready to accept connections on port " << ClientPortNumber <<  ".\n" << std::endl;
    if (SignalParent)
    {
        kill(getppid(), SIGUSR2);
    }
#endif

#if !MOBILEAPP && ENABLE_DEBUG
    const std::string postMessageURI =
        getServiceURI("/browser/dist/framed.doc.html?file_path=" DEBUG_ABSSRCDIR
                      "/" LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_WRITER);
    std::ostringstream oss;
    oss << "\nLaunch one of these in your browser:\n\n"
        << "Edit mode:" << '\n'
        << "    Writer:      " << getLaunchURI(LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_WRITER) << '\n'
        << "    Calc:        " << getLaunchURI(LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_CALC) << '\n'
        << "    Impress:     " << getLaunchURI(LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_IMPRESS) << '\n'
        << "    Draw:        " << getLaunchURI(LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_DRAW) << '\n'
        << "\nReadonly mode:" << '\n'
        << "    Writer readonly:  " << getLaunchURI(LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_WRITER, true) << '\n'
        << "    Calc readonly:    " << getLaunchURI(LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_CALC, true) << '\n'
        << "    Impress readonly: " << getLaunchURI(LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_IMPRESS, true) << '\n'
        << "    Draw readonly:    " << getLaunchURI(LOOLWSD_TEST_DOCUMENT_RELATIVE_PATH_DRAW, true) << '\n'
        << "\npostMessage: " << postMessageURI << std::endl;

    const std::string adminURI = getServiceURI(LOOLWSD_TEST_ADMIN_CONSOLE, true);
    if (!adminURI.empty())
        oss << "\nOr for the admin, monitoring, capabilities & discovery:\n\n"
            << adminURI << '\n'
            << getServiceURI(LOOLWSD_TEST_METRICS, true) << '\n'
            << getServiceURI("/hosting/capabilities") << '\n'
            << getServiceURI("/hosting/discovery") << '\n';

    oss << std::endl;
    std::cerr << oss.str();
#endif

    const auto startStamp = std::chrono::steady_clock::now();
#if !MOBILEAPP
    auto stampFetch = startStamp - (fetchUpdateCheck - std::chrono::milliseconds(60000));

#ifdef __linux__
    if (getConfigValue<bool>("stop_on_config_change", false)) {
        std::shared_ptr<InotifySocket> inotifySocket = std::make_shared<InotifySocket>();
        mainWait.insertNewSocket(inotifySocket);
    }
#endif
#endif

    while (!SigUtil::getShutdownRequestFlag())
    {
        // This timeout affects the recovery time of prespawned children.
        std::chrono::microseconds waitMicroS = SocketPoll::DefaultPollTimeoutMicroS * 4;

        if (UnitWSD::isUnitTesting() && !SigUtil::getShutdownRequestFlag())
        {
            UnitWSD::get().invokeTest();

            // More frequent polling while testing, to reduce total test time.
            waitMicroS =
                std::min(UnitWSD::get().getTimeoutMilliSeconds(), std::chrono::milliseconds(1000));
            waitMicroS /= 4;
        }

        mainWait.poll(waitMicroS);

        // Wake the prisoner poll to spawn some children, if necessary.
        PrisonerPoll->wakeup();

        const auto timeNow = std::chrono::steady_clock::now();
        const std::chrono::milliseconds timeSinceStartMs
            = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - startStamp);
        // Unit test timeout
        if (UnitWSD::isUnitTesting() && !SigUtil::getShutdownRequestFlag())
        {
            UnitWSD::get().checkTimeout(timeSinceStartMs);
        }

#if !MOBILEAPP
        SavedClipboards->checkexpiry();

        const std::chrono::milliseconds durationFetch
            = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - stampFetch);
        if (fetchUpdateCheck > std::chrono::milliseconds::zero() && durationFetch > fetchUpdateCheck)
        {
            processFetchUpdate();
            stampFetch = timeNow;
        }
#endif

#if ENABLE_DEBUG && !MOBILEAPP
        if (careerSpanMs > std::chrono::milliseconds::zero() && timeSinceStartMs > careerSpanMs)
        {
            LOG_INF("Setting ShutdownRequestFlag: " << timeSinceStartMs << " gone, career of "
                                                    << careerSpanMs << " expired.");
            SigUtil::requestShutdown();
        }
#endif
    }

    LOOLWSD::alertAllUsersInternal("close: shuttingdown");

    // Lots of polls will stop; stop watching them first.
    SocketPoll::PollWatchdog.reset();

    // Stop the listening to new connections
    // and wait until sockets close.
    LOG_INF("Stopping server socket listening. ShutdownRequestFlag: " <<
            SigUtil::getShutdownRequestFlag() << ", TerminationFlag: " << SigUtil::getTerminationFlag());

    if (!UnitWSD::isUnitTesting())
    {
        // When running unit-tests the listening port will
        // get recycled and another test will be listening.
        // This is very problematic if a DocBroker here is
        // saving and uploading before shutting down, because
        // the test that gets the same port will receive this
        // unexpected upload and fail.

        // Otherwise, in production, we should probably respond
        // with some error that we are recycling. But for now,
        // don't change the behavior and stop listening.
        Server->stop();
    }

    // atexit handlers tend to free Admin before Documents
    LOG_INF("Exiting. Cleaning up lingering documents.");
#if !MOBILEAPP
    if (!SigUtil::getShutdownRequestFlag())
    {
        // This shouldn't happen, but it's fail safe to always cleanup properly.
        LOG_WRN("Setting ShutdownRequestFlag: Exiting WSD without ShutdownRequestFlag. Setting it "
                "now.");
        SigUtil::requestShutdown();
    }
#endif

    // Wait until documents are saved and sessions closed.
    // Don't stop the DocBroker, they will exit.
    constexpr size_t sleepMs = 200;
    constexpr size_t count = (COMMAND_TIMEOUT_MS * 6) / sleepMs;
    for (size_t i = 0; i < count; ++i)
    {
        std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
        if (DocBrokers.empty())
            break;

        LOG_DBG("Waiting for " << DocBrokers.size() << " documents to stop.");
        cleanupDocBrokers();
        docBrokersLock.unlock();

        // Give them time to save and cleanup.
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    }

    if (UnitWSD::isUnitTesting() && !SigUtil::getTerminationFlag())
    {
        LOG_INF("Setting TerminationFlag to avoid deadlocking unittest.");
        SigUtil::setTerminationFlag();
    }

    // Disable thread checking - we'll now cleanup lots of things if we can
    Socket::InhibitThreadChecks = true;
    SocketPoll::InhibitThreadChecks = true;

    // Wait for the DocumentBrokers. They must be saving/uploading now.
    // Do not stop them! Otherwise they might not save/upload the document.
    // We block until they finish, or the service stopping times out.
    {
        std::unique_lock<std::mutex> docBrokersLock(DocBrokersMutex);
        for (auto& docBrokerIt : DocBrokers)
        {
            std::shared_ptr<DocumentBroker> docBroker = docBrokerIt.second;
            if (docBroker && docBroker->isAlive())
            {
                LOG_DBG("Joining docBroker [" << docBrokerIt.first << "].");
                docBroker->joinThread();
            }
        }

        // Now should be safe to destroy what's left.
        cleanupDocBrokers();
        DocBrokers.clear();
    }

    if (TraceEventFile != NULL)
    {
        // If we have written any objects to it, it ends with a comma and newline. Back over those.
        if (ftell(TraceEventFile) > 2)
            (void)fseek(TraceEventFile, -2, SEEK_CUR);
        // Close the JSON array.
        fprintf(TraceEventFile, "\n]\n");
        fclose(TraceEventFile);
        TraceEventFile = NULL;
    }

#if !MOBILEAPP
    if (!Util::isKitInProcess())
    {
        // Terminate child processes
        LOG_INF("Requesting forkit process " << ForKitProcId << " to terminate.");
#if CODE_COVERAGE || VALGRIND_LOOLFORKIT
        constexpr auto signal = SIGTERM;
#else
        constexpr auto signal = SIGKILL;
#endif
        SigUtil::killChild(ForKitProcId, signal);
    }
#endif

    Server->stopPrisoners();

    if (UnitWSD::isUnitTesting())
    {
        Server->stop();
        Server.reset();
    }

    PrisonerPoll.reset();

    WebServerPoll.reset();

    // Terminate child processes
    LOG_INF("Requesting child processes to terminate.");
    for (auto& child : NewChildren)
    {
        child->terminate();
    }

    NewChildren.clear();

#if !MOBILEAPP
    if (!Util::isKitInProcess())
    {
        // Wait for forkit process finish.
        LOG_INF("Waiting for forkit process to exit");
        int status = 0;
        waitpid(ForKitProcId, &status, WUNTRACED);
        ForKitProcId = -1;
        ForKitProc.reset();
    }

    JailUtil::cleanupJails(CleanupChildRoot);
#endif // !MOBILEAPP

    const int returnValue = UnitBase::uninit();

    LOG_INF("Process [loolwsd] finished with exit status: " << returnValue);

    // At least on centos7, Poco deadlocks while
    // cleaning up its SSL context singleton.
    Util::forcedExit(returnValue);

    return returnValue;
}

std::shared_ptr<TerminatingPoll> LOOLWSD:: getWebServerPoll ()
{
    return WebServerPoll;
}

void LOOLWSD::cleanup()
{
    try
    {
        Server.reset();

        PrisonerPoll.reset();

        WebServerPoll.reset();

#if !MOBILEAPP
        SavedClipboards.reset();

        FileRequestHandler.reset();
        JWTAuth::cleanup();

#if ENABLE_SSL
        // Finally, we no longer need SSL.
        if (LOOLWSD::isSSLEnabled())
        {
            Poco::Net::uninitializeSSL();
            Poco::Crypto::uninitializeCrypto();
            ssl::Manager::uninitializeClientContext();
            ssl::Manager::uninitializeServerContext();
        }
#endif
#endif

        TraceDumper.reset();

        Socket::InhibitThreadChecks = true;
        SocketPoll::InhibitThreadChecks = true;

        // Delete these while the static Admin instance is still alive.
        std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);
        DocBrokers.clear();
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Failed to uninitialize: " << ex.what());
    }
}

int LOOLWSD::main(const std::vector<std::string>& /*args*/)
{
#if MOBILEAPP && !defined IOS
    SigUtil::resetTerminationFlags();
#endif

    int returnValue;

    try {
        returnValue = innerMain();
    }
    catch (const std::exception& e)
    {
        LOG_FTL("Exception: " << e.what());
        cleanup();
        throw;
    } catch (...) {
        cleanup();
        throw;
    }

    cleanup();

    returnValue = UnitBase::uninit();

    LOG_INF("Process [loolwsd] finished with exit status: " << returnValue);

#if CODE_COVERAGE
    __gcov_dump();
#endif

    return returnValue;
}

int LOOLWSD::getClientPortNumber()
{
    return ClientPortNumber;
}

#if !MOBILEAPP

std::vector<std::shared_ptr<DocumentBroker>> LOOLWSD::getBrokersTestOnly()
{
    std::lock_guard<std::mutex> docBrokersLock(DocBrokersMutex);
    std::vector<std::shared_ptr<DocumentBroker>> result;

    result.reserve(DocBrokers.size());
    for (auto& brokerIt : DocBrokers)
        result.push_back(brokerIt.second);
    return result;
}

std::set<pid_t> LOOLWSD::getKitPids()
{
    std::set<pid_t> pids = getSpareKitPids();
    pids.merge(getDocKitPids());
    return pids;
}

std::set<pid_t> LOOLWSD::getSpareKitPids()
{
    std::set<pid_t> pids;
    pid_t pid;
    {
        std::unique_lock<std::mutex> lock(NewChildrenMutex);
        for (const auto &child : NewChildren)
        {
            pid = child->getPid();
            if (pid > 0)
                pids.emplace(pid);
        }
    }
    return pids;
}

std::set<pid_t> LOOLWSD::getDocKitPids()
{
    std::set<pid_t> pids;
    pid_t pid;
    {
        std::unique_lock<std::mutex> lock(DocBrokersMutex);
        for (const auto &it : DocBrokers)
        {
            pid = it.second->getPid();
            if (pid > 0)
                pids.emplace(pid);
        }
    }
    return pids;
}

#if !defined(BUILDING_TESTS)
namespace Util
{

void alertAllUsers(const std::string& cmd, const std::string& kind)
{
    alertAllUsers("error: cmd=" + cmd + " kind=" + kind);
}

void alertAllUsers(const std::string& msg)
{
    LOOLWSD::alertAllUsersInternal(msg);
}

}
#endif

#endif

void dump_state()
{
    std::ostringstream oss;

    if (Server)
        Server->dumpState(oss);

    const std::string msg = oss.str();
    fprintf(stderr, "%s\n", msg.c_str());
    LOG_TRC(msg);
}

void forwardSigUsr2()
{
    LOG_TRC("forwardSigUsr2");

    if (Util::isKitInProcess())
        return;

    Util::assertIsLocked(DocBrokersMutex);
    std::lock_guard<std::mutex> newChildLock(NewChildrenMutex);

#if !MOBILEAPP
    if (LOOLWSD::ForKitProcId > 0)
    {
        LOG_INF("Sending SIGUSR2 to forkit " << LOOLWSD::ForKitProcId);
        ::kill(LOOLWSD::ForKitProcId, SIGUSR2);
    }
#endif

    for (const auto& child : NewChildren)
    {
        if (child && child->getPid() > 0)
        {
            LOG_INF("Sending SIGUSR2 to child " << child->getPid());
            ::kill(child->getPid(), SIGUSR2);
        }
    }

    for (const auto& pair : DocBrokers)
    {
        std::shared_ptr<DocumentBroker> docBroker = pair.second;
        if (docBroker)
        {
            LOG_INF("Sending SIGUSR2 to docBroker " << docBroker->getPid());
            ::kill(docBroker->getPid(), SIGUSR2);
        }
    }
}

// Avoid this in the Util::isFuzzing() case because libfuzzer defines its own main().
#if !MOBILEAPP && !LIBFUZZER

int main(int argc, char** argv)
{
    SigUtil::setUserSignals();
    SigUtil::setFatalSignals("wsd " LOOLWSD_VERSION " " LOOLWSD_VERSION_HASH);
    setKitInProcess();

    try
    {
        LOOLWSD app;
        return app.run(argc, argv);
    }
    catch (Poco::Exception& exc)
    {
        std::cerr << exc.displayText() << std::endl;
        return EX_SOFTWARE;
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
