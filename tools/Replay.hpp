/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <math.h>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <unordered_map>

#include "Socket.hpp"
#include "WebSocketHandler.hpp"
#include <TraceFile.hpp>
#include <Util.hpp>
#include <common/Log.hpp>
#include <net/Ssl.hpp>
#include <wsd/TileDesc.hpp>
#if ENABLE_SSL
#  include <SslSocket.hpp>
#endif

#include <iostream>
#include <fstream>
#include <ctime>
#include <utility>

struct PerfMetricInfo
{
    std::string _phase;
    std::string _metric;
    size_t _data;

    PerfMetricInfo(const std::string& phase, const std::string& metric, size_t data)
        : _phase(phase)
        , _metric(metric)
        , _data(data)
    {
    }
};

// store buckets of latency
struct Histogram {
    const size_t incLowMs = 10;
    const size_t maxLowMs = incLowMs * 10;
    const size_t incHighMs = 100;
    const size_t maxHighMs = incHighMs * 10;
    size_t _items;
    size_t _tooLong;
    std::vector<size_t> _buckets;

    Histogram() : _items(0), _tooLong(0), _buckets(20)
    {
    }

    void addTime(size_t ms)
    {
        if (ms < maxLowMs)
            _buckets[ms/incLowMs]++;
        else if (ms < maxHighMs)
            _buckets[(ms - maxLowMs) / maxHighMs]++;
        else
            _tooLong++;
        _items++;
    }

    void dump(const char *legend)
    {
        size_t max = 0;
        ssize_t firstBucket = -1;
        for (size_t i = 0; i < _buckets.size(); ++i)
        {
            size_t n = _buckets[i];
            if (n > 0 && firstBucket < 0)
                firstBucket = i;
            max = std::max(max, n);
        }
        if (firstBucket < 0 || max == 0)
            return;

        size_t last; // ignore
        for (last = _buckets.size()-1; last > 0; --last)
            if (_buckets[last] > 0)
                break;

        std::cout << legend << ' ' << _items << " items, max #: " << max
                  << " too long: " << _tooLong << '\n';

        const double chrsPerFreq = 60.0 / max;
        for (size_t i = firstBucket; i <= last; ++i)
        {
            int chrs = ::ceil(chrsPerFreq * _buckets[i]);
            int ms = i < 10 ? (incLowMs * (i+1)) : (maxLowMs + (i+1-10) * incHighMs);
            std::cout << "< " << std::setw(4) << ms << " ms |" << std::string(chrs, '-') << "| "
                      << _buckets[i] << '\n';
        }
    }

    std::vector<PerfMetricInfo> getLatencyStats(const std::string& typeOfLatency,
                                                const std::string& testPhase)
    {
        size_t totalTiles = _items;
        size_t subTenCount = _buckets[0];
        size_t subOneHundredCount = 0;
        size_t overOneHundredCount = _tooLong;

        for(size_t i = 1 ; i < _buckets.size(); i++)
        {
            if(i < 10)
            {
                subOneHundredCount += _buckets[i];
            }
            else
            {
                overOneHundredCount += _buckets[i];
            }
        }

        std::vector<PerfMetricInfo> latencyStatsList;
        latencyStatsList.push_back(PerfMetricInfo {testPhase, typeOfLatency + " Total tiles", totalTiles});
        latencyStatsList.push_back(PerfMetricInfo {testPhase, typeOfLatency + " Sub_10ms", subTenCount});
        latencyStatsList.push_back(PerfMetricInfo {testPhase, typeOfLatency + " Sub_100ms", subOneHundredCount});
        latencyStatsList.push_back(PerfMetricInfo {testPhase, typeOfLatency + " Over_100ms", overOneHundredCount});

        return latencyStatsList;
    }

};

