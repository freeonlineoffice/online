/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <common/Anonymizer.hpp>
#include <common/Common.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Message.hpp>
#include <common/Protocol.hpp>
#include <common/RegexUtil.hpp>
#include <common/StateEnum.hpp>
#include <common/ThreadPool.hpp>
#include <common/Util.hpp>
#include <wsd/TileCache.hpp>
#include <wsd/TileDesc.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <chrono>
#include <cstddef>
#include <fstream>
#include <sstream>

/// WhiteBox unit-tests.
class WhiteBoxTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(WhiteBoxTests);
    CPPUNIT_TEST(testLOOLProtocolFunctions);
    CPPUNIT_TEST(testSplitting);
    CPPUNIT_TEST(testMessage);
    CPPUNIT_TEST(testPathPrefixTrimming);
    CPPUNIT_TEST(testMessageAbbreviation);
    CPPUNIT_TEST(testReplace);
    CPPUNIT_TEST(testReplaceChar);
    CPPUNIT_TEST(testReplaceCharInPlace);
    CPPUNIT_TEST(testReplaceAllOf);
    CPPUNIT_TEST(testRegexListMatcher);
    CPPUNIT_TEST(testRegexListMatcher_Init);
    CPPUNIT_TEST(testTileDesc);
    CPPUNIT_TEST(testTileData);
    CPPUNIT_TEST(testRectanglesIntersect);
    CPPUNIT_TEST(testJson);
    CPPUNIT_TEST(testAnonymization);
    CPPUNIT_TEST(testIso8601Time);
    CPPUNIT_TEST(testClockAsString);
    CPPUNIT_TEST(testStat);
    CPPUNIT_TEST(testStringCompare);
    CPPUNIT_TEST(testSafeAtoi);
    CPPUNIT_TEST(testJsonUtilEscapeJSONValue);
    CPPUNIT_TEST(testStateEnum);
    CPPUNIT_TEST(testFindInVector);
    CPPUNIT_TEST(testThreadPool);
    CPPUNIT_TEST_SUITE_END();

    void testLOOLProtocolFunctions();
    void testSplitting();
    void testMessage();
    void testPathPrefixTrimming();
    void testMessageAbbreviation();
    void testReplace();
    void testReplaceChar();
    void testReplaceCharInPlace();
    void testReplaceAllOf();
    void testRegexListMatcher();
    void testRegexListMatcher_Init();
    void testTileDesc();
    void testTileData();
    void testRectanglesIntersect();
    void testJson();
    void testAnonymization();
    void testIso8601Time();
    void testClockAsString();
    void testStat();
    void testStringCompare();
    void testSafeAtoi();
    void testJsonUtilEscapeJSONValue();
    void testStateEnum();
    void testFindInVector();
    void testThreadPool();

    size_t waitForThreads(size_t count);
};

void WhiteBoxTests::testLOOLProtocolFunctions()
{
    constexpr std::string_view testname = __func__;

    int foo;
    LOK_ASSERT(LOOLProtocol::getTokenInteger("foo=42", "foo", foo));
    LOK_ASSERT_EQUAL(42, foo);

    std::string bar;
    LOK_ASSERT(LOOLProtocol::getTokenString("bar=hello-sailor", "bar", bar));
    LOK_ASSERT_EQUAL(std::string("hello-sailor"), bar);

    LOK_ASSERT(LOOLProtocol::getTokenString("bar=", "bar", bar));
    LOK_ASSERT_EQUAL(std::string(""), bar);

    int mumble;
    std::map<std::string, int> map { { "hello", 1 }, { "goodbye", 2 }, { "adieu", 3 } };

    LOK_ASSERT(LOOLProtocol::getTokenKeyword("mumble=goodbye", "mumble", map, mumble));
    LOK_ASSERT_EQUAL(2, mumble);

    std::string message("hello x=1 y=2 foo=42 bar=hello-sailor mumble='goodbye' zip zap");
    StringVector tokens(StringVector::tokenize(message));

    LOK_ASSERT(LOOLProtocol::getTokenInteger(tokens, "foo", foo));
    LOK_ASSERT_EQUAL(42, foo);

    LOK_ASSERT(LOOLProtocol::getTokenString(tokens, "bar", bar));
    LOK_ASSERT_EQUAL(std::string("hello-sailor"), bar);

    LOK_ASSERT(LOOLProtocol::getTokenKeyword(tokens, "mumble", map, mumble));
    LOK_ASSERT_EQUAL(2, mumble);

    LOK_ASSERT(LOOLProtocol::getTokenIntegerFromMessage(message, "foo", foo));
    LOK_ASSERT_EQUAL(42, foo);

    LOK_ASSERT(LOOLProtocol::getTokenStringFromMessage(message, "bar", bar));
    LOK_ASSERT_EQUAL(std::string("hello-sailor"), bar);

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trimmed("A").size());
    LOK_ASSERT_EQUAL(std::string("A"), Util::trimmed("A"));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trimmed(" X").size());
    LOK_ASSERT_EQUAL(std::string("X"), Util::trimmed(" X"));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trimmed("Y ").size());
    LOK_ASSERT_EQUAL(std::string("Y"), Util::trimmed("Y "));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trimmed(" Z ").size());
    LOK_ASSERT_EQUAL(std::string("Z"), Util::trimmed(" Z "));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), Util::trimmed(" ").size());
    LOK_ASSERT_EQUAL(std::string(""), Util::trimmed(" "));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), Util::trimmed("   ").size());
    LOK_ASSERT_EQUAL(std::string(""), Util::trimmed("   "));

    std::string s;

    s = "A";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trim(s).size());
    s = "A";
    LOK_ASSERT_EQUAL(std::string("A"), Util::trim(s));

    s = " X";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trim(s).size());
    s = " X";
    LOK_ASSERT_EQUAL(std::string("X"), Util::trim(s));

    s = "Y ";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trim(s).size());
    s = "Y ";
    LOK_ASSERT_EQUAL(std::string("Y"), Util::trim(s));

    s = " Z ";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trim(s).size());
    s = " Z ";
    LOK_ASSERT_EQUAL(std::string("Z"), Util::trim(s));

    s = " ";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), Util::trim(s).size());
    s = " ";
    LOK_ASSERT_EQUAL(std::string(""), Util::trim(s));

    s = "   ";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), Util::trim(s).size());
    s = "   ";
    LOK_ASSERT_EQUAL(std::string(""), Util::trim(s));

    // Integer lists.
    std::vector<int> ints;

    ints = LOOLProtocol::tokenizeInts(std::string("-1"));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), ints.size());
    LOK_ASSERT_EQUAL(-1, ints[0]);

    ints = LOOLProtocol::tokenizeInts(std::string("1,2,3,4"));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(4), ints.size());
    LOK_ASSERT_EQUAL(1, ints[0]);
    LOK_ASSERT_EQUAL(2, ints[1]);
    LOK_ASSERT_EQUAL(3, ints[2]);
    LOK_ASSERT_EQUAL(4, ints[3]);

    ints = LOOLProtocol::tokenizeInts("");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), ints.size());

    ints = LOOLProtocol::tokenizeInts(std::string(",,,"));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), ints.size());
}

