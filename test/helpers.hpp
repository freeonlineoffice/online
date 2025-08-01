/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <Common.hpp>
#include <JsonUtil.hpp>
#include <Socket.hpp>
#include <WebSocketSession.hpp>
#include <common/ConfigUtil.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <test/lokassert.hpp>
#include <test/testlog.hpp>
#include <tools/LOOLWebSocket.hpp>
#include <wsd/TileDesc.hpp>

#include <Poco/BinaryReader.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/Path.h>
#include <Poco/URI.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <thread>

#ifndef TDOC
#error TDOC must be defined (see Makefile.am)
#endif

// Sometimes we need to retry some commands as they can (due to timing or load) soft-fail.
constexpr int COMMAND_RETRY_COUNT = 5;

/// Common helper testing functions.
/// Avoid the temptation to reuse from LOOL code!
/// These are supposed to be testing the latter.
namespace helpers
{

inline
std::vector<char> genRandomData(const size_t size)
{
    std::vector<char> v(size);
    v.resize(size);
    char* data = v.data();
    for (size_t i = 0; i < size; ++i)
    {
        data[i] = static_cast<char>(Util::rng::getNext());
    }

    return v;
}

inline
std::string genRandomString(const size_t size)
{
    std::string text;
    text.reserve(size);
    for (size_t i = 0; i < size; ++i)
    {
        // Sensible characters only, avoiding 0x7f DEL
        text += static_cast<char>('!' + Util::rng::getNext() % 94);
    }

    return text;
}

inline
std::vector<char> readDataFromFile(const std::string& filename)
{
    std::ifstream ifs(Poco::Path(TDOC, filename).toString(), std::ios::binary);

    // Apparently std::ios::binary is not good
    // enough to stop eating new-line chars!
    ifs.unsetf(std::ios::skipws);

    std::istream_iterator<char> start(ifs);
    std::istream_iterator<char> end;
    return std::vector<char>(start, end);
}

inline
std::vector<char> readDataFromFile(std::unique_ptr<std::fstream>& file)
{
    file->seekg(0, std::ios_base::end);
    const std::streamsize size = file->tellg();

    std::vector<char> v;
    v.resize(size);

    file->seekg(0, std::ios_base::beg);
    file->read(v.data(), size);

    return v;
}

namespace
{
/// Class to delete files when the process ends.
class FileDeleter
{
    std::vector<std::string> _filesToDelete;
    std::mutex _lock;
public:
    FileDeleter() {}
    ~FileDeleter()
    {
        std::unique_lock<std::mutex> guard(_lock);
        for (const std::string& file: _filesToDelete)
            std::filesystem::remove(file);
    }

