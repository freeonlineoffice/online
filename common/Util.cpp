/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "Util.hpp"
#include "Rectangle.hpp"

#if !MOBILEAPP
#include "SigHandlerTrap.hpp"
#endif

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
#  include <execinfo.h>
#  include <cxxabi.h>
#endif
#ifdef __linux__
#  include <sys/prctl.h>
#  include <sys/syscall.h>
#  include <sys/vfs.h>
#  include <sys/resource.h>
#elif defined __FreeBSD__
#  include <sys/resource.h>
#  include <sys/thr.h>
#elif defined IOS
#import <Foundation/Foundation.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>

#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <spawn.h>

#if defined __GLIBC__
#include <malloc.h>
#if defined(M_TRIM_THRESHOLD)
#include <dlfcn.h>
#endif
#endif

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <limits>

#include <Poco/Base64Encoder.h>
#include <Poco/HexBinaryEncoder.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/Exception.h>
#include <Poco/Format.h>

#include <Poco/TemporaryFile.h>
#include <Poco/Util/Application.h>
#include <Poco/URI.h>

// for version info
#include <Poco/Version.h>
#if ENABLE_SSL
#include <openssl/opensslv.h>
#endif
#include <zstd.h>
#define PNG_VERSION_INFO_ONLY
#include <png.h>
#undef PNG_VERSION_INFO_ONLY

#include "Log.hpp"
#include "Protocol.hpp"
#include "TraceEvent.hpp"
#include "common/Common.hpp"

namespace Util
{
    namespace rng
    {
        static std::mutex _rngMutex;

        // Create the prng with a random-device for seed.
        // If we don't have a hardware random-device, we will get the same seed.
        // In that case we are better off with an arbitrary, but changing, seed.
        static std::mt19937_64 _rng = std::mt19937_64(rng::getSeed());

        uint_fast64_t getSeed()
        {
            std::vector<char> hardRandom = getBytes(16);
            uint_fast64_t seed = *reinterpret_cast<uint_fast64_t *>(hardRandom.data());
            return seed;
        }

        // A new seed is used to shuffle the sequence.
        // N.B. Always reseed after getting forked!
        void reseed()
        {
            std::unique_lock<std::mutex> lock(_rngMutex);
            _rng.seed(rng::getSeed());
        }

        // Returns a new random number.
        unsigned getNext()
        {
            std::unique_lock<std::mutex> lock(_rngMutex);
            return _rng();
        }

        /// Generate a string of random characters.
        std::string getHexString(const std::size_t length)
        {
            std::stringstream ss;
            Poco::HexBinaryEncoder hex(ss);
            hex.rdbuf()->setLineLength(0); // Don't insert line breaks.
            hex.write(getBytes(length).data(), length);
            hex.close(); // Flush.
            return ss.str().substr(0, length);
        }

        /// Generates a random string in Base64.
        /// Note: May contain '/' characters.
        std::string getB64String(const std::size_t length)
        {
            std::stringstream ss;
            Poco::Base64Encoder b64(ss);
            b64.write(getBytes(length).data(), length);
            return ss.str().substr(0, length);
        }

        std::string getFilename(const std::size_t length)
        {
            std::string s = getB64String(length * 2);
            s.erase(std::remove_if(s.begin(), s.end(),
                                   [](const std::string::value_type& c)
                                   {
                                       // Remove undesirable characters in a filename.
                                       return c == '/' || c == ' ' || c == '+';
                                   }),
                     s.end());
            return s.substr(0, length);
        }
    } // namespace rng

    bool windowingAvailable()
    {
        return std::getenv("DISPLAY") != nullptr;
    }

    bool kitInProcess = false;
    void setKitInProcess(bool value) { kitInProcess = value; }
    bool isKitInProcess() { return isFuzzing() || isMobileApp() || kitInProcess; }

    std::string replace(std::string result, const std::string& a, const std::string& b)
    {
        const std::size_t aSize = a.size();
        if (aSize > 0)
        {
            const std::size_t bSize = b.size();
            std::string::size_type pos = 0;
            while ((pos = result.find(a, pos)) != std::string::npos)
            {
                result.replace(pos, aSize, b);
                pos += bSize; // Skip the replacee to avoid endless recursion.
            }
        }

        return result;
    }