void WhiteBoxTests::testSplitting()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring(nullptr, 5, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring(nullptr, -1, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring("abc", 0, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring("abc", -1, '\n'));
    LOK_ASSERT_EQUAL(std::string("ab"), Util::getDelimitedInitialSubstring("abc", 2, '\n'));

    std::string first;
    std::string second;

    std::tie(first, second) = Util::split(std::string(""), '.', true);
    std::tie(first, second) = Util::split(std::string(""), '.', false);

    std::tie(first, second) = Util::splitLast(std::string(""), '.', true);
    std::tie(first, second) = Util::splitLast(std::string(""), '.', false);

    // Split first, remove delim.
    std::tie(first, second) = Util::split(std::string("a"), '.', true);
    LOK_ASSERT_EQUAL(std::string("a"), first);
    LOK_ASSERT_EQUAL(std::string(""), second);

    // Split first, keep delim.
    std::tie(first, second) = Util::split(std::string("a"), '.', false);
    LOK_ASSERT_EQUAL(std::string("a"), first);
    LOK_ASSERT_EQUAL(std::string(""), second);

    // Split first, remove delim.
    std::tie(first, second) = Util::splitLast(std::string("a"), '.', true);
    LOK_ASSERT_EQUAL(std::string("a"), first);
    LOK_ASSERT_EQUAL(std::string(""), second);

    // Split first, keep delim.
    std::tie(first, second) = Util::splitLast(std::string("a"), '.', false);
    LOK_ASSERT_EQUAL(std::string("a"), first);
    LOK_ASSERT_EQUAL(std::string(""), second);


    // Split first, remove delim.
    std::tie(first, second) = Util::split(std::string("a."), '.', true);
    LOK_ASSERT_EQUAL(std::string("a"), first);
    LOK_ASSERT_EQUAL(std::string(""), second);

    // Split first, keep delim.
    std::tie(first, second) = Util::split(std::string("a."), '.', false);
    LOK_ASSERT_EQUAL(std::string("a"), first);
    LOK_ASSERT_EQUAL(std::string("."), second);

    // Split first, remove delim.
    std::tie(first, second) = Util::splitLast(std::string("a."), '.', true);
    LOK_ASSERT_EQUAL(std::string("a"), first);
    LOK_ASSERT_EQUAL(std::string(""), second);

    // Split first, keep delim.
    std::tie(first, second) = Util::splitLast(std::string("a."), '.', false);
    LOK_ASSERT_EQUAL(std::string("a"), first);
    LOK_ASSERT_EQUAL(std::string("."), second);


    // Split first, remove delim.
    std::tie(first, second) = Util::split(std::string("aa.bb"), '.', true);
    LOK_ASSERT_EQUAL(std::string("aa"), first);
    LOK_ASSERT_EQUAL(std::string("bb"), second);

    // Split first, keep delim.
    std::tie(first, second) = Util::split(std::string("aa.bb"), '.', false);
    LOK_ASSERT_EQUAL(std::string("aa"), first);
    LOK_ASSERT_EQUAL(std::string(".bb"), second);

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(5), Util::getLastDelimiterPosition("aa.bb.cc", 8, '.'));

    // Split last, remove delim.
    std::tie(first, second) = Util::splitLast(std::string("aa.bb.cc"), '.', true);
    LOK_ASSERT_EQUAL(std::string("aa.bb"), first);
    LOK_ASSERT_EQUAL(std::string("cc"), second);

    // Split last, keep delim.
    std::tie(first, second) = Util::splitLast(std::string("aa.bb.cc"), '.', false);
    LOK_ASSERT_EQUAL(std::string("aa.bb"), first);
    LOK_ASSERT_EQUAL(std::string(".cc"), second);

    // Split last, remove delim.
    std::tie(first, second) = Util::splitLast(std::string("/owncloud/index.php/apps/richdocuments/wopi/files/13_ocgdpzbkm39u"), '/', true);
    LOK_ASSERT_EQUAL(std::string("/owncloud/index.php/apps/richdocuments/wopi/files"), first);
    LOK_ASSERT_EQUAL(std::string("13_ocgdpzbkm39u"), second);

    // Split last, keep delim.
    std::tie(first, second) = Util::splitLast(std::string("/owncloud/index.php/apps/richdocuments/wopi/files/13_ocgdpzbkm39u"), '/', false);
    LOK_ASSERT_EQUAL(std::string("/owncloud/index.php/apps/richdocuments/wopi/files"), first);
    LOK_ASSERT_EQUAL(std::string("/13_ocgdpzbkm39u"), second);

    std::string third;
    std::string fourth;

    std::tie(first, second, third, fourth) = Util::splitUrl("filename");
    LOK_ASSERT_EQUAL(std::string(""), first);
    LOK_ASSERT_EQUAL(std::string("filename"), second);
    LOK_ASSERT_EQUAL(std::string(""), third);
    LOK_ASSERT_EQUAL(std::string(""), fourth);

    std::tie(first, second, third, fourth) = Util::splitUrl("filename.ext");
    LOK_ASSERT_EQUAL(std::string(""), first);
    LOK_ASSERT_EQUAL(std::string("filename"), second);
    LOK_ASSERT_EQUAL(std::string(".ext"), third);
    LOK_ASSERT_EQUAL(std::string(""), fourth);

    std::tie(first, second, third, fourth) = Util::splitUrl("/path/to/filename");
    LOK_ASSERT_EQUAL(std::string("/path/to/"), first);
    LOK_ASSERT_EQUAL(std::string("filename"), second);
    LOK_ASSERT_EQUAL(std::string(""), third);
    LOK_ASSERT_EQUAL(std::string(""), fourth);

    std::tie(first, second, third, fourth) = Util::splitUrl("http://domain.com/path/filename");
    LOK_ASSERT_EQUAL(std::string("http://domain.com/path/"), first);
    LOK_ASSERT_EQUAL(std::string("filename"), second);
    LOK_ASSERT_EQUAL(std::string(""), third);
    LOK_ASSERT_EQUAL(std::string(""), fourth);

    std::tie(first, second, third, fourth) = Util::splitUrl("http://domain.com/path/filename.ext");
    LOK_ASSERT_EQUAL(std::string("http://domain.com/path/"), first);
    LOK_ASSERT_EQUAL(std::string("filename"), second);
    LOK_ASSERT_EQUAL(std::string(".ext"), third);
    LOK_ASSERT_EQUAL(std::string(""), fourth);

    std::tie(first, second, third, fourth) = Util::splitUrl("http://domain.com/path/filename.ext?params=3&command=5");
    LOK_ASSERT_EQUAL(std::string("http://domain.com/path/"), first);
    LOK_ASSERT_EQUAL(std::string("filename"), second);
    LOK_ASSERT_EQUAL(std::string(".ext"), third);
    LOK_ASSERT_EQUAL(std::string("?params=3&command=5"), fourth);
}