    void registerForDeletion(const std::string& file)
    {
        std::unique_lock<std::mutex> guard(_lock);
        _filesToDelete.push_back(file);
    }
};
}

/// Make a temp copy of a file, and prepend it with a prefix.
/// Used by tests to avoid tainting the originals.
inline std::string getTempFileCopyPath(const std::string& srcDir, const std::string& srcFilename, const std::string& dstFilenamePrefix)
{
    const std::string srcPath = srcDir + '/' + srcFilename;
    std::string dstPath;

    bool retry;
    do {
        std::string dstFilename = dstFilenamePrefix + Util::encodeId(Util::rng::getNext()) + '_' + srcFilename;

        retry = false;
        dstPath = std::filesystem::temp_directory_path() / dstFilename;
        try {
            std::filesystem::copy(srcPath, dstPath);
        }
        catch (const std::exception& ex)
        {
            LOG_SYS("ERROR: unexpected conflict creating file: " << ex.what());
            retry = true;;
        }
    } while (retry);

    static FileDeleter fileDeleter;
    fileDeleter.registerForDeletion(dstPath);

    return dstPath;
}

/// Make a temp copy of a file.
/// Used by tests to avoid tainting the originals.
/// srcDir shouldn't end with '/' and srcFilename shouldn't contain '/'.
/// Returns the created file path.
inline std::string getTempFileCopyPath(const std::string& srcDir, const std::string& srcFilename)
{
    return getTempFileCopyPath(srcDir, srcFilename, std::string());
}

inline void getDocumentPathAndURL(const std::string& docFilename, std::string& documentPath,
                                  std::string& documentURL, std::string prefix)
{
    const std::string testname = prefix;


    std::replace(prefix.begin(), prefix.end(), ' ', '_');
    documentPath = getTempFileCopyPath(TDOC, docFilename, prefix);
    std::string encodedUri;
    Poco::URI::encode("file://" + Poco::Path(documentPath).makeAbsolute().toString(), ":/?",
                      encodedUri);
    documentURL = "lool/" + encodedUri + "/ws";
    TST_LOG("Test file: " << documentPath);
}

inline
void sendTextFrame(LOOLWebSocket& socket, const std::string& string, const std::string& testname)
{
    TST_LOG("Sending " << string.size()
                       << " bytes: " << LOOLProtocol::getAbbreviatedMessage(string));
    socket.sendFrame(string.data(), string.size());
}

inline void sendTextFrame(const std::shared_ptr<LOOLWebSocket>& socket, const std::string& string,
                          const std::string& testname)
{
    sendTextFrame(*socket, string, testname);
}

inline void sendTextFrame(const std::shared_ptr<http::WebSocketSession>& ws,
                          const std::string& string, const std::string& testname = std::string())
{
    TST_LOG("Sending " << string.size()
                       << " bytes: " << LOOLProtocol::getAbbreviatedMessage(string));
    ws->sendMessage(string);
}

inline std::unique_ptr<Poco::Net::HTTPClientSession> createSession(const Poco::URI& uri)
{
#if ENABLE_SSL
    if (uri.getScheme() == "https" || uri.getScheme() == "wss")
        return std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort());
#endif

    return std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
}

/// Uses Poco to make an HTTP GET from the given URI.
inline std::pair<std::shared_ptr<Poco::Net::HTTPResponse>, std::string>
pocoGet(const Poco::URI& uri)
{
    LOG_INF("pocoGet: " << uri.toString());
    std::unique_ptr<Poco::Net::HTTPClientSession> session(helpers::createSession(uri));
    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri.getPathAndQuery(),
                                   Poco::Net::HTTPMessage::HTTP_1_1);
    session->sendRequest(request);
    auto response = std::make_shared<Poco::Net::HTTPResponse>();
    std::istream& rs = session->receiveResponse(*response);

    LOG_DBG("pocoGet response for [" << uri.toString() << "]: " << response->getStatus() << ' '
                                     << response->getReason()
                                     << ", hasContentLength: " << response->hasContentLength()
                                     << ", ContentLength: " << response->getContentLength64());

    for (const auto& pair : *response)
    {
        LOG_TRC(pair.first << '=' << pair.second);
    }

    std::string responseString;
    try
    {
        std::ostringstream outputStringStream;
        Poco::StreamCopier::copyStream(rs, outputStringStream);
        responseString = outputStringStream.str();
        LOG_DBG("pocoGet [" << uri.toString() << "]: " << responseString);
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Exception while reading Poco stream: " << ex.what());
    }

    return std::make_pair(response, responseString);
}

/// Uses Poco to make an HTTP GET from the given URI.
/// And optionally retries up to @retry times with
/// @delayMs between attempts.
inline std::pair<std::shared_ptr<Poco::Net::HTTPResponse>, std::string>
pocoGetRetry(const Poco::URI& uri, int retry = 3,
             std::chrono::milliseconds delayMs = std::chrono::seconds(1))
{
    for (int attempt = 1; attempt <= retry; ++attempt)
    {
        try
        {
            LOG_INF("pocoGet #" << attempt << ": " << uri.toString());
            auto res = pocoGet(uri);
            if (!res.first)
            {
                throw std::runtime_error("Server unavilable");
            }

            return res;
        }
        catch (const std::exception& ex)
        {
            LOG_ERR("pocoGet #" << attempt << " failed for [" << uri.toString()
                                << "]: " << ex.what());
            if (attempt == retry)
                throw;

            std::this_thread::sleep_for(delayMs);
        }
    }

    auto response = std::make_shared<Poco::Net::HTTPResponse>();
    std::string responseString;
    return std::make_pair(response, responseString);
}