    std::string replaceAllOf(std::string_view str, std::string_view match, std::string_view repl)
    {
        std::ostringstream os;

        assert(match.size() == repl.size());
        if (match.size() != repl.size())
            return std::string("replaceAllOf failed");

        const std::size_t strSize = str.size();
        for (size_t i = 0; i < strSize; ++i)
        {
            auto pos = match.find(str[i]);
            if (pos != std::string::npos)
                os << repl[pos];
            else
                os << str[i];
        }

        return os.str();
    }

    void replaceAllSubStr(std::string& input, const std::string& target, const std::string& replacement)
    {
        if (target.empty())
            return;

        std::size_t pos = 0;
        while ((pos = input.find(target, pos)) != std::string::npos)
        {
            input.replace(pos, target.length(), replacement);
            pos += replacement.length();
        }
    }

    std::string cleanupFilename(const std::string &filename)
    {
        constexpr std::string_view mtch(",/?:@&=+$#'\"");
        constexpr std::string_view repl("------------");
        return replaceAllOf(filename, mtch, repl);
    }

    std::string formatLinesForLog(const std::string& s)
    {
        std::string r;
        std::string::size_type n = s.size();
        if (n > 0 && s.back() == '\n')
            r = s.substr(0, n-1);
        else
            r = s;
        return replace(std::move(r), "\n", " / ");
    }

    static thread_local long ThreadTid = 0;

    long getThreadId()
    {
        // Avoid so many redundant system calls
#if defined __linux__
        if (!ThreadTid)
            ThreadTid = ::syscall(SYS_gettid);
        return ThreadTid;
#elif defined __FreeBSD__
        if (!ThreadTid)
            thr_self(&ThreadTid);
        return ThreadTid;
#else
        static long threadCounter = 1;
        if (!ThreadTid)
            ThreadTid = threadCounter++;
        return ThreadTid;
#endif
    }

    void killThreadById(int tid, [[maybe_unused]] int signal)
    {
#if defined __linux__
        ::syscall(SYS_tgkill, getpid(), tid, signal);
#else
        LOG_WRN("No tgkill for thread " << tid);
#endif
    }

    // prctl(2) supports names of up to 16 characters, including null-termination.
    // Although in practice on linux more than 16 chars is supported.
    static thread_local char ThreadName[32] = {0};
    static_assert(sizeof(ThreadName) >= 16, "ThreadName should have a statically known size, and not be a pointer.");

    void setThreadName(const std::string& s)
    {
        // Clear the cache - perhaps we forked
        ThreadTid = 0;

        // Copy the current name.
        const std::string knownAs
            = ThreadName[0] ? "known as [" + std::string(ThreadName) + ']' : "unnamed";

        // Set the new name.
        strncpy(ThreadName, s.c_str(), sizeof(ThreadName) - 1);
        ThreadName[sizeof(ThreadName) - 1] = '\0';
#ifdef __linux__
        if (prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(s.c_str()), 0, 0, 0) != 0)
            LOG_SYS("Cannot set thread name of "
                    << getThreadId() << " (" << std::hex << std::this_thread::get_id() << std::dec
                    << ") of process " << getpid() << " currently " << knownAs << " to [" << s
                    << ']');
        else
            LOG_INF("Thread " << getThreadId() << " (" << std::hex << std::this_thread::get_id()
                              << std::dec << ") of process " << getpid() << " formerly " << knownAs
                              << " is now called [" << s << ']');
#elif defined IOS
        [[NSThread currentThread] setName:[NSString stringWithUTF8String:ThreadName]];
        LOG_INF("Thread " << getThreadId() << ") is now called [" << s << ']');
#endif