struct Stats {
    Stats() :
        _start(std::chrono::steady_clock::now()),
        _bytesSent(0),
        _bytesRecvd(0),
        _tileCount(0),
        _connections(0)
    {
        _startUpMemoryUsage = getMemoryUsage();
        _timer.reset(new Util::SysStopwatch());
        _peakMemoryUsage = 0;
    }
    std::chrono::steady_clock::time_point _start;
    std::unique_ptr<Util::SysStopwatch> _timer;
    size_t _bytesSent;
    size_t _bytesRecvd;
    size_t _tileCount;
    size_t _connections;
    Histogram _pingLatency;
    Histogram _tileLatency;

    size_t _peakMemoryUsage;
    size_t _startUpMemoryUsage;


    std::string _testType;

    // message size breakdown
    struct MessageStat {
        size_t size;
        size_t count;
    };
    std::unordered_map<std::string, MessageStat> _recvd;
    std::unordered_map<std::string, MessageStat> _sent;

    std::vector<PerfMetricInfo> _perfStatsList;

    size_t getMemoryUsage()
    {
        std::ifstream smapsFile("/proc/" + std::to_string(getpid()) + "/smaps");

        std::string line;
        size_t totalDirtyPss = 0;

        while (std::getline(smapsFile, line))
        {
            if (line.find("Private_Dirty:") == 0)
            {
                std::stringstream ss(line);
                std::string key;
                size_t value;
                // coverity[tainted_data_argument : FALSE] - we trust the kernel-provided data
                ss >> key >> value;
                totalDirtyPss += value;
            }
        }

        return totalDirtyPss;
    }

    void accumulate(std::unordered_map<std::string, MessageStat>& map, const std::string& token,
                    size_t size)
    {
        auto it = map.find(token);
        MessageStat st = { 0, 0 };
        if (it != map.end())
            st = it->second;
        st.size += size;
        st.count++;
        map[token] = st;
    }

    void accumulateRecv(const std::string &token, size_t size)
    {
        _bytesRecvd += size;
        accumulate(_recvd, token, size);
    }

    void accumulateSend(const char* msg, const size_t len, bool /* flush */)
    {
        _bytesSent += len;
        size_t i;
        for (i = 0; i < len && msg[i] != ' '; ++i);
        accumulate(_sent, std::string(msg, std::min(i, size_t(len))), len);
    }

    void addConnection() { _connections++; }

    void dumpMap(std::unordered_map<std::string, MessageStat> &map)
    {
        // how much from each command ?
        std::vector<std::string> sortKeys;
        size_t total = 0;
        for(const auto& it : map)
        {
            sortKeys.push_back(it.first);
            total += it.second.size;
        }
        std::sort(sortKeys.begin(), sortKeys.end(),
                  [&](const std::string &a, const std::string &b)
                      { return map[a].size > map[b].size; } );
        std::cout << "size\tcount\tcommand\n";
        for (const auto& it : sortKeys)
        {
            std::cout << map[it].size << '\t' << map[it].count << '\t' << it << '\n';
            if (map[it].size < (total / 100))
                break;
        }
    }

    void dump()
    {
        const auto now = std::chrono::steady_clock::now();
        const size_t runMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - _start).count();

        std::cout << "Peak memory usage: " << _peakMemoryUsage << "kB";
        std::cout << "Stress run took " << runMs << " ms\n";
        std::cout << "  tiles: " << _tileCount << " => TPS: " << ((_tileCount * 1000.0)/runMs) << "\n";
        _pingLatency.dump("ping latency:");
        _tileLatency.dump("tile latency:");
        size_t recvKbps = (_bytesRecvd * 1000) / (_connections * runMs * 1024);
        size_t sentKbps = (_bytesSent * 1000) / (_connections * runMs * 1024);
        std::cout << "  we sent " << Util::getHumanizedBytes(_bytesSent) <<
            " (" << sentKbps << " kB/s) " <<
            " server sent " << Util::getHumanizedBytes(_bytesRecvd) <<
            " (" << recvKbps << " kB/s) to " << _connections << " connections.\n";


       endPhase(Log::Phase::Edit);
       dumpPerfStatsToCSV(_perfStatsList);

        std::cout << "we sent:\n";
        dumpMap(_sent);

