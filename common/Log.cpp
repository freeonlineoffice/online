/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#ifdef __linux__
#include <sys/prctl.h>
#include <sys/syscall.h>
#endif
#include <unistd.h>

#include <atomic>
#include <cassert>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <Poco/AutoPtr.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/FileChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/SplitterChannel.h>

#include "Common.hpp"
#include "Log.hpp"
#include "Util.hpp"

namespace Log
{
    using namespace Poco;

    class ConsoleChannel : public Poco::Channel
    {
    protected:
        static constexpr std::size_t BufferSize = 64 * 1024;

    public:
        void close() override { flush(); }

        /// Write the given buffer to stderr directly.
        static inline int writeRaw(const char* data, std::size_t size)
        {
            std::size_t i = 0;
            for (; i < size;)
            {
                int wrote;
                while ((wrote = ::write(STDERR_FILENO, data + i, size - i)) < 0 && errno == EINTR)
                {
                }

                if (wrote < 0)
                {
                    return i;
                }

                i += wrote;
            }

            return i;
        }

        template <std::size_t N> inline void writeRaw(const char (&data)[N])
        {
            writeRaw(data, N - 1); // Minus the null.
        }

        inline void writeRaw(const std::string& string) { writeRaw(string.data(), string.size()); }

        /// Flush the stderr file data.
        static inline bool flush() { return ::fflush(stderr) == 0; }

        /// Overloaded log function that takes a naked data pointer to log.
        /// Appends new-line to the given data.
        inline void log(const char* data, std::size_t size)
        {
            static char buffer[BufferSize];
            if (size < sizeof(buffer) - 1)
            {
                memcpy(buffer, data, size);
                buffer[size] = '\n';
                writeRaw(buffer, size + 1);
            }
            else
            {
                // The buffer is too small, we must split the write.
                writeRaw(data, size);
                writeRaw("\n", 1);
            }
        }

        /// Implement the Channel log virtual.
        void log(const Poco::Message& msg) override
        {
            const std::string& s = msg.getText();
            log(s.data(), s.size());
        }
    };

    class BufferedConsoleChannel : public ConsoleChannel
    {
    protected:
        std::size_t size() const { return _size; }
        std::size_t available() const { return BufferSize - _size; }

        inline void flush()
        {
            if (_size)
            {
                writeRaw(_buffer, _size);
                _size = 0;
                ConsoleChannel::flush();
            }
        }

        inline void buffer(const char* data, std::size_t size)
        {
            assert(_size + size <= BufferSize && "Buffer overflow");

            memcpy(_buffer + _size, data, size);
            _size += size;

            assert(_size <= BufferSize && "Buffer overflow");
        }

        template <std::size_t N> inline void buffer(const char (&data)[N])
        {
            buffer(data, N - 1); // Minus the null.
        }

        inline void buffer(const std::string& string) { buffer(string.data(), string.size()); }

    public:
        ~BufferedConsoleChannel() { flush(); }

        void close() override { flush(); }

        inline void log(const char* data, std::size_t size, bool force)
        {
            if (_size + size > BufferSize - 1)
            {
                flush();
                if (size > BufferSize - 1)
                {
                    // The buffer is too small, we must split the write.
                    writeRaw(data, size);
                    writeRaw("\n", 1);
                    return;
                }
            }

            // Fits.
            memcpy(_buffer + _size, data, size);
            _size += size;
            _buffer[_size] = '\n';
            ++_size;

            // Flush important messages and large caches immediately.
            if (force || _size >= BufferSize / 2)
            {
                flush();
            }
        }

        void log(const Poco::Message& msg) override
        {
            const std::string& s = msg.getText();
            log(s.data(), s.size(), msg.getPriority() <= Message::PRIO_WARNING);
        }

    private:
        static thread_local char _buffer[BufferSize];
        static thread_local std::size_t _size;
    };

    thread_local char BufferedConsoleChannel::_buffer[BufferSize];
    thread_local std::size_t BufferedConsoleChannel::_size = 0;

    /// Helper to avoid destruction ordering issues.
    static struct StaticHelper
    {
    private:
        Poco::Logger* _logger;
        static thread_local Poco::Logger* _threadLocalLogger;
        std::string _name;
        std::string _logLevel;
        std::string _id;
        std::atomic<bool> _inited;
    public:
        StaticHelper() :
            _logger(nullptr),
            _inited(true)
        {
        }
        ~StaticHelper()
        {
            _inited = false;
        }

        bool getInited() const { return _inited; }

        void setId(const std::string& id) { _id = id; }

        const std::string& getId() const { return _id; }