        // Emit a metadata Trace Event identifying this thread. This will invoke a different function
        // depending on which executable this is in.
        TraceEvent::emitOneRecordingIfEnabled("{\"name\":\"thread_name\",\"ph\":\"M\",\"args\":{\"name\":\""
                                              + s
                                              + "\"},\"pid\":"
                                              + std::to_string(Util::getProcessId())
                                              + ",\"tid\":"
                                              + std::to_string(Util::getThreadId())
                                              + "},\n");
    }

    const char *getThreadName()
    {
        // Main process and/or not set yet.
        if (ThreadName[0] == '\0')
        {
#ifdef __linux__
            // prctl(2): The buffer should allow space for up to 16 bytes; the returned string will be null-terminated.
            if (prctl(PR_GET_NAME, reinterpret_cast<unsigned long>(ThreadName), 0, 0, 0) != 0)
                strncpy(ThreadName, "<noid>", sizeof(ThreadName) - 1);
#elif defined IOS
            const char *const name = [[[NSThread currentThread] name] UTF8String];
            strncpy(ThreadName, name, sizeof(ThreadName) - 1);
#endif
            ThreadName[sizeof(ThreadName) - 1] = '\0';
        }

        // Avoid so many redundant system calls
        return ThreadName;
    }

    std::string getLoolVersion() { return std::string(LOOLWSD_VERSION); }

    std::string getLoolVersionHash() { return std::string(LOOLWSD_VERSION_HASH); }

    void getVersionInfo(std::string& version, std::string& hash)
    {
        version = getLoolVersion();
        hash = getLoolVersionHash();
    }

    const std::string& getProcessIdentifier()
    {
        static std::string id = Util::rng::getHexString(8);

        return id;
    }

    std::string getVersionJSON(bool enableExperimental, const std::string& timezone)
    {
        std::string version, hash;
        Util::getVersionInfo(version, hash);

        std::string pocoVersion;
        pocoVersion += std::to_string((POCO_VERSION & 0xff000000) >> 24) + ".";
        pocoVersion += std::to_string((POCO_VERSION & 0x00ff0000) >> 16) + ".";
        pocoVersion += std::to_string((POCO_VERSION & 0x0000ff00) >> 8);
        std::string zstdVersion;
        zstdVersion += std::to_string(ZSTD_VERSION_MAJOR) + ".";
        zstdVersion += std::to_string(ZSTD_VERSION_MINOR) + ".";
        zstdVersion += std::to_string(ZSTD_VERSION_RELEASE);

        std::string json = "{ \"Version\":     \"" + version +
                           "\", "
                           "\"Hash\":        \"" +
                           hash +
                           "\", "
                           "\"PocoVersion\": \"" +
                           pocoVersion +
                           "\", "
#if ENABLE_SSL
                           "\"OpenSSLVersion\": \"" +
                           std::string(OPENSSL_VERSION_STR) +
                           "\", "
#endif
                           "\"ZstdVersion\": \"" +
                           zstdVersion +
                           "\", "
                           "\"LibPngVersion\": \"" +
                           std::string(PNG_LIBPNG_VER_STRING) +
                           "\", "
                           "\"Protocol\":    \"" +
                           LOOLProtocol::GetProtocolVersion() +
                           "\", "
                           "\"Id\":          \"" +
                           Util::getProcessIdentifier() + "\", ";

        if (!timezone.empty())
            json += "\"TimeZone\":     \"" + timezone + "\", ";

        json += "\"Options\":     \"" + std::string(enableExperimental ? " (E)" : "") + "\" }";
        return json;
    }

    std::string trimURI(const std::string &uriStr)
    {
        Poco::URI uri(uriStr);
        uri.setUserInfo("");
        uri.setPath("");
        uri.setQuery("");
        uri.setFragment("");
        return uri.toString();
    }

    /// Split a string in two at the delimiter and give the delimiter to the first.
    static
    std::pair<std::string, std::string> splitLast2(const std::string& str, const char delimiter = ' ')
    {
        if (!str.empty())
        {
            const char* s = str.c_str();
            const int length = str.size();
            const int pos = getLastDelimiterPosition(s, length, delimiter);
            if (pos < length)
                return std::make_pair(std::string(s, pos + 1), std::string(s + pos + 1));
        }

        // Not found; return in first.
        return std::make_pair(str, std::string());
    }

    std::tuple<std::string, std::string, std::string, std::string> splitUrl(const std::string& url)
    {
        // In case we have a URL that has parameters.
        std::string base;
        std::string params;
        std::tie(base, params) = Util::split(url, '?', false);

        std::string filename;
        std::tie(base, filename) = Util::splitLast2(base, '/');
        if (filename.empty())
        {
            // If no '/', then it's only filename.
            std::swap(base, filename);
        }

        std::string ext;
        std::tie(filename, ext) = Util::splitLast(filename, '.', false);

        return std::make_tuple(base, filename, ext, params);
    }

    std::string getTimeNow(const char* format)
    {
        char time_now[64];
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm;
        time_t_to_gmtime(now_c, now_tm);
        strftime(time_now, sizeof(time_now), format, &now_tm);

        return time_now;
    }

    std::string getHttpTimeNow()
    {
        return getTimeNow("%a, %d %b %Y %T");
    }

    std::string getHttpTime(std::chrono::system_clock::time_point time)
    {
        char http_time[64];
        std::time_t time_c = std::chrono::system_clock::to_time_t(time);
        std::tm time_tm;
        time_t_to_gmtime(time_c, time_tm);
        strftime(http_time, sizeof(http_time), "%a, %d %b %Y %T", &time_tm);

        return http_time;
    }

    std::size_t findInVector(const std::vector<char>& tokens, const char *cstring, std::size_t offset)
    {
        assert(cstring);
        for (std::size_t i = 0; i < tokens.size() - offset; ++i)
        {
            std::size_t j;
            for (j = 0; i + j < tokens.size() - offset && cstring[j] != '\0' && tokens[i + j + offset] == cstring[j]; ++j)
                ;
            if (cstring[j] == '\0')
                return i + offset;
        }
        return std::string::npos;
    }

    // For copyToMatch/seekToMatch
    bool processToMatch(std::istream& in, std::ostream* out, std::string_view search)
    {
        const size_t searchLen = search.length();
        assert(searchLen && "need to search for something");

        const std::streamsize overlap = searchLen - 1;
        std::streamsize carrySize = 0;

        std::vector<char> scratch(READ_BUFFER_SIZE + overlap);
        char* buffer = scratch.data();

        // Read READ_BUFFER_SIZE at a time, keep enough from last iteration to
        // match 'search' against what existed at the end of the last window (but
        // was too short to match) that might match now at the start of this
        // new window.
        while (in)
        {
            in.read(buffer + carrySize, READ_BUFFER_SIZE);
            std::streamsize bytesRead = in.gcount();
            if (!bytesRead)
                break;

            std::streamsize bytesInBuffer = carrySize + bytesRead;

            std::string_view view(buffer, bytesInBuffer);
            const auto match = view.find(search);

            if (match != std::string_view::npos)
            {
                // Copy as far as match
                if (out)
                    out->write(buffer, match);
                // Seek back to before match and return
                in.clear();
                in.seekg(-static_cast<std::streamoff>(bytesInBuffer - match), std::ios_base::cur);
                return true;
            }
            else
            {
                if (out)
                {
                    // Copy what definitely doesn't match so far to output.
                    std::streamsize bytesToWrite = bytesInBuffer > overlap ? bytesInBuffer - overlap : 0;
                    out->write(buffer, bytesToWrite);
                }
                // Rotate <= overlap to start of buffer for next iteration
                carrySize = std::min(overlap, bytesInBuffer);
                std::memmove(buffer, buffer + bytesInBuffer - carrySize, carrySize);
            }
        }

        // write left over
        if (carrySize > 0 && out)
            out->write(buffer, carrySize);
        return false;
    }

    bool seekToMatch(std::istream& in, std::string_view search)
    {
        return processToMatch(in, nullptr, search);
    }

    bool copyToMatch(std::istream& in, std::ostream& out, std::string_view search)
    {
        return processToMatch(in, &out, search);
    }

    std::string getIso8601FracformatTime(std::chrono::system_clock::time_point time){
        char time_modified[64];
        std::time_t lastModified_us_t = std::chrono::system_clock::to_time_t(time);
        std::tm lastModified_tm;
        time_t_to_gmtime(lastModified_us_t, lastModified_tm);
        strftime(time_modified, sizeof(time_modified), "%FT%T.", &lastModified_tm);

        auto lastModified_s = std::chrono::time_point_cast<std::chrono::seconds>(time);

        std::ostringstream oss;
        oss << std::setfill('0')
            << time_modified
            << std::setw(6)
            << (time - lastModified_s).count() / (std::chrono::system_clock::period::den / std::chrono::system_clock::period::num / 1000000)
            << 'Z';

        return oss.str();
    }