        std::cout << "server sent us:\n";
        dumpMap(_recvd);
    }


    void endPhase(Log::Phase phase)
    {
        const std::string phaseAsString{ nameShort(phase) };

        const auto now = std::chrono::steady_clock::now();
        const size_t runMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - _start).count();
        _start = std::chrono::steady_clock::now();

        size_t cpuTime = _timer->elapsedTime().count();
        _timer.reset(new Util::SysStopwatch);

        _perfStatsList.push_back(getStressStats(runMs, phaseAsString));
        _perfStatsList.push_back(getCPUUSageStats(cpuTime, phaseAsString));

        if(phase == Log::Phase::Edit)
        {
            std::vector<PerfMetricInfo> statsList;

            _perfStatsList.push_back(getPeakMemoryUsageStats(_peakMemoryUsage, phaseAsString));

            statsList = getNetworkStats(_bytesRecvd / 1000, _bytesSent / 1000, phaseAsString);
            for(size_t i = 0; i < statsList.size(); i++)
            {
                _perfStatsList.push_back(statsList[i]);
            }

            statsList = _pingLatency.getLatencyStats("PL", phaseAsString);
            for(size_t i = 0; i < statsList.size(); i++)
            {
                _perfStatsList.push_back(statsList[i]);
            }

            statsList = _tileLatency.getLatencyStats("TL", phaseAsString);
            for(size_t i = 0; i < statsList.size(); i++)
            {
                _perfStatsList.push_back(statsList[i]);
            }
        }
    }

    PerfMetricInfo getStressStats(size_t runMs, std::string testPhase)
    {
        return PerfMetricInfo(std::move(testPhase), "Stress run (ms)", runMs);
    }

    PerfMetricInfo getCPUUSageStats(size_t cpuUsage, std::string testPhase)
    {
        return PerfMetricInfo(std::move(testPhase), "CPU Usage (us)", cpuUsage);
    }

    std::vector<PerfMetricInfo> getNetworkStats(size_t recievedKb, size_t sentKb, const std::string& testPhase)
    {
        std::vector<PerfMetricInfo> networkStatsList;

        networkStatsList.emplace_back(testPhase, "Bytes recieved (kB)", recievedKb);
        networkStatsList.emplace_back(testPhase, "Bytes sent (kB)", sentKb);

        return networkStatsList;
    }

    PerfMetricInfo getPeakMemoryUsageStats(size_t peakMemory, std::string testPhase)
    {
        return PerfMetricInfo(std::move(testPhase), "Peak memory usage (kB)", peakMemory);
    }

    void dumpPerfStatsToCSV(std::vector<PerfMetricInfo> perfData)
    {
        std::ofstream file("PerformanceMetricsSummary.csv", std::ios::out | std::ios::app);

        if(file.tellp() == 0)
        {
            file << "Commit Hash" << ',' << "Date" << ',' << "Test" << ',' << "Phase" << ','
                 << "Metric" << ',' << "Value";
            file << '\n';
        }

        std::string commitHash = Util::getLoolVersionHash();

        time_t now = time(0);
        struct tm datetime;
        localtime_r(&now, &datetime);

        char formattedDate[50];
        strftime(formattedDate, 50, "%d/%m/%y", &datetime);

        for(size_t i = 0; i < perfData.size(); i++)
        {
            file << commitHash << ',' << formattedDate << ',' << _testType << ','
                 << perfData[i]._phase << ',' << perfData[i]._metric << ',' << perfData[i]._data;
            file << '\n';
        }
    }

    void setTypeOfTest(const std::string& testType)
    {
        _testType = testType;
    }

};

// Avoid a MessageHandler for now.
class StressSocketHandler : public WebSocketHandler
{
    SocketPoll &_poll;
    TraceFileReader _reader;
    TraceFileRecord _next;
    std::chrono::steady_clock::time_point _start;
    std::chrono::steady_clock::time_point _nextPing;
    bool _connecting;
    std::string _logPre;
    std::string _uri;
    std::string _trace;