/// Uses Poco to make an HTTP GET from the given URI components.
inline std::pair<std::shared_ptr<Poco::Net::HTTPResponse>, std::string>
pocoGet(bool secure, const std::string& host, const int port, const std::string& url)
{
    const char* scheme = (secure ? "https://" : "http://");
    Poco::URI uri(scheme + host + ':' + std::to_string(port) + url);
    return pocoGet(uri);
}

// Sets read / write timeout for the given file descriptor.
inline void setSocketTimeOut(int socketFD, int timeMS)
{
    struct timeval tv;
    tv.tv_sec = (float)timeMS / (float)1000;
    tv.tv_usec = timeMS;
    setsockopt(socketFD, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}

// Sets socket's blocking mode. true for blocking, false for non blocking.
inline void setSocketBlockingMode(int socketFD, bool blocking)
{
    ioctl(socketFD, FIONBIO, blocking == true ? 0: 1);
}

// Creates a socket and connects it to a local server. Returns the file descriptor.
inline int connectToLocalServer(int portNumber, int socketTimeOutMS, bool blocking)
{
    int socketFD = 0;
    struct sockaddr_in serv_addr;

    if ((socketFD = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)) < 0)
    {
        LOG_ERR("helpers::connectToLocalServer: Server client could not be created.");
        return -1;
    }
    else
    {
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(portNumber);
        if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
        {
            LOG_ERR("helpers::connectToLocalServer: Invalid address.");
            close(socketFD);
            return -1;
        }
        else
        {
            if (connect(socketFD, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
            {
                LOG_ERR("helpers::connectToLocalServer: Connection failed.");
                close(socketFD);
                return -1;
            }
            else
            {
                setSocketTimeOut(socketFD, socketTimeOutMS);
                setSocketBlockingMode(socketFD, blocking);
                return socketFD;
            }
        }
    }
}

/// Returns true iff built with SSL and it is successfully initialized.
inline bool haveSsl()
{
#if ENABLE_SSL
    return ssl::Manager::isClientContextInitialized();
#else
    return false;
#endif
}

/// Return a fully-qualified URI, with schema, to the test loopback server.
inline std::string const& getTestServerURI(const std::string& proto = "http")
{
    static std::string serverURI(
        proto + ((haveSsl() && ConfigUtil::isSslEnabled()) ? "s://127.0.0.1:" : "://127.0.0.1:") +
        std::to_string(ClientPortNumber));

    return serverURI;
}


inline std::vector<char>
getResponseMessage(LOOLWebSocket& ws, const std::string& prefix, const std::string& testname,
                   const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10))
{
    try
    {
        int flags = 0;
        std::vector<char> response;

        auto endTime = std::chrono::steady_clock::now() + timeoutMs;

        ws.setReceiveTimeout(0);
        do
        {
            auto now = std::chrono::steady_clock::now();
            if (now > endTime) // timedout
            {
                TST_LOG("Timeout waiting for [" << prefix << "] after " << timeoutMs);
                break;
            }
            long waitTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - now).count();
            if (ws.poll(Poco::Timespan(waitTimeUs), Poco::Net::Socket::SELECT_READ))
            {
                response.resize(READ_BUFFER_SIZE * 8);
                const int bytes = ws.receiveFrame(response.data(), response.size(), flags, testname);
                response.resize(std::max(bytes, 0));
                const auto message = LOOLProtocol::getFirstLine(response);
                if (bytes > 0 && (flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) != Poco::Net::WebSocket::FRAME_OP_CLOSE)
                {
                    if (LOOLProtocol::matchPrefix(prefix, message))
                    {
                        TST_LOG('[' << prefix << "] Matched "
                                    << LOOLWebSocket::getAbbreviatedFrameDump(response.data(),
                                                                              bytes, flags));
                        return response;
                    }
                }
                else
                {
                    response.resize(0);
                }

                if (bytes <= 0)
                {
                    // Try again, timeout will be handled above.
                    continue;
                }

                if ((flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) != Poco::Net::WebSocket::FRAME_OP_CLOSE)
                {
                    // Don't ignore errors.
                    if (LOOLProtocol::matchPrefix("error:", message))
                    {
                        throw std::runtime_error(message);
                    }

                    TST_LOG('[' << prefix << "] Ignored "
                                << LOOLWebSocket::getAbbreviatedFrameDump(response.data(), bytes,
                                                                          flags));
                }
            }
        }
        while ((flags & Poco::Net::WebSocket::FRAME_OP_BITMASK) != Poco::Net::WebSocket::FRAME_OP_CLOSE);
    }
    catch (const Poco::Net::WebSocketException& exc)
    {
        TST_LOG('[' << prefix << "] ERROR in helpers::getResponseMessage: " << exc.message());
    }

    return std::vector<char>();
}