#if !MOBILEAPP
    // These are used in test/WhiteBoxTests.cpp and thus not needed in a mobile app.

    std::string time_point_to_iso8601(std::chrono::system_clock::time_point tp)
    {
        const std::time_t tt = std::chrono::system_clock::to_time_t(tp);
        std::tm tm;
        time_t_to_gmtime(tt, tm);

        std::ostringstream oss;
        oss << tm.tm_year + 1900 << '-' << std::setfill('0') << std::setw(2) << tm.tm_mon + 1 << '-'
            << std::setfill('0') << std::setw(2) << tm.tm_mday << 'T';
        oss << std::setfill('0') << std::setw(2) << tm.tm_hour << ':';
        oss << std::setfill('0') << std::setw(2) << tm.tm_min << ':';
        const std::chrono::duration<double> sec
            = tp - std::chrono::system_clock::from_time_t(tt) + std::chrono::seconds(tm.tm_sec);
        if (sec.count() < 10)
            oss << '0';
        oss << std::fixed << sec.count() << 'Z';

        return oss.str();
    }

    std::chrono::system_clock::time_point iso8601ToTimestamp(const std::string& iso8601Time,
                                                             const std::string& logName)
    {
        std::chrono::system_clock::time_point timestamp;
        std::tm tm;
        const char* cstr = iso8601Time.c_str();
        const char* trailing;
        if (!(trailing = strptime(cstr, "%Y-%m-%dT%H:%M:%S", &tm)))
        {
            LOG_WRN(logName << " [" << iso8601Time << "] is in invalid format."
                            << "Returning " << timestamp.time_since_epoch().count());
            return timestamp;
        }

        timestamp += std::chrono::seconds(timegm(&tm));
        if (trailing[0] == '\0')
            return timestamp;

        if (trailing[0] != '.')
        {
            LOG_WRN(logName << " [" << iso8601Time << "] is in invalid format."
                            << ". Returning " << timestamp.time_since_epoch().count());
            return timestamp;
        }

        char* end = nullptr;
        const std::size_t us = strtoul(trailing + 1, &end, 10); // Skip the '.' and read as integer.

        std::size_t denominator = 1;
        for (const char* i = trailing + 1; i != end; i++)
        {
            denominator *= 10;
        }

        const std::size_t seconds_us = us * std::chrono::system_clock::period::den
                                       / std::chrono::system_clock::period::num / denominator;

        timestamp += std::chrono::system_clock::duration(seconds_us);

        return timestamp;
    }