void WhiteBoxTests::testMessage()
{
    // try to force an isolated page alloc, likely to have
    // an invalid, electrified fence page after it.
    size_t sz = 4096*128;
    char *big = static_cast<char *>(malloc(sz));
    const char msg[] = "bogus-forward";
    char *dest = big + sz - (sizeof(msg) - 1);
    memcpy(dest, msg, sizeof (msg) - 1);
    Message overrun(dest, sizeof (msg) - 1, Message::Dir::Out);
    free(big);
}

void WhiteBoxTests::testPathPrefixTrimming()
{
    constexpr std::string_view testname = __func__;

    // These helpers are used by the logging macros.
    // See Log.hpp for details.

#ifdef IOS

    LOK_ASSERT_EQUAL(std::size_t(23), skipPathToFilename("./path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(std::size_t(21), skipPathToFilename("path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(std::size_t(22), skipPathToFilename("/path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(std::size_t(24), skipPathToFilename("../path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(std::size_t(0), skipPathToFilename(""));
    LOK_ASSERT_EQUAL(std::size_t(0), skipPathToFilename("/"));
    LOK_ASSERT_EQUAL(std::size_t(0), skipPathToFilename("."));

    LOK_ASSERT_EQUAL(std::string("filename.cpp"),
                     std::string(LOG_FILE_NAME("./path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL(std::string("filename.cpp"),
                     std::string(LOG_FILE_NAME("path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL(std::string("filename.cpp"),
                     std::string(LOG_FILE_NAME("/path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME("")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME("/")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME(".")));

#else

    LOK_ASSERT_EQUAL(std::size_t(2), skipPathPrefix("./path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(std::size_t(0), skipPathPrefix("path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(std::size_t(1), skipPathPrefix("/path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(std::size_t(3), skipPathPrefix("../path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(std::size_t(0), skipPathPrefix(""));
    LOK_ASSERT_EQUAL(std::size_t(1), skipPathPrefix("/"));
    LOK_ASSERT_EQUAL(std::size_t(1), skipPathPrefix("."));

    LOK_ASSERT_EQUAL(std::string("path/to/a/looooooong/filename.cpp"),
                     std::string(LOG_FILE_NAME("./path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL(std::string("path/to/a/looooooong/filename.cpp"),
                     std::string(LOG_FILE_NAME("path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL(std::string("path/to/a/looooooong/filename.cpp"),
                     std::string(LOG_FILE_NAME("/path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL(std::string("path/to/a/looooooong/filename.cpp"),
                     std::string(LOG_FILE_NAME("../path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME("")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME("/")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME(".")));

#endif
}

void WhiteBoxTests::testMessageAbbreviation()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring(nullptr, 5, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring(nullptr, -1, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring("abc", 0, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring("abc", -1, '\n'));
    LOK_ASSERT_EQUAL(std::string("ab"), Util::getDelimitedInitialSubstring("abc", 2, '\n'));

    // The end arg of getAbbreviatedMessage is the length of the first argument, not
    // the point at which it should be abbreviated. Abbreviation appends ... to the
    // result
    LOK_ASSERT_EQUAL(std::string(), LOOLProtocol::getAbbreviatedMessage(nullptr, 5));
    LOK_ASSERT_EQUAL(std::string(), LOOLProtocol::getAbbreviatedMessage(nullptr, -1));
    LOK_ASSERT_EQUAL(std::string(), LOOLProtocol::getAbbreviatedMessage("abc", 0));
    LOK_ASSERT_EQUAL(std::string(), LOOLProtocol::getAbbreviatedMessage("abc", -1));
    LOK_ASSERT_EQUAL(std::string("ab"), LOOLProtocol::getAbbreviatedMessage("abc", 2));

    std::string s;
    std::string abbr;

    s = "abcdefg";
    LOK_ASSERT_EQUAL(s, LOOLProtocol::getAbbreviatedMessage(s));

    s = "1234567890123\n45678901234567890123456789012345678901234567890123";
    abbr = "1234567890123...";
    LOK_ASSERT_EQUAL(abbr, LOOLProtocol::getAbbreviatedMessage(s.data(), s.size()));
    LOK_ASSERT_EQUAL(abbr, LOOLProtocol::getAbbreviatedMessage(s));

    std::string long_utf8_str_a(LOOLProtocol::maxNonAbbreviatedMsgLen - 3, 'a');
    LOK_ASSERT_EQUAL(long_utf8_str_a + std::string("mü..."),
                     LOOLProtocol::getAbbreviatedMessage(long_utf8_str_a + "müsli"));

    // don't allow the ü sequence to be broken
    std::string long_utf8_str_b(LOOLProtocol::maxNonAbbreviatedMsgLen - 2, 'a');
    LOK_ASSERT_EQUAL(long_utf8_str_b + std::string("mü..."),
                     LOOLProtocol::getAbbreviatedMessage(long_utf8_str_b + "müsli"));
}

void WhiteBoxTests::testReplace()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(std::string("zesz one zwo flee"), Util::replace("test one two flee", "t", "z"));
    LOK_ASSERT_EQUAL(std::string("testt one two flee"), Util::replace("test one two flee", "tes", "test"));
    LOK_ASSERT_EQUAL(std::string("testest one two flee"), Util::replace("test one two flee", "tes", "testes"));
    LOK_ASSERT_EQUAL(std::string("tete one two flee"), Util::replace("tettet one two flee", "tet", "te"));
    LOK_ASSERT_EQUAL(std::string("t one two flee"), Util::replace("test one two flee", "tes", ""));
    LOK_ASSERT_EQUAL(std::string("test one two flee"), Util::replace("test one two flee", "", "X"));
}

void WhiteBoxTests::testReplaceChar()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR("zesz one zwo flee", Util::replace("test one two flee", 't', 'z'));
    LOK_ASSERT_EQUAL_STR("test one two flee", Util::replace("test one two flee", ' ', ' '));
}

void WhiteBoxTests::testReplaceCharInPlace()
{
    constexpr std::string_view testname = __func__;

    // Can't compile, because the argument is a temporary.
    // LOK_ASSERT_EQUAL_STR("zesz one zwo flee", Util::replaceInPlace("test one two flee", 't', 'z'));
    std::string s = "test one two flee";
    LOK_ASSERT_EQUAL_STR("zesz one zwo flee", Util::replaceInPlace(s, 't', 'z'));
    LOK_ASSERT_EQUAL_STR("zesz one zwo flee", Util::replaceInPlace(s, ' ', ' '));
}

void WhiteBoxTests::testReplaceAllOf()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(std::string("humvee"), Util::replaceAllOf("humans","san", "eve"));
    LOK_ASSERT_EQUAL(std::string("simple.odt"), Util::replaceAllOf("s#&-le.odt", "#&-", "imp"));
}

void WhiteBoxTests::testRegexListMatcher()
{
    constexpr std::string_view testname = __func__;

    RegexUtil::RegexListMatcher matcher;

    matcher.allow("localhost");
    LOK_ASSERT(matcher.match("localhost"));
    LOK_ASSERT(!matcher.match(""));
    LOK_ASSERT(!matcher.match("localhost2"));
    LOK_ASSERT(!matcher.match("xlocalhost"));
    LOK_ASSERT(!matcher.match("192.168.1.1"));

    matcher.deny("localhost");
    LOK_ASSERT(!matcher.match("localhost"));

    matcher.allow("www[0-9].*");
    LOK_ASSERT(matcher.match("www1example"));

    matcher.allow("192\\.168\\..*\\..*");
    LOK_ASSERT(matcher.match("192.168.1.1"));
    LOK_ASSERT(matcher.match("192.168.159.1"));
    LOK_ASSERT(matcher.match("192.168.1.134"));
    LOK_ASSERT(!matcher.match("192.169.1.1"));
    LOK_ASSERT(matcher.match("192.168.."));

    matcher.deny("192\\.168\\.1\\..*");
    LOK_ASSERT(!matcher.match("192.168.1.1"));

    matcher.allow("staging\\.collaboracloudsuite\\.com.*");
    matcher.deny(".*collaboracloudsuite.*");
    LOK_ASSERT(!matcher.match("staging.collaboracloudsuite"));
    LOK_ASSERT(!matcher.match("web.collaboracloudsuite"));
    LOK_ASSERT(!matcher.match("staging.collaboracloudsuite.com"));

    matcher.allow("10\\.10\\.[0-9]{1,3}\\.[0-9]{1,3}");
    matcher.deny("10\\.10\\.10\\.10");
    LOK_ASSERT(matcher.match("10.10.001.001"));
    LOK_ASSERT(!matcher.match("10.10.10.10"));
    LOK_ASSERT(matcher.match("10.10.250.254"));
}

void WhiteBoxTests::testRegexListMatcher_Init()
{
    constexpr std::string_view testname = __func__;

    RegexUtil::RegexListMatcher matcher;
    matcher.allow("localhost");
    matcher.allow("192\\..*");
    matcher.deny("192\\.168\\..*");

    LOK_ASSERT(matcher.match("localhost"));
    LOK_ASSERT(!matcher.match(""));
    LOK_ASSERT(!matcher.match("localhost2"));
    LOK_ASSERT(!matcher.match("xlocalhost"));
    LOK_ASSERT(!matcher.match("192.168.1.1"));
    LOK_ASSERT(matcher.match("192.172.10.122"));

    matcher.deny("localhost");
    LOK_ASSERT(!matcher.match("localhost"));

    matcher.allow("www[0-9].*");
    LOK_ASSERT(matcher.match("www1example"));

    matcher.allow("192\\.168\\..*\\..*");
    LOK_ASSERT(!matcher.match("192.168.1.1"));
    LOK_ASSERT(!matcher.match("192.168.159.1"));
    LOK_ASSERT(!matcher.match("192.168.1.134"));
    LOK_ASSERT(matcher.match("192.169.1.1"));
    LOK_ASSERT(!matcher.match("192.168.."));

    matcher.clear();

    matcher.allow("192\\.168\\..*\\..*");
    LOK_ASSERT(matcher.match("192.168.1.1"));
    LOK_ASSERT(matcher.match("192.168.159.1"));
    LOK_ASSERT(matcher.match("192.168.1.134"));
    LOK_ASSERT(!matcher.match("192.169.1.1"));
    LOK_ASSERT(matcher.match("192.168.."));
}

void WhiteBoxTests::testTileDesc()
{
    constexpr std::string_view testname = __func__;

    // simulate a previous overflow
    errno = ERANGE;
    TileDesc desc = TileDesc::parse(
        "tile nviewid=0 part=5 width=256 height=256 tileposx=0 tileposy=12288 tilewidth=3072 tileheight=3072 oldwid=0 wid=0 ver=33");
    (void)desc; // exception in parse if we have problems.
    TileCombined combined = TileCombined::parse(
        "tilecombine nviewid=0 part=5 width=256 height=256 tileposx=0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504 tileposy=0,0,0,0,0,0,0,0,3072,3072,3072,3072,3072,3072,3072,3072,6144,6144,6144,6144,6144,6144,6144,6144,9216,9216,9216,9216,9216,9216,9216,9216,12288,12288,12288,12288,12288,12288,12288,12288,15360,15360,15360,15360,15360,15360,15360,15360,18432,18432,18432,18432,18432,18432,18432,18432 oldwid=2,3,4,5,6,7,8,8,9,10,11,12,13,14,15,16,17,18,19,20,21,0,0,0,24,25,26,27,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 tilewidth=3072 tileheight=3072");
    (void)combined; // exception in parse if we have problems.

    // Test parsing removing un-used pieces
    std::string base = "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 ";
    struct {
        std::string inp;
        std::string outp;
    } tests[] = {
        { "imgsize=0,0 tilewidth=3840 tileheight=3840 ver=-1,-1",
          "tilewidth=3840 tileheight=3840 ver=-1,-1" },
        { "imgsize=1,0 tilewidth=3840 tileheight=3840 ver=-1,-1",
          "imgsize=1,0 tilewidth=3840 tileheight=3840 ver=-1,-1" },
        { "wid=0,0 tilewidth=3840 tileheight=3840 ver=-1,-1",
          "tilewidth=3840 tileheight=3840 ver=-1,-1" },
        { "tilewidth=3840 tileheight=3840 ver=-1,-1 wid=0,1",
          "tilewidth=3840 tileheight=3840 ver=-1,-1 wid=0,1" },
        { "oldwid=0,0 tilewidth=3840 tileheight=3840 ver=-1,-1",
          "tilewidth=3840 tileheight=3840 ver=-1,-1" },
        { "tilewidth=3840 tileheight=3840 ver=-1,-1 oldwid=0,1",
          "tilewidth=3840 tileheight=3840 ver=-1,-1 oldwid=0,1" },
    };
    for (auto &s : tests)
    {
        combined = TileCombined::parse(base + s.inp);
        LOK_ASSERT_EQUAL(combined.serialize("tilecombine"), base + s.outp);
    }
}

void WhiteBoxTests::testTileData()
{
    constexpr std::string_view testname = __func__;

    TileData data(42, "Zfoo", 4);

    // replace keyframe
    data.appendBlob(43, "Zfoo", 4);
    LOK_ASSERT_EQUAL(size_t(3), data.size());

    // append a delta
    data.appendBlob(44, "Dbaa", 4);
    LOK_ASSERT_EQUAL(size_t(6), data.size());

    LOK_ASSERT_EQUAL(data.isPng(), false);

    // validation.
    LOK_ASSERT_EQUAL(data.isValid(), true);
    data.invalidate();
    LOK_ASSERT_EQUAL(data.isValid(), false);

    std::vector<char> out;
    LOK_ASSERT_EQUAL(data.appendChangesSince(out, 128), false);
    LOK_ASSERT_EQUAL(out.size(), size_t(0));

    LOK_ASSERT_EQUAL(data.appendChangesSince(out, 42), true);
    LOK_ASSERT_EQUAL(std::string("foobaa"), Util::toString(out));

    out.clear();
    LOK_ASSERT_EQUAL(data.appendChangesSince(out, 43), true);
    LOK_ASSERT_EQUAL(std::string("baa"), Util::toString(out));

    // append another delta
    data.appendBlob(47, "Dbaz", 4);
    LOK_ASSERT_EQUAL(data.size(), size_t(9));

    out.clear();
    LOK_ASSERT_EQUAL(data.appendChangesSince(out, 1), true);
    LOK_ASSERT_EQUAL(std::string("foobaabaz"), Util::toString(out));

    out.clear();
    LOK_ASSERT_EQUAL(data.appendChangesSince(out, 43), true);
    LOK_ASSERT_EQUAL(std::string("baabaz"), Util::toString(out));

    // append an empty delta
    data.appendBlob(52, "D", 1);
    LOK_ASSERT_EQUAL(data.size(), size_t(9));
    LOK_ASSERT_EQUAL(data._wids.size(), size_t(4));
    LOK_ASSERT_EQUAL(data._wids.back(), unsigned(52));

    // the next empty delta should pack into the last one
    data.appendBlob(54, "D", 1);
    LOK_ASSERT_EQUAL(data.size(), size_t(9));
    LOK_ASSERT_EQUAL(data._wids.size(), size_t(4));
    LOK_ASSERT_EQUAL(data._wids.back(), unsigned(54));
}

void WhiteBoxTests::testRectanglesIntersect()
{
    constexpr std::string_view testname = __func__;

    // these intersect
    LOK_ASSERT(TileDesc::rectanglesIntersect(1000, 1000, 2000, 1000,
                                                 2000, 1000, 2000, 1000));
    LOK_ASSERT(TileDesc::rectanglesIntersect(2000, 1000, 2000, 1000,
                                                 1000, 1000, 2000, 1000));

    LOK_ASSERT(TileDesc::rectanglesIntersect(1000, 1000, 2000, 1000,
                                                 3000, 2000, 1000, 1000));
    LOK_ASSERT(TileDesc::rectanglesIntersect(3000, 2000, 1000, 1000,
                                                 1000, 1000, 2000, 1000));

    // these don't
    LOK_ASSERT(!TileDesc::rectanglesIntersect(1000, 1000, 2000, 1000,
                                                  2000, 3000, 2000, 1000));
    LOK_ASSERT(!TileDesc::rectanglesIntersect(2000, 3000, 2000, 1000,
                                                  1000, 1000, 2000, 1000));

    LOK_ASSERT(!TileDesc::rectanglesIntersect(1000, 1000, 2000, 1000,
                                                  2000, 3000, 1000, 1000));
    LOK_ASSERT(!TileDesc::rectanglesIntersect(2000, 3000, 1000, 1000,
                                                  1000, 1000, 2000, 1000));
}

void WhiteBoxTests::testJson()
{
    constexpr std::string_view testname = __func__;

    static const char* testString =
         "{\"BaseFileName\":\"SomeFile.pdf\",\"DisableCopy\":true,\"DisableExport\":true,\"DisableInactiveMessages\":true,\"DisablePrint\":true,\"EnableOwnerTermination\":true,\"HideExportOption\":true,\"HidePrintOption\":true,\"OwnerId\":\"id@owner.com\",\"PostMessageOrigin\":\"*\",\"Size\":193551,\"UserCanWrite\":true,\"UserFriendlyName\":\"Owning user\",\"UserId\":\"user@user.com\",\"WatermarkText\":null}";

    Poco::JSON::Object::Ptr object;
    LOK_ASSERT(JsonUtil::parseJSON(testString, object));

    std::size_t iValue = 0;
    JsonUtil::findJSONValue(object, "Size", iValue);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(193551), iValue);

    bool bValue = false;
    JsonUtil::findJSONValue(object, "DisableCopy", bValue);
    LOK_ASSERT_EQUAL(true, bValue);

    std::string sValue;
    JsonUtil::findJSONValue(object, "BaseFileName", sValue);
    LOK_ASSERT_EQUAL(std::string("SomeFile.pdf"), sValue);

    // Don't accept inexact key names.
    sValue.clear();
    JsonUtil::findJSONValue(object, "basefilename", sValue);
    LOK_ASSERT_EQUAL(std::string(), sValue);

    JsonUtil::findJSONValue(object, "invalid", sValue);
    LOK_ASSERT_EQUAL(std::string(), sValue);

    JsonUtil::findJSONValue(object, "UserId", sValue);
    LOK_ASSERT_EQUAL(std::string("user@user.com"), sValue);
}

void WhiteBoxTests::testAnonymization()
{
    constexpr std::string_view testname = __func__;

    static const std::string name = "some name with space";
    static const std::string filename = "filename.ext";
    static const std::string filenameTestx = "testx (6).odt";
    static const std::string path = "/path/to/filename.ext";
    static const std::string plainUrl
        = "http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/"
          "736_ocgdpzbkm39u?access_token=Hn0zttjbwkvGWb5BHbDa5ArgTykJAyBl&access_token_ttl=0&"
          "permission=edit";
    static const std::string fileUrl = "http://localhost/owncloud/index.php/apps/richdocuments/"
                                       "wopi/files/736_ocgdpzbkm39u/"
                                       "secret.odt?access_token=Hn0zttjbwkvGWb5BHbDa5ArgTykJAyBl&"
                                       "access_token_ttl=0&permission=edit";

    std::uint64_t anonymizationSalt = 1111111111182589933;
    Anonymizer::initialize(true, anonymizationSalt);

    LOK_ASSERT_EQUAL(std::string("#0#5e45aef91248a8aa#"), Anonymizer::anonymizeUrl(name));
    LOK_ASSERT_EQUAL(std::string("#1#8f8d95bd2a202d00#.odt"),
                     Anonymizer::anonymizeUrl(filenameTestx));
    LOK_ASSERT_EQUAL(std::string("/path/to/#2#5c872b2d82ecc8a0#.ext"),
                     Anonymizer::anonymizeUrl(path));
    LOK_ASSERT_EQUAL(
        std::string("http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/"
                    "#3#22c6f0caad277666#?access_token=Hn0zttjbwkvGWb5BHbDa5ArgTykJAyBl&access_"
                    "token_ttl=0&permission=edit"),
        Anonymizer::anonymizeUrl(plainUrl));
    LOK_ASSERT_EQUAL(
        std::string("http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/"
                    "736_ocgdpzbkm39u/"
                    "#4#294f0dfb18f6a80b#.odt?access_token=Hn0zttjbwkvGWb5BHbDa5ArgTykJAyBl&access_"
                    "token_ttl=0&permission=edit"),
        Anonymizer::anonymizeUrl(fileUrl));

    anonymizationSalt = 0;
    Anonymizer::initialize(true, anonymizationSalt);

    LOK_ASSERT_EQUAL(std::string("#0#42027f9b6df09510#"), Anonymizer::anonymizeUrl(name));
    Anonymizer::mapAnonymized(name, name);
    LOK_ASSERT_EQUAL(name, Anonymizer::anonymizeUrl(name));

    LOK_ASSERT_EQUAL(std::string("#1#366ab9ebe19ea09e#.ext"), Anonymizer::anonymizeUrl(filename));
    Anonymizer::mapAnonymized("filename",
                              "filename"); // Identity map of the filename without extension.
    LOK_ASSERT_EQUAL(filename, Anonymizer::anonymizeUrl(filename));

    LOK_ASSERT_EQUAL(std::string("#2#eac31ed57854de54#.odt"),
                     Anonymizer::anonymizeUrl(filenameTestx));
    Anonymizer::mapAnonymized("testx (6)",
                              "testx (6)"); // Identity map of the filename without extension.
    LOK_ASSERT_EQUAL(filenameTestx, Anonymizer::anonymizeUrl(filenameTestx));

    LOK_ASSERT_EQUAL(path, Anonymizer::anonymizeUrl(path));

    const std::string urlAnonymized =
        Util::replace(plainUrl, "736_ocgdpzbkm39u", "#3#f64fbe55134cd5f0#");
    LOK_ASSERT_EQUAL(urlAnonymized, Anonymizer::anonymizeUrl(plainUrl));
    Anonymizer::mapAnonymized("736_ocgdpzbkm39u", "736_ocgdpzbkm39u");
    LOK_ASSERT_EQUAL(plainUrl, Anonymizer::anonymizeUrl(plainUrl));

    const std::string urlAnonymized2 = Util::replace(fileUrl, "secret", "#4#dcac6c9cae1b3b95#");
    LOK_ASSERT_EQUAL(urlAnonymized2, Anonymizer::anonymizeUrl(fileUrl));
    Anonymizer::mapAnonymized("secret", "736_ocgdpzbkm39u");
    const std::string urlAnonymized3 = Util::replace(fileUrl, "secret", "736_ocgdpzbkm39u");
    LOK_ASSERT_EQUAL(urlAnonymized3, Anonymizer::anonymizeUrl(fileUrl));
}

void WhiteBoxTests::testIso8601Time()
{
    constexpr std::string_view testname = __func__;

    std::ostringstream oss;

    std::chrono::system_clock::time_point t(std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds(1567444337874777375)));
    LOK_ASSERT_EQUAL(std::string("2019-09-02T17:12:17.874777Z"),
                         Util::getIso8601FracformatTime(t));

    t = std::chrono::system_clock::time_point(std::chrono::system_clock::duration::zero());
    LOK_ASSERT_EQUAL(std::string("1970-01-01T00:00:00.000000Z"),
                         Util::getIso8601FracformatTime(t));

    t = Util::iso8601ToTimestamp("1970-01-01T00:00:00.000000Z", "LastModifiedTime");
    oss << t.time_since_epoch().count();
    LOK_ASSERT_EQUAL(std::string("0"), oss.str());
    LOK_ASSERT_EQUAL(std::string("1970-01-01T00:00:00.000000Z"),
                         Util::time_point_to_iso8601(t));

    oss.str(std::string());
    t = Util::iso8601ToTimestamp("2019-09-02T17:12:17.874777Z", "LastModifiedTime");
    oss << t.time_since_epoch().count();
    if (std::is_same<std::chrono::system_clock::period, std::nano>::value)
        LOK_ASSERT_EQUAL(std::string("1567444337874777000"), oss.str());
    else
        LOK_ASSERT_EQUAL(std::string("1567444337874777"), oss.str());
    LOK_ASSERT_EQUAL(std::string("2019-09-02T17:12:17.874777Z"),
                         Util::time_point_to_iso8601(t));

    oss.str(std::string());
    t = Util::iso8601ToTimestamp("2019-10-24T14:31:28.063730Z", "LastModifiedTime");
    oss << t.time_since_epoch().count();
    if (std::is_same<std::chrono::system_clock::period, std::nano>::value)
        LOK_ASSERT_EQUAL(std::string("1571927488063730000"), oss.str());
    else
        LOK_ASSERT_EQUAL(std::string("1571927488063730"), oss.str());
    LOK_ASSERT_EQUAL(std::string("2019-10-24T14:31:28.063730Z"),
                         Util::time_point_to_iso8601(t));

    t = Util::iso8601ToTimestamp("2020-02-20T20:02:20.100000Z", "LastModifiedTime");
    LOK_ASSERT_EQUAL(std::string("2020-02-20T20:02:20.100000Z"),
                         Util::time_point_to_iso8601(t));

    t = std::chrono::system_clock::time_point();
    LOK_ASSERT_EQUAL(std::string("Thu, 01 Jan 1970 00:00:00"), Util::getHttpTime(t));

    t = std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds(1569592993495336798)));
    LOK_ASSERT_EQUAL(std::string("Fri, 27 Sep 2019 14:03:13"), Util::getHttpTime(t));

    t = Util::iso8601ToTimestamp("2020-09-22T21:45:12.583000Z", "LastModifiedTime");
    LOK_ASSERT_EQUAL(std::string("2020-09-22T21:45:12.583000Z"),
                         Util::time_point_to_iso8601(t));

    t = Util::iso8601ToTimestamp("2020-09-22T21:45:12.583Z", "LastModifiedTime");
    LOK_ASSERT_EQUAL(std::string("2020-09-22T21:45:12.583000Z"),
                         Util::time_point_to_iso8601(t));

    for (int i = 0; i < 100; ++i)
    {
        t = std::chrono::system_clock::now();
        const uint64_t t_in_micros = (t.time_since_epoch().count() / 1000) * 1000;

        const std::string s = Util::getIso8601FracformatTime(t);
        t = Util::iso8601ToTimestamp(s, "LastModifiedTime");

        std::string t_in_micros_str = std::to_string(t_in_micros);
        std::string time_since_epoch_str = std::to_string(t.time_since_epoch().count());
        if (!std::is_same<std::chrono::system_clock::period, std::nano>::value)
        {
            // If the system clock has nanoseconds precision, the last 3 digits
            // of these strings may not match. For example,
            // 1567444337874777000
            // 1567444337874777123
            t_in_micros_str.resize(t_in_micros_str.length() - 3);
            time_since_epoch_str.resize(time_since_epoch_str.length() - 3);
        }

        LOK_ASSERT_EQUAL(t_in_micros_str, time_since_epoch_str);

        // Allow a small delay to get a different timestamp on next iteration.
        sleep(0);
    }
}

void WhiteBoxTests::testClockAsString()
{
    // This test depends on locale and timezone.
    // It is only here to test changes to these functions,
    // but the tests can't be run elsewhere.
    // I left them here to avoid recreating them when needed.
#if 0
    constexpr std::string_view testname = __func__;

    const auto steady_tp = std::chrono::steady_clock::time_point(
        std::chrono::steady_clock::duration(std::chrono::nanoseconds(295708311764285)));
    LOK_ASSERT_EQUAL(std::string("Sat Feb 12 18:58.889 2022"),
                     Util::getSteadyClockAsString(steady_tp));

    const auto sys_tp = std::chrono::system_clock::time_point(
        std::chrono::system_clock::duration(std::chrono::nanoseconds(1644764467739980124)));
    LOK_ASSERT_EQUAL(std::string("Sat Feb 12 18:58.889 2022"),
                     Util::getSystemClockAsString(sys_tp));
#endif
}

void WhiteBoxTests::testStat()
{
    constexpr std::string_view testname = __func__;

    FileUtil::Stat invalid("/missing/file/path");
    LOK_ASSERT(!invalid.good());
    LOK_ASSERT(invalid.bad());
    LOK_ASSERT(!invalid.exists());

    const std::string tmpFile = FileUtil::getSysTempDirectoryPath() + "/test_stat";
    std::ofstream ofs(tmpFile);
    FileUtil::Stat st(tmpFile);
    LOK_ASSERT(st.good());
    LOK_ASSERT(!st.bad());
    LOK_ASSERT(st.exists());
    LOK_ASSERT(!st.isDirectory());
    LOK_ASSERT(st.isFile());
    LOK_ASSERT(!st.isLink());

    // Modified-time tests.
    // Some test might fail when the system has a different resolution for file timestamps
    // and time_point. Specifically, if the filesystem has microsecond precision but time_point
    // has lower resolution (milliseconds or seconds, f.e.), modifiedTimepoint() will not match
    // modifiedTimeUs(), and the checks will fail.
    // So far, microseconds seem to be the lower common denominator. At least on Android and
    // iOS that's the precision of time_point (as of late 2020), but Linux servers have
    // nanosecond precision.

    LOK_ASSERT(std::chrono::time_point_cast<std::chrono::microseconds>(st.modifiedTimepoint())
                   .time_since_epoch()
                   .count()
               == static_cast<long>(st.modifiedTimeUs()));
    LOK_ASSERT(std::chrono::time_point_cast<std::chrono::milliseconds>(st.modifiedTimepoint())
                   .time_since_epoch()
                   .count()
               == static_cast<long>(st.modifiedTimeMs()));
    LOK_ASSERT(std::chrono::time_point_cast<std::chrono::seconds>(st.modifiedTimepoint())
                   .time_since_epoch()
                   .count()
               == static_cast<long>(st.modifiedTimeMs() / 1000));
    LOK_ASSERT(st.modifiedTime().tv_sec == static_cast<long>(st.modifiedTimeMs() / 1000));
    LOK_ASSERT(st.modifiedTime().tv_nsec / 1000
               == static_cast<long>(st.modifiedTimeUs())
                      - (st.modifiedTime().tv_sec * 1000 * 1000));

    ofs.close();
    FileUtil::removeFile(tmpFile);
}

void WhiteBoxTests::testStringCompare()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT(Util::iequal("abcd", "abcd"));
    LOK_ASSERT(Util::iequal("aBcd", "abCd"));
    LOK_ASSERT(Util::iequal("", ""));

    LOK_ASSERT(!Util::iequal("abcd", "abc"));
    LOK_ASSERT(!Util::iequal("abc", "abcd"));
    LOK_ASSERT(!Util::iequal("abc", "abcd"));

    LOK_ASSERT(!Util::iequal("abc", 3, "abcd", 4));
}

void WhiteBoxTests::testSafeAtoi()
{
    constexpr std::string_view testname = __func__;

    {
        std::string s("7");
        LOK_ASSERT_EQUAL(7, Util::safe_atoi(s.data(), s.size()));
    }
    {
        std::string s("+7");
        LOK_ASSERT_EQUAL(7, Util::safe_atoi(s.data(), s.size()));
    }
    {
        std::string s("-7");
        LOK_ASSERT_EQUAL(-7, Util::safe_atoi(s.data(), s.size()));
    }
    {
        std::string s("42");
        LOK_ASSERT_EQUAL(42, Util::safe_atoi(s.data(), s.size()));
    }
    {
        std::string s("42");
        LOK_ASSERT_EQUAL(4, Util::safe_atoi(s.data(), 1));
    }
    {
        std::string s("  42");
        LOK_ASSERT_EQUAL(42, Util::safe_atoi(s.data(), s.size()));
    }
    {
        std::string s("42xy");
        LOK_ASSERT_EQUAL(42, Util::safe_atoi(s.data(), s.size()));
    }
    {
        // Make sure signed integer overflow doesn't happen.
        std::string s("9999999990");
        // Good:       2147483647
        // Bad:        1410065398
        LOK_ASSERT_EQUAL(std::numeric_limits<int>::max(), Util::safe_atoi(s.data(), s.size()));
    }
    {
        std::string s("123");
        s[1] = '\0';
        LOK_ASSERT_EQUAL(1, Util::safe_atoi(s.data(), s.size()));
    }
    {
        LOK_ASSERT_EQUAL(0, Util::safe_atoi(nullptr, 0));
    }
}

void WhiteBoxTests::testJsonUtilEscapeJSONValue()
{
    constexpr std::string_view testname = __func__;

    constexpr std::string_view in = "domain\\username";
    const std::string expected = "domain\\\\username";
    LOK_ASSERT_EQUAL(JsonUtil::escapeJSONValue(in), expected);
}

STATE_ENUM(TestState, First, Second, Last);
void WhiteBoxTests::testStateEnum()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR("TestState::First", name(TestState::First));
    LOK_ASSERT_EQUAL_STR("TestState::Second", name(TestState::Second));
    LOK_ASSERT_EQUAL_STR("TestState::Last", name(TestState::Last));

    LOK_ASSERT_EQUAL_STR("First", nameShort(TestState::First));
    LOK_ASSERT_EQUAL_STR("Second", nameShort(TestState::Second));
    LOK_ASSERT_EQUAL_STR("Last", nameShort(TestState::Last));

    TestState e = TestState::First;

    e = TestState::First;
    LOK_ASSERT_EQUAL_STR("TestState::First", name(e));
    e = TestState::Second;
    LOK_ASSERT_EQUAL_STR("TestState::Second", name(e));
    e = TestState::Last;
    LOK_ASSERT_EQUAL_STR("TestState::Last", name(e));

    e = TestState::First;
    LOK_ASSERT_EQUAL_STR("First", nameShort(e));
    e = TestState::Second;
    LOK_ASSERT_EQUAL_STR("Second", nameShort(e));
    e = TestState::Last;
    LOK_ASSERT_EQUAL_STR("Last", nameShort(e));

    std::ostringstream oss;

    e = TestState::First;
    oss << e;
    LOK_ASSERT_EQUAL_STR("TestState::First", oss.str());
    oss.str("");

    e = TestState::Second;
    oss << e;
    LOK_ASSERT_EQUAL_STR("TestState::Second", oss.str());
    oss.str("");

    e = TestState::Last;
    oss << e;
    LOK_ASSERT_EQUAL_STR("TestState::Last", oss.str());
    oss.str("");
}

void WhiteBoxTests::testFindInVector()
{
    constexpr std::string_view testname = __func__;
    std::string s("fooBarfooBaz");
    std::vector<char> v(s.begin(), s.end());

    // Normal case, we find the first "foo".
    std::size_t ret = Util::findInVector(v, "foo");
    std::size_t expected = 0;
    LOK_ASSERT_EQUAL(expected, ret);

    // Offset, so we find the second "foo".
    ret = Util::findInVector(v, "foo", 1);
    expected = 6;
    LOK_ASSERT_EQUAL(expected, ret);

    // Negative testing.
    ret = Util::findInVector(v, "blah");
    expected = std::string::npos;
    LOK_ASSERT_EQUAL(expected, ret);
}

#if 0
size_t WhiteBoxTests::waitForThreads(size_t count)
{
    auto start = std::chrono::steady_clock::now();
    while (Util::getCurrentThreadCount() != count)
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count() >= 250)
        {
            std::cerr << "Failed to get correct thread count " << count <<
                " instead we have " << Util::getCurrentThreadCount() << "\n";
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return Util::getCurrentThreadCount();
}
#endif

void WhiteBoxTests::testThreadPool()
{
    constexpr std::string_view testname = __func__;
    //    const size_t existingUnrelatedThreads = Util::getCurrentThreadCount();
    // coverity[tainted_data_argument : FALSE] - we trust this variable in tests
    setenv("MAX_CONCURRENCY","8",1);
    // coverity[tainted_argument] : don't warn that getenv("MAX_CONCURRENCY") is tainted
    ThreadPool pool;
    LOK_ASSERT_EQUAL(int(8), pool._maxConcurrency);
    LOK_ASSERT_EQUAL(size_t(7), pool._threads.size());
//    LOK_ASSERT_EQUAL(size_t(7 + existingUnrelatedThreads), waitForThreads(8 + existingUnrelatedThreads));

    pool.stop();
    LOK_ASSERT_EQUAL(size_t(0), pool._threads.size());
//    LOK_ASSERT_EQUAL(size_t(existingUnrelatedThreads), waitForThreads(existingUnrelatedThreads));

    pool.start();
    LOK_ASSERT_EQUAL(size_t(7), pool._threads.size());
//    LOK_ASSERT_EQUAL(size_t(7 + existingUnrelatedThreads), waitForThreads(8 + existingUnrelatedThreads));
}

CPPUNIT_TEST_SUITE_REGISTRATION(WhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