inline std::vector<char> getResponseMessage(const std::shared_ptr<http::WebSocketSession>& ws,
                                     const std::string& prefix, const std::string& testname,
                                     const std::chrono::milliseconds timeoutMs
                                     = std::chrono::seconds(10))
{
    return ws->waitForMessage(prefix, timeoutMs, testname);
}

inline std::shared_ptr<TileDesc> getResponseDesc(const std::shared_ptr<http::WebSocketSession>& ws,
                                                 const std::string& prefix, const std::string& testname,
                                                 const std::chrono::milliseconds timeoutMs
                                                 = std::chrono::seconds(10))
{
    std::vector<char> tile = getResponseMessage(ws, prefix, testname, timeoutMs);

    if (tile.empty())
        return std::shared_ptr<TileDesc>();

    return std::make_shared<TileDesc>(
        TileDesc::parse(StringVector::tokenize(tile.data(), tile.size())));
}

inline std::string getResponseString(const std::shared_ptr<http::WebSocketSession>& ws,
                                     const std::string& prefix, const std::string& testname,
                                     const std::chrono::milliseconds timeoutMs
                                     = std::chrono::seconds(10))
{
    const std::vector<char> response = ws->waitForMessage(prefix, timeoutMs, testname);

    return std::string(response.data(), response.size());
}

inline std::string
getResponseStringAny(const std::shared_ptr<http::WebSocketSession>& ws,
                     const std::vector<std::string>& prefixes, const std::string& testname,
                     const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10))
{
    const std::vector<char> response = ws->waitForMessageAny(prefixes, timeoutMs, testname);

    return std::string(response.data(), response.size());
}

inline std::vector<std::string> getAllResponsesTimed(const std::shared_ptr<http::WebSocketSession>& ws,
                                                     const std::string& prefix, const std::string& testname,
                                                     const std::chrono::milliseconds timeoutMs
                                                     = std::chrono::seconds(5))
{
    std::vector<std::string> responses;

    auto endTime = std::chrono::steady_clock::now() + timeoutMs;
    std::chrono::steady_clock::time_point now;
    while ((now = std::chrono::steady_clock::now()) < endTime)
    {
        auto response = helpers::getResponseString(ws, prefix, testname,
                                                   std::chrono::duration_cast<std::chrono::milliseconds>(endTime - now));
        if (response.length() > 0)
            responses.push_back(response);
    }

    return responses;
}


inline std::string assertResponseString(const std::shared_ptr<http::WebSocketSession>& ws,
                                        const std::string& prefix, const std::string& testname,
                                        const std::chrono::milliseconds timeoutMs
                                        = std::chrono::seconds(10))
{
    auto res = getResponseString(ws, prefix, testname, timeoutMs);
    LOK_ASSERT_EQUAL(prefix, res.substr(0, prefix.length()));
    return res;
}

inline int countMessages(const std::shared_ptr<http::WebSocketSession>& ws,
                         const std::string& prefix, const std::string& testname,
                         const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10))
{
    int count = 0;
    while (!getResponseMessage(ws, prefix, testname, timeoutMs).empty())
        ++count;

    return count;
}

template <typename T>
std::string getResponseString(T& ws, const std::string& prefix, const std::string& testname,
                              const std::chrono::milliseconds timeoutMs = std::chrono::seconds(10))
{
    const auto response = getResponseMessage(ws, prefix, testname, timeoutMs);
    return std::string(response.data(), response.size());
}

/// Assert that we don't get a response with the given prefix.
template <typename T>
std::string assertNotInResponse(T& ws, const std::string& prefix, const std::string& testname)
{
    const auto res = getResponseString(ws, prefix, testname, std::chrono::milliseconds(1000));
    LOK_ASSERT_MESSAGE(testname + "Did not expect getting message [" + res + "].", res.empty());
    return res;
}