        void setName(const std::string& name) { _name = name; }

        const std::string& getName() const { return _name; }

        void setLevel(const std::string& logLevel) { _logLevel = logLevel; }

        const std::string& getLevel() const { return _logLevel; }

        void setLogger(Poco::Logger* logger) { _logger = logger; };

        void setThreadLocalLogger(Poco::Logger* logger)
        {
            // FIXME: What to do with the previous thread-local logger, if any? Will deleting it
            // destroy also its channel? That won't be good as we use the same channel for all
            // loggers. Best to just leak it?
            _threadLocalLogger = logger;
        }

        Poco::Logger* getLogger() const { return _logger; }

        Poco::Logger* getThreadLocalLogger() const { return _threadLocalLogger; }

    } Static;

    thread_local Poco::Logger* StaticHelper::_threadLocalLogger = nullptr;

    bool IsShutdown = false;

    /// Convert an unsigned number to ascii with 0 padding.
    template <int Width> void to_ascii_fixed(char* buf, std::size_t num)
    {
        buf[Width - 1] = '0' + num % 10; // Units.

        if (Width > 1)
        {
            num /= 10;
            buf[Width - 2] = '0' + num % 10; // Tens.
        }

        if (Width > 2)
        {
            num /= 10;
            buf[Width - 3] = '0' + num % 10; // Hundreds.
        }

        if (Width > 3)
        {
            num /= 10;
            buf[Width - 4] = '0' + num % 10; // Thousands.
        }

        if (Width > 4)
        {
            num /= 10;
            buf[Width - 5] = '0' + num % 10; // Ten-Thousands.
        }

        if (Width > 5)
        {
            num /= 10;
            buf[Width - 6] = '0' + num % 10; // Hundred-Thousands.
        }

        static_assert(Width >= 1 && Width <= 6, "Width is invalid.");
    }

    /// Copy a null-terminated string into another.
    /// Expects the destination to be large enough.
    /// Note: unlike strcpy, this returns the *new* out
    /// (destination) pointer, which saves a strlen call.
    char* strcopy(const char* in, char* out)
    {
        while (*in)
            *out++ = *in++;
        return out;
    }

    /// Convert unsigned long num to base-10 ascii in place.
    /// Returns the *end* position.
    char* to_ascii(char* buf, std::size_t num)
    {
        int i = 0;
        do
        {
            buf[i++] = '0' + num % 10;
            num /= 10;
        } while (num > 0);

        // Reverse.
        for (char *front = buf, *back = buf + i - 1; back > front; ++front, --back)
        {
            const char t = *front;
            *front = *back;
            *back = t;
        }

        return buf + i;
    }

    char* prefix(const Poco::LocalDateTime& time, char* buffer, const char* level)
    {
#if defined(IOS) || defined(__FreeBSD__)
        // Don't bother with the "Source" which would be just "Mobile" always and non-informative as
        // there is just one process in the app anyway.
        char *pos = buffer;

        // Don't bother with the thread identifier either. We output the thread name which is much
        // more useful anyway.
#else
        // Note that snprintf is deemed signal-safe in most common implementations.
        char* pos = strcopy((Static.getInited() ? Static.getId().c_str() : "<shutdown>"), buffer);
        *pos++ = '-';

        // Thread ID.
        const auto osTid = Util::getThreadId();
#if defined(__linux__)
        // On Linux osTid is pid_t.
        if (osTid > 99999)
        {
            if (osTid > 999999)
                pos = to_ascii(pos, osTid);
            else
            {
                to_ascii_fixed<6>(pos, osTid);
                pos += 6;
            }
        }
        else
        {
            to_ascii_fixed<5>(pos, osTid);
            pos += 5;
        }
#else
        // On all other systems osTid is std::thread::id.
        std::stringstream ss;
        ss << osTid;
        pos = strcopy(ss.str().c_str(), pos);
#endif

        *pos++ = ' ';
#endif

        // YYYY-MM-DD.
        to_ascii_fixed<4>(pos, time.year());
        pos[4] = '-';
        pos += 5;
        to_ascii_fixed<2>(pos, time.month());
        pos[2] = '-';
        pos += 3;
        to_ascii_fixed<2>(pos, time.day());
        pos[2] = ' ';
        pos += 3;

        // HH:MM:SS.uS
        to_ascii_fixed<2>(pos, time.hour());
        pos[2] = ':';
        pos += 3;
        to_ascii_fixed<2>(pos, time.minute());
        pos[2] = ':';
        pos += 3;
        to_ascii_fixed<2>(pos, time.second());
        pos[2] = '.';
        pos += 3;
        to_ascii_fixed<6>(pos, time.millisecond() * 1000 + time.microsecond());
        pos[6] = ' ';
        pos += 7;

        // Time zone differential
        int tzd = time.tzd();
        if (tzd < 0)
        {
            pos[0] = '-';
            tzd = -tzd;
        }
        else
        {
            pos[0] = '+';
        }
        pos += 1;
        tzd = (tzd / 36) - (tzd / 36) % 100 + ((tzd / 36) % 100) * 60 / 100;  // seconds to HHMM format
        to_ascii_fixed<4>(pos, tzd);
        pos[4] = ' ';
        pos += 5;

        // Thread name and log level
        pos[0] = '[';
        pos[1] = ' ';
        pos += 2;
        pos = strcopy(Util::getThreadName(), pos);
        pos[0] = ' ';
        pos[1] = ']';
        pos[2] = ' ';
        pos += 3;
        pos = strcopy(level, pos);
        pos[0] = ' ';
        pos[1] = ' ';
        pos[2] = '\0';

        return buffer;
    }