    std::shared_ptr<Stats> _stats;
    std::chrono::steady_clock::time_point _lastTile;

public:
    StressSocketHandler(SocketPoll& poll, /* bad style */
                        const std::shared_ptr<Stats>& stats, const std::string& uri,
                        const std::string& trace, const int delayMs = 0)
        : WebSocketHandler(true, true)
        , _poll(poll)
        , _reader(trace)
        , _connecting(true)
        , _uri(uri)
        , _trace(trace)
        , _stats(stats)
    {
        assert(_stats && "stats must be provided");

        static std::atomic<int> number;
        _logPre = '[' + std::to_string(++number) + "] ";
        std::cerr << "Attempt connect to " << uri << " for trace " << _trace << '\n';
        getNextRecord();
        _start = std::chrono::steady_clock::now() + std::chrono::milliseconds(delayMs);
        _nextPing = _start + std::chrono::milliseconds(Util::rng::getNext() % 1000);
        _lastTile = _start;
    }

    void gotPing(WSOpCode /* code */, int pingTimeUs) override
    {
        _stats->_pingLatency.addTime(pingTimeUs/1000);
    }

    int getPollEvents(std::chrono::steady_clock::time_point now,
                      int64_t &timeoutMaxMicroS) override
    {
        if (_connecting)
        {
            std::cerr << _logPre << "Waiting for outbound connection to " << _uri
                      << " to complete for trace " << _trace << '\n';
            return POLLOUT;
        }

        int events = WebSocketHandler::getPollEvents(now, timeoutMaxMicroS);

        if (now >= _nextPing)
        {
            // ping more frequently
            sendPing(now, getSocket().lock());
            _nextPing += std::chrono::seconds(1);
        }

        int64_t nextTime = -1;
        while (nextTime <= 0) {
            nextTime = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::microseconds((_next.getTimestampUs() - _reader.getEpochStart()) * TRACE_MULTIPLIER)
                + _start - now).count();
            if (nextTime <= 0)
            {
                sendTraceMessage();
                events = WebSocketHandler::getPollEvents(now, timeoutMaxMicroS);
                break;
            }
        }

//        std::cerr << "next event in " << nextTime << " us\n";
        if (nextTime < timeoutMaxMicroS)
            timeoutMaxMicroS = nextTime;