inline bool getProgressWithIdValue(const std::string &msg, const std::string &idValue)
{
    const std::string prefix = "progress:";
    if (!LOOLProtocol::matchPrefix(prefix, msg))
        return false;

    Poco::JSON::Object::Ptr obj;
    if (!JsonUtil::parseJSON(msg, obj))
        return false;

    std::string jsonId = JsonUtil::getJSONValue<std::string>(obj, "id");
    return jsonId == idValue;
}

inline bool isDocumentLoaded(
    const std::shared_ptr<http::WebSocketSession>& ws, const std::string& testname,
    bool isView = true,
    const std::chrono::milliseconds timeout = std::chrono::seconds(COMMAND_TIMEOUT_SECS * 4))
{
    bool success = false;

    if (isView) // 2nd connection - someone else did the load
    {
        const std::string message = getResponseString(ws, "status:", testname, timeout);
        success = LOOLProtocol::matchPrefix("status:", message);
    }
    else
    {
        const std::string prefix = "progress:";
        while (true)
        {
            const std::string message = getResponseString(ws, prefix, testname, timeout);
            if (!LOOLProtocol::matchPrefix(prefix, message))
                break; // timeout
            if (getProgressWithIdValue(message, "finish"))
            {
                success = true;
                break;
            }
        }
    }

    return success;
}

// Connecting to a Kit process is managed by document broker, that it does several
// jobs to establish the bridge connection between the Client and Kit process,
// The result, it is mostly time outs to get messages in the unit test and it could fail.
// connectLOKit ensures the websocket is connected to a kit process.
inline std::shared_ptr<http::WebSocketSession>
connectLOKit(const std::shared_ptr<SocketPoll>& socketPoll, const Poco::URI& uri,
             const std::string& url, const std::string& testname)
{
    TST_LOG("Connecting to " << uri.toString() << " with URL: " << url);
    constexpr int max_retries = 11;
    int retries = max_retries - 1;
    do
    {
        try
        {
            // Load a document and get its status.
            auto ws = http::WebSocketSession::create(uri.toString());

            TST_LOG("Connection to " << uri.toString() << " is "
                                     << (ws->secure() ? "secure" : "plain")
                                     << ", requesting: " << url);

            http::Request req(url);
            ws->asyncRequest(req, socketPoll);

            TST_LOG("Requested " << url << " from " << uri.toString()
                                 << ", waiting for progress: id:find response");
            std::string msg;
            if (!(msg = getResponseString(ws, "progress:", testname)).empty() &&
                LOOLProtocol::matchPrefix("progress:", msg) &&
                getProgressWithIdValue(msg, "find"))
            {
                return ws;
            }

            if (SigUtil::getShutdownRequestFlag())
            {
                TST_LOG("Shutdown requested, giving up connectLOKit");
                break;
            }

            if (UnitBase::get().isFinished())
            {
                TST_LOG("The test has finished, giving up connectLOKit");
                break;
            }

            TST_LOG("ERROR: Reconnecting (retry #" << (max_retries - retries) << ") to "
                                                   << uri.toString());
        }
        catch (const std::exception& ex)
        {
            TST_LOG("ERROR: Failed to connect to " << uri.toString() << ": " << ex.what());
        }

        std::this_thread::sleep_for(std::chrono::microseconds(POLL_TIMEOUT_MICRO_S));
    } while (retries-- && !SigUtil::getShutdownRequestFlag());

    TST_LOG("ERROR: Giving up connecting to " << uri.toString());
    throw std::runtime_error("Cannot connect to [" + uri.toString() + "].");
}