    void initialize(const std::string& name,
                    const std::string& logLevel,
                    const bool withColor,
                    const bool logToFile,
                    const std::map<std::string, std::string>& config)
    {
        Static.setName(name);
        std::ostringstream oss;
        oss << Static.getName();
#if !MOBILEAPP // Just one process in a mobile app, the pid is uninteresting.
        oss << '-'
            << std::setw(5) << std::setfill('0') << getpid();
#endif
        Static.setId(oss.str());

        // Configure the logger.
        AutoPtr<Channel> channel;

        if (logToFile)
        {
            channel = static_cast<Poco::Channel*>(new Poco::FileChannel("loolwsd.log"));
            for (const auto& pair : config)
            {
                channel->setProperty(pair.first, pair.second);
            }
        }
        else if (withColor)
        {
            channel = static_cast<Poco::Channel*>(new Poco::ColorConsoleChannel());
            channel->setProperty("traceColor", "green");
            channel->setProperty("warningColor", "magenta");
        }
        else
        {
            if (EnableExperimental)
                channel = static_cast<Poco::Channel*>(new Log::BufferedConsoleChannel());
            else
                channel = static_cast<Poco::Channel*>(new Poco::ConsoleChannel());
        }

        /**
         * Open the channel explicitly, instead of waiting for first log message
         * This is important especially for the kit process where opening the channel
         * after chroot can cause file creation inside the jail instead of outside
         * */
        channel->open();

        try
        {
            auto& logger = Poco::Logger::create(Static.getName(), channel, Poco::Message::PRIO_TRACE);
            Static.setLogger(&logger);
        }
        catch (ExistsException&)
        {
            auto& logger = Poco::Logger::get(Static.getName());
            Static.setLogger(&logger);
        }

        auto logger = Static.getLogger();

        const std::string level = logLevel.empty() ? std::string("trace") : logLevel;
        logger->setLevel(level);
        Static.setLevel(level);

        const std::time_t t = std::time(nullptr);
        struct tm tm;
        LOG_INF("Initializing " << name << ". Local time: "
                                << std::put_time(localtime_r(&t, &tm), "%a %F %T %z")
                                << ". Log level is [" << logger->getLevel() << ']');
    }

    Poco::Logger& logger()
    {
        Poco::Logger* pLogger = Static.getThreadLocalLogger();
        if (pLogger != nullptr)
            return *pLogger;

        pLogger = Static.getLogger();
        return pLogger ? *pLogger
                       : Poco::Logger::get(Static.getInited() ? Static.getName() : std::string());
    }

    void shutdown()
    {
#if !MOBILEAPP
        IsShutdown = true;

        Poco::Logger::shutdown();

        // Flush
        std::flush(std::cout);
        fflush(stdout);
        std::flush(std::cerr);
        fflush(stderr);
#endif
    }

    void setThreadLocalLogLevel(const std::string& logLevel)
    {
        if (!Static.getLogger())
        {
            return;
        }

        // Use the same channel for all Poco loggers.
        auto channel = Static.getLogger()->getChannel();

        // The Poco loggers have to have names that are unique, but those aren't displayed anywhere.
        // So just use the name of the default logger for this process plus a counter.
        static int counter = 1;
        auto& logger = Poco::Logger::create(Static.getName() + "." + std::to_string(counter++),
                                            std::move(channel),
                                            Poco::Logger::parseLevel(logLevel));
        Static.setThreadLocalLogger(&logger);
    }

    const std::string& getLevel()
    {
        return Static.getLevel();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