        return events;
    }

    bool getNextRecord()
    {
        bool found = false;
        while (!found) {
            _next = _reader.getNextRecord();
            switch (_next.getDir()) {
            case TraceFileRecord::Direction::Invalid:
            case TraceFileRecord::Direction::Incoming:
                // FIXME: need to subset output quite a bit.
                found = true;
                break;
            default:
                found = false;
                break;
            }
        }
        return _next.getDir () != TraceFileRecord::Direction::Invalid;
    }

    void performWrites(std::size_t capacity) override
    {
        if (_connecting)
            std::cerr << _logPre << "Outbound websocket - connected\n";
        _connecting = false;
        return WebSocketHandler::performWrites(capacity);
    }

    void onDisconnect() override
    {
        std::cerr << _logPre << "Websocket " << _uri <<
            " dis-connected, re-trying in 20 seconds\n";
        WebSocketHandler::onDisconnect();
    }

    // send outgoing messages
    void sendTraceMessage()
    {
        if (_next.getDir() == TraceFileRecord::Direction::Invalid)
            return; // shutting down

        std::string msg = rewriteMessage(_next.getPayload());
        if (!msg.empty())
        {
            std::cerr << _logPre << "Send: '" << msg << "'\n";
            sendMessage(msg);
        }

        if (!getNextRecord())
        {
            std::cerr << _logPre << "Shutdown\n";
            shutdown();
        }
    }

    std::string rewriteMessage(const std::string &msg)
    {
        StringVector tokens = StringVector::tokenize(LOOLProtocol::getFirstLine(msg));

        std::string out = msg;

        if (tokens.equals(0, "tileprocessed"))
            out.clear(); // we do this accurately below
        else if (tokens.equals(0, "requestloksession") || (tokens.equals(0, "canceltiles")))
            out.clear(); // These commands have been removed.
        else if (tokens.equals(0, "uno .uno:Save"))
            out = "save dontTerminateEdit=0 dontSaveIfUnmodified=1"; // .uno:Save will crash in debug

        else if (tokens.equals(0, "load")) {
            std::string url = tokens[1];
            assert(!strncmp(url.c_str(), "url=", 4));

            // load url=file%3A%2F%2F%2Ftmp%2Fhello-world.odt deviceFormFactor=desktop
            out = "load url=" + _uri; // already encoded
            for (size_t i = 2; i < tokens.size(); ++i)
                out += ' ' + tokens[i];
            std::cerr << _logPre << "msg " << out << '\n';
        }

        size_t currentMemoryUsage = _stats->getMemoryUsage();
        if (currentMemoryUsage > _stats->_startUpMemoryUsage)
        {
            size_t postDocumentLoadingMemory = currentMemoryUsage - _stats->_startUpMemoryUsage;
            if (postDocumentLoadingMemory > _stats->_peakMemoryUsage)
            {
                _stats->_peakMemoryUsage = postDocumentLoadingMemory;
            }
        }
        // FIXME: translate mouse events relative to view-port etc.
        return out;
    }

    // handle incoming messages
    void handleMessage(const std::vector<char> &data) override
    {
        const auto now = std::chrono::steady_clock::now();

        const std::string firstLine = LOOLProtocol::getFirstLine(data.data(), data.size());
        StringVector tokens = StringVector::tokenize(firstLine);
        std::cerr << _logPre << "Got msg: " << firstLine << '\n';

        _stats->accumulateRecv(tokens[0], data.size());

        if (tokens.equals(0, "tile:")) {
            // accumulate latencies
            _stats->_tileLatency.addTime(std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastTile).count());
            _stats->_tileCount++;
            _lastTile = now;

            // eg. tileprocessed tile=0:9216:0:3072:3072:0
            TileDesc desc = TileDesc::parse(tokens);

            sendMessage("tileprocessed tile=" + desc.generateID());
            std::cerr << _logPre << "Sent tileprocessed tile= " + desc.generateID() << '\n';
        }
        else if (tokens.equals(0, "error:"))
        {
            bool reconnect = false;
            if (firstLine == "error: cmd=load kind=docunloading")
            {
                std::cerr << ": wait and try again later ...!\n";
                reconnect = true;
            }
            else if (firstLine == "error: cmd=storage kind=documentconflict")
            {
                std::cerr << "Document conflict - need to resolve it first ...\n";
                sendMessage("closedocument");
                reconnect = true;
            }
            else
            {
                std::cerr << _logPre << "Error while processing " << _uri
                          << " and trace " << _trace << ":\n"
                          << "'" << firstLine << "'\n";
            }

            if (reconnect)
            {
                shutdown(true, "bye");
                auto handler = std::make_shared<StressSocketHandler>(
                    _poll, _stats, _uri, _trace, 1000 /* delay 1 second */);
                _poll.insertNewWebSocketSync(Poco::URI(_uri), handler);
                return;
            }
            else
                Util::forcedExit(70);
        }

        // FIXME: implement code to send new view-ports based
        // on cursor position etc.
    }

    /// override ProtocolHandlerInterface piece
    int sendTextMessage(const char* msg, const size_t len, bool flush = false) const override
    {
        _stats->accumulateSend(msg, len, flush);
        return WebSocketHandler::sendTextMessage(msg, len, flush);
    }

    static void addPollFor(SocketPoll &poll, const std::string &server,
                           const std::string &filePath, const std::string &tracePath,
                           const std::shared_ptr<Stats> &optStats)
    {
        assert(optStats && "optStats must be provided");

        std::string file, wrap;
        std::string fileabs = Poco::Path(filePath).makeAbsolute().toString();
        Poco::URI::encode("file://" + fileabs, ":/?", file);
        Poco::URI::encode(file, ":/?", wrap); // double encode.
        std::string uri = server + "/lool/" + wrap + "/ws";

        auto handler = std::make_shared<StressSocketHandler>(poll, optStats, file, tracePath);
        poll.insertNewWebSocketSync(Poco::URI(uri), handler);

        optStats->addConnection();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