/// Load a document and get the WS Session.
/// By default, allow longer time for loading.
inline std::shared_ptr<http::WebSocketSession> loadDocAndGetSession(
    const std::shared_ptr<SocketPoll>& socketPoll, const Poco::URI& uri,
    const std::string& documentURL, const std::string& testname, bool isView = true,
    bool isAssert = true, const std::string& loadParams = std::string(),
    const std::chrono::milliseconds timeout = std::chrono::seconds(COMMAND_TIMEOUT_SECS * 4))
{
    try
    {
        // Load a document and get its status.
        auto ws = http::WebSocketSession::create(uri.toString());
        http::Request req(documentURL);
        ws->asyncRequest(req, socketPoll);

        sendTextFrame(ws, "load url=" + documentURL + loadParams, testname);
        const bool isLoaded = isDocumentLoaded(ws, testname, isView, timeout);
        if (!isLoaded && !isAssert)
        {
            return nullptr;
        }

        LOK_ASSERT_MESSAGE("Failed to load the document " + documentURL, isLoaded);

        TST_LOG("Loaded document [" << documentURL << "].");
        return ws;
    }
    catch (const Poco::Exception& ex)
    {
        LOK_ASSERT_FAIL(ex.displayText());
    }
    catch (const std::exception& ex)
    {
        LOK_ASSERT_FAIL(ex.what());
    }

    // Really can't reach here, but the compiler doesn't know better.
    return nullptr;
}

inline std::shared_ptr<http::WebSocketSession>
loadDocAndGetSession(const std::shared_ptr<SocketPoll>& socketPoll, const std::string& docFilename,
                     const Poco::URI& uri, const std::string& testname, bool isView = true,
                     bool isAssert = true)
{
    try
    {
        std::string documentPath, documentURL;
        getDocumentPathAndURL(docFilename, documentPath, documentURL, testname);
        return loadDocAndGetSession(socketPoll, uri, documentURL, testname, isView, isAssert);
    }
    catch (const std::exception& ex)
    {
        LOK_ASSERT_FAIL(ex.what());
    }

    // Really couldn't reach here, but the compiler doesn't know any better.
    return nullptr;
}

inline void
SocketProcessor(const std::string& testname, const std::shared_ptr<http::WebSocketSession>& ws,
                const std::function<bool(const std::string& msg)>& handler,
                const std::chrono::milliseconds timeout = std::chrono::milliseconds(10000))
{
    ws->poll(
        [&](const std::vector<char>& message) {
            return !handler(std::string(message.data(), message.size()));
        },
        timeout, testname);
}

inline
void parseDocSize(const std::string& message, const std::string& type,
                  int& part, int& parts, int& width, int& height, int& viewid,
                  const std::string& testname)
{
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var statusJsonVar = parser.parse(message);
    const Poco::SharedPtr<Poco::JSON::Object>& statusJsonObject = statusJsonVar.extract<Poco::JSON::Object::Ptr>();

    const std::string text = statusJsonObject->get("type").toString();
    parts = std::stoi(statusJsonObject->get("partscount").toString());
    part = std::stoi(statusJsonObject->get("selectedpart").toString());
    width = std::stoi(statusJsonObject->get("width").toString());
    height = std::stoi(statusJsonObject->get("height").toString());
    viewid = std::stoi(statusJsonObject->get("viewid").toString());
    LOK_ASSERT_EQUAL(type, text);
    LOK_ASSERT(parts > 0);
    LOK_ASSERT(part >= 0);
    LOK_ASSERT(width > 0);
    LOK_ASSERT(height > 0);
    LOK_ASSERT(viewid >= 0);
}

inline std::vector<char> getTileMessage(const std::shared_ptr<http::WebSocketSession>& ws,
                                        const std::string& testname)
{
    return getResponseMessage(ws, "tile", testname);
}

enum SpecialKey : std::uint16_t { skNone=0, skShift=0x1000, skCtrl=0x2000, skAlt=0x4000 };

inline int getCharChar(char ch, SpecialKey specialKeys)
{
    // Some primitive code just suitable to basic needs of specific test.
    // TODO: improve as appropriate.
    if (specialKeys & (skCtrl | skAlt))
        return 0;

    switch (ch)
    {
        case '\x0a': // Enter
            return 13;
        default:
            return ch;
    }
}

inline int getCharKey(char ch, SpecialKey specialKeys)
{
    // Some primitive code just suitable to basic needs of specific test.
    // TODO: improve as appropriate.
    int result;
    switch (ch)
    {
        case '\x0a': // Enter
            result = 1280;
            break;
        default:
            result = ch;
    }
    return result | specialKeys;
}