#endif // !MOBILEAPP

    /// Returns the given system_clock time_point as string in the local time.
    /// Format: Thu Jan 27 03:45:27.123 2022
    std::string getSystemClockAsString(const std::chrono::system_clock::time_point time)
    {
        const auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(time);
        const std::time_t t = std::chrono::system_clock::to_time_t(ms);
        const int msFraction =
            std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch())
                .count() %
            1000;

        std::tm tm;
        time_t_to_localtime(t, tm);

        char buffer[128] = { 0 };
        std::strftime(buffer, 80, "%a %b %d %H:%M:%S", &tm);
        std::stringstream ss;
        ss << buffer << '.' << std::setfill('0') << std::setw(3) << msFraction << ' '
           << tm.tm_year + 1900;
        return ss.str();
    }

    std::map<std::string, std::string> stringVectorToMap(const std::vector<std::string>& strvector, const char delimiter)
    {
        std::map<std::string, std::string> result;

        for (auto it = strvector.begin(); it != strvector.end(); ++it)
        {
            std::size_t delimiterPosition = 0;
            delimiterPosition = (*it).find(delimiter, 0);
            if (delimiterPosition != std::string::npos)
            {
                std::string key = (*it).substr(0, delimiterPosition);
                delimiterPosition++;
                result[key] = (*it).substr(delimiterPosition);
            }
            else
            {
                LOG_WRN("Util::stringVectorToMap => record is misformed: " << (*it));
            }
        }

        return result;
    }

    static std::string ApplicationPath;
    void setApplicationPath(const std::string& path)
    {
        ApplicationPath = Poco::Path(path).absolute().toString();
    }

    std::string getApplicationPath()
    {
        return ApplicationPath;
    }

    int safe_atoi(const char* p, int len)
    {
        long ret{};
        if (!p || !len)
        {
            return ret;
        }

        int multiplier = 1;
        int offset = 0;
        while (isspace(p[offset]))
        {
            ++offset;
            if (offset >= len)
            {
                return ret;
            }
        }

        switch (p[offset])
        {
            case '-':
                multiplier = -1;
                ++offset;
                break;
            case '+':
                ++offset;
                break;
        }
        if (offset >= len)
        {
            return ret;
        }

        while (isdigit(p[offset]))
        {
            std::int64_t next = ret * 10 + (p[offset] - '0');
            if (next >= std::numeric_limits<int>::max())
                return multiplier * std::numeric_limits<int>::max();
            ret = next;
            ++offset;
            if (offset >= len)
            {
                return multiplier * ret;
            }
        }

        return multiplier * ret;
    }

    void forcedExit(int code)
    {
        if (code)
            LOG_FTL("Forced Exit with code: " << code);
        else
            LOG_INF("Forced Exit with code: " << code);

        Log::shutdown();

#if CODE_COVERAGE
        __gcov_dump();
#endif

#if !MOBILEAPP
        /// Wait for the signal handler, if any,
        /// and prevent _Exit while collecting backtrace.
        SigUtil::SigHandlerTrap::wait();
#endif

        std::_Exit(code);
    }

    std::string getMallocInfo()
    {
        std::string info;

#if defined __GLIBC__
        size_t size = 0;
        char* p = nullptr;
        FILE* f = open_memstream(&p, &size);
        if (f)
        {
            // Dump malloc internal structures.
            malloc_info(0, f);
            fclose(f);

            if (size)
                info = std::string(p, size);

            free(p);
        }
#endif // __GLIBC__

        return info;
    }

    void assertCorrectThread(std::thread::id owner, const char* fileName, int lineNo)
    {
        // uninitialized owner means detached and can be invoked by any thread.
        const bool sameThread = (owner == std::thread::id() || owner == std::this_thread::get_id());
        if (!sameThread)
            LOG_ERR("Incorrect thread affinity. Expected: "
                    << Log::to_string(owner) << " but called from "
                    << Log::to_string(std::this_thread::get_id()) << " (" << Util::getThreadId()
                    << "). (" << fileName << ":" << lineNo << ")");

        assert(sameThread);
    }

    void sleepFromEnvIfSet(const char *domain, const char *envVar)
    {
        const char *value;
        if ((value = std::getenv(envVar)))
        {
            const size_t delaySecs = std::stoul(value);
            if (delaySecs > 0)
            {
                std::cerr << domain << ": Sleeping " << delaySecs
                          << " seconds to give you time to attach debugger to process "
                          << Util::getProcessId() << std::endl
                          << "sudo gdb --pid=" << Util::getProcessId() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(delaySecs));
            }
        }
    }

    std::string Backtrace::Symbol::toString() const
    {
        std::string s;
        if (isDemangled())
        {
            s.append(demangled);
            s.append(" <= ");
        }
        s.append(mangled);
        if (!offset.empty())
        {
            s.append("+").append(offset);
        }
        if (!blob.empty())
        {
            s.append(" @ ").append(blob);
        }
        return s;
    }
    std::string Backtrace::Symbol::toMangledString() const
    {
        std::string s;
        s.append(mangled);
        if (!offset.empty())
        {
            s.append("+").append(offset);
        }
        if (!blob.empty())
        {
            s.append(" @ ").append(blob);
        }
        return s;
    }
    bool Backtrace::separateRawSymbol(const std::string& raw, Symbol& s)
    {
        auto idx0 = raw.find('(');
        if (idx0 != std::string::npos)
        {
            auto idx2 = raw.find(')', idx0 + 1);
            if (idx2 != std::string::npos && idx2 > idx0)
            {
                auto idx1 = raw.find('+', idx0 + 1);
                if (idx1 != std::string::npos && idx1 > idx0 && idx1 < idx2)
                {
                    //  0123456789abcd
                    // "abc(def+0x123)"
                    s.blob = raw.substr(0, idx0);
                    s.mangled = raw.substr(idx0 + 1, idx1 - idx0 - 1);
                    s.offset = raw.substr(idx1 + 1, idx2 - idx1 - 1);
                    return true;
                }
            }
        }
        s.mangled = raw;
        return false;
    }

    Backtrace::Backtrace([[maybe_unused]] const int maxFrames, const int skip)
        : skipFrames(skip)
    {
#if defined(__linux) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
        std::vector<void*> backtraceBuffer(maxFrames + skip, nullptr);

        const int numSlots = ::backtrace(backtraceBuffer.data(), backtraceBuffer.size());
        if (numSlots > 0)
        {
            char** rawSymbols = ::backtrace_symbols(backtraceBuffer.data(), numSlots);
            if (rawSymbols)
            {
                for (int i = skip; i < numSlots; ++i)
                {
                    Symbol symbol;
                    separateRawSymbol(rawSymbols[i], symbol);
                    int status;
                    char* demangled;
                    std::string s("`");
                    if ((demangled = abi::__cxa_demangle(symbol.mangled.c_str(), nullptr, nullptr,
                                                         &status)) != nullptr)
                    {
                        symbol.demangled = demangled;
                        free(demangled);
                    }
                    _frames.emplace_back(backtraceBuffer[i], symbol);
                }
                free(rawSymbols);
            }
        }
#endif
        if (0 == _frames.size())
        {
            _frames.emplace_back(nullptr, Symbol{"n/a", "empty", "0x00", ""});
        }
    }

    std::ostream& Backtrace::send(std::ostream& os) const
    {
        os << "Backtrace:\n";
        int fidx = skipFrames;
        for (const auto& p : _frames)
        {
            const Symbol& sym = p.second;
            if (sym.isDemangled())
            {
                os << fidx++ << ": " << sym.demangled << "\n";
                os << "\t" << sym.toMangledString() << '\n';
            }
            else
            {
                os << fidx++ << ": " << sym.toMangledString() << '\n';
            }
        }
        return os;
    }
    std::string Backtrace::toString() const
    {
        std::string s = "Backtrace:\n";
        int fidx = skipFrames;
        for (const auto& p : _frames)
        {
            const Symbol& sym = p.second;
            if (sym.isDemangled())
            {
                s.append(std::to_string(fidx++)).append(": ").append(sym.demangled).append("\n");
                s.append("\t").append(sym.toMangledString()).append("\n");
            }
            else
            {
                s.append(std::to_string(fidx++))
                    .append(": ")
                    .append(sym.toMangledString())
                    .append("\n");
            }
        }
        return s;
    }

    Rectangle::Rectangle(const std::string &rectangle)
    {
        StringVector tokens(StringVector::tokenize(rectangle, ','));
        if (tokens.size() == 4)
        {
            _x1 = std::stoi(tokens[0]);
            _y1 = std::stoi(tokens[1]);
            _x2 = _x1 + std::stoi(tokens[2]);
            _y2 = _y1 + std::stoi(tokens[3]);
        }
        else
        {
            _x1 = _y1 = _x2 = _y2 = 0;
        }
    }

    std::string base64Encode(std::string_view input)
    {
        std::ostringstream oss;
        Poco::Base64Encoder encoder(oss);
        encoder << input;
        encoder.close();
        return oss.str();
    }

} // namespace Util

#if !MOBILEAPP
namespace SigUtil {
    std::atomic<int> SigHandlerTrap::SigHandling;
} // end namespace SigUtil
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