inline void sendKeyEvent(std::shared_ptr<http::WebSocketSession>& socket, const char* type, int chr,
                         int key, const std::string& testname)
{
    std::ostringstream ssIn;
    ssIn << "key type=" << type << " char=" << chr << " key=" << key;
    sendTextFrame(socket, ssIn.str(), testname);
}

inline void sendKeyPress(std::shared_ptr<http::WebSocketSession>& socket, int chr, int key,
                         const std::string& testname)
{
    sendKeyEvent(socket, "input", chr, key, testname);
    sendKeyEvent(socket, "up", chr, key, testname);
}

inline void sendChar(std::shared_ptr<http::WebSocketSession>& socket, char ch,
                     SpecialKey specialKeys, const std::string& testname)
{
    sendKeyPress(socket, getCharChar(ch, specialKeys), getCharKey(ch, specialKeys), testname);
}

inline void sendText(std::shared_ptr<http::WebSocketSession>& socket, const std::string& text,
                     const std::string& testname)
{
    for (char ch : text)
    {
        sendChar(socket, ch, skNone, testname);
    }
}

inline void saveTileAs(const std::vector<char> &tileResponse,
                       const std::string &filename,
                       const std::string &testname)
{
    const std::string firstLine = LOOLProtocol::getFirstLine(tileResponse);
    std::vector<char> res(tileResponse.begin() + firstLine.size() + 1, tileResponse.end());
    std::stringstream streamRes;
    std::copy(res.begin(), res.end(), std::ostream_iterator<char>(streamRes));
    std::fstream outStream(filename, std::ios::out);
    outStream.write(res.data(), res.size());
    outStream.close();
    TST_LOG("Saved [" << firstLine << "] to [" << filename << ']');
}

template <typename T>
std::vector<char> getTileAndSave(T& socket, const std::string& req, const std::string& filename,
                                 const std::string& testname)
{
    TST_LOG("Requesting: " << req);
    sendTextFrame(socket, req, testname);

    const std::vector<char> tile = getResponseMessage(socket, "tile:", testname);
    TST_LOG(" Tile PNG size: " << tile.size());

    const std::string firstLine = LOOLProtocol::getFirstLine(tile);
    std::vector<char> res(tile.begin() + firstLine.size() + 1, tile.end());
    std::stringstream streamRes;
    std::copy(res.begin(), res.end(), std::ostream_iterator<char>(streamRes));

    if (!filename.empty())
        saveTileAs(tile, filename, testname);

    return res;
}

template <typename T>
inline void getServerVersion(T& socket, int& major, int& minor, const std::string& testname)
{
    const std::string clientVersion = "loolclient 0.1";
    sendTextFrame(socket, clientVersion, testname);
    std::vector<char> loVersion = getResponseMessage(socket, "lokitversion", testname);
    std::string line = LOOLProtocol::getFirstLine(loVersion.data(), loVersion.size());
    line = line.substr(strlen("lokitversion "));
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var loVersionVar = parser.parse(line);
    const Poco::SharedPtr<Poco::JSON::Object>& loVersionObject = loVersionVar.extract<Poco::JSON::Object::Ptr>();
    std::string loProductVersion = loVersionObject->get("ProductVersion").toString();
    std::istringstream stream(loProductVersion);
    stream >> major;
    if (stream.get() == '.')
    {
        stream >> minor;
    }
    else
    {
        minor = 0;
    }

    TST_LOG("Client [" << major << '.' << minor << "].");
}

inline bool svgMatch(const std::string& testname, const std::vector<char>& response,
                     const char* templateFile)
{
    const std::vector<char> expectedSVG = helpers::readDataFromFile(templateFile);
    if (expectedSVG != response)
    {
        std::ostringstream oss;
        oss << "Svg mismatch: response is\n";
        if(response.empty())
            oss << "<empty>";
        else
            oss << std::string(response.data(), response.size());
        oss << "\nvs. expected (from '" << templateFile << "' :\n";
        oss << std::string(expectedSVG.data(), expectedSVG.size());
        std::string newName = templateFile;
        newName += ".new";
        oss << "Updated template writing to: " << newName << '\n';
        TST_LOG(oss.str());

        FILE *of = fopen(Poco::Path(TDOC, newName).toString().c_str(), "w");
        LOK_ASSERT(of != nullptr);
        LOK_ASSERT(fwrite(response.data(), response.size(), 1, of) == response.size());
        fclose(of);
        return false;
    }
    return true;
}

/// Sends a command and waits for an event in response, with retrying.
inline bool sendAndWait(const std::shared_ptr<http::WebSocketSession>& ws,
                        const std::string& testname, const std::string& command,
                        const std::string& response,
                        std::chrono::milliseconds timeoutPerAttempt = std::chrono::seconds(10),
                        int repeat = COMMAND_RETRY_COUNT)
{
    for (int i = 1; i <= repeat; ++i)
    {
        TST_LOG("Sending [" << command << "], waiting for [" << response << "], attempt #" << i);
        sendTextFrame(ws, command, testname);
        if (!getResponseString(ws, response, testname, timeoutPerAttempt).empty())
            return true;
    }

    return false;
}

/// Drain all events.
/// Draining happens until nothing is received for @timeoutDrain.
inline void drain(const std::shared_ptr<http::WebSocketSession>& ws, const std::string& testname,
                  std::chrono::milliseconds timeoutDrain = std::chrono::milliseconds(300))
{
    TST_LOG("Draining events");

    while (!getResponseString(ws, "", testname, timeoutDrain).empty())
        ; // Skip.
}

/// Sends a command and drain an event in response.
/// We expect @response within the given @timeoutResponse and
/// we drain all messages to get a steady-state.
/// Draining happens for @timeoutDrain.
inline bool sendAndDrain(const std::shared_ptr<http::WebSocketSession>& ws,
                         const std::string& testname, const std::string& command,
                         const std::string& response,
                         std::chrono::milliseconds timeoutResponse = std::chrono::seconds(10),
                         std::chrono::milliseconds timeoutDrain = std::chrono::milliseconds(300))
{
    TST_LOG("Sending [" << command << "], waiting for [" << response
                        << "], and draining the events.");
    sendTextFrame(ws, command, testname);
    const std::string res = getResponseString(ws, response, testname, timeoutResponse);
    if (res.empty())
    {
        TST_LOG("Timed-out waiting for [" << response << "] after sending [" << command << "].");
        return false;
    }

    while (!getResponseString(ws, response, testname, timeoutDrain).empty())
        ; // Skip.

    return true;
}

/// Select all and wait for the text selection update.
inline bool selectAll(const std::shared_ptr<http::WebSocketSession>& ws,
                      const std::string& testname,
                      std::chrono::milliseconds timeoutPerAttempt = std::chrono::seconds(10),
                      int retry = COMMAND_RETRY_COUNT)
{
    return sendAndWait(ws, testname, "uno .uno:SelectAll", "textselection:", timeoutPerAttempt,
                       retry);
}

/// Delete all and wait for the text selection update.
inline bool deleteAll(const std::shared_ptr<http::WebSocketSession>& ws,
                      const std::string& testname,
                      std::chrono::milliseconds timeoutPerAttempt = std::chrono::seconds(10),
                      int retry = COMMAND_RETRY_COUNT)
{
    for (int i = 1; i <= retry; ++i)
    {
        selectAll(ws, testname, timeoutPerAttempt, retry);

        if (sendAndWait(ws, testname, "uno .uno:Delete", "textselection:", timeoutPerAttempt, 1))
            return true;
    }

    return false;
}

inline std::string getAllText(const std::shared_ptr<http::WebSocketSession>& socket,
                              const std::string& testname,
                              const std::string& expected = std::string(),
                              int retry = COMMAND_RETRY_COUNT)
{
    static const std::string prefix = "textselectioncontent: ";

    for (int i = 1; i <= retry; ++i)
    {
        TST_LOG("getAllText attempt #" << i);

        selectAll(socket, testname);

        sendTextFrame(socket, "gettextselection mimetype=text/plain;charset=utf-8", testname);
        std::string text = getResponseString(socket, prefix, testname);
        if (!text.empty())
        {
            if (expected.empty() || (prefix + expected) == text)
                return text;
        }
    }

    return std::string();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
