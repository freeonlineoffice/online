bin_PROGRAMS = mobile

include_paths = -I${top_srcdir}/common -I${top_srcdir}/net -I${top_srcdir}/kit -I${top_srcdir}/wsd -I${top_srcdir} ${ZSTD_CFLAGS}

warning_flags = -Wall -Werror -Wno-parentheses -Wno-sign-compare -Wno-unused-variable
define_flags = -DMOBILEAPP=1

AM_CPPFLAGS = -pthread \
              -DTOPSRCDIR='"'$(realpath ${top_srcdir})'"' \
              -DGTKAPP \
              ${define_flags} \
              ${warning_flags} \
              ${include_paths} \
              ${WEBKIT_CFLAGS}

AM_LDFLAGS = -pthread ${WEBKIT_LIBS} -ldl $(ZSTD_LIBS) ${PNG_LIBS}

# Android's CMakefile list re-worked:

cmake_list= \
            ../common/Authorization.cpp \
            ../common/ConfigUtil.cpp \
            ../common/CommandControl.cpp \
            ../common/Crypto-stub.cpp \
            ../common/FileUtil.cpp \
            ../common/FileUtil-unix.cpp \
            ../common/Log.cpp \
            ../common/TraceEvent.cpp \
            ../common/Protocol.cpp \
            ../common/RegexUtil.cpp \
            ../common/Simd.cpp \
            ../common/StringVector.cpp \
            ../common/Session.cpp \
            ../common/SigUtil-mobile.cpp \
            ../common/SpookyV2.cpp \
            ../common/Unit.cpp \
            ../common/Unit-mobile.cpp \
            ../common/Util.cpp \
            ../common/Util-mobile.cpp \
            ../common/Util-unix.cpp \
            ../common/Uri.cpp \
            ../kit/ChildSession.cpp \
            ../kit/DeltaSimd.c \
            ../kit/Kit.cpp \
            ../kit/KitQueue.cpp \
            ../kit/KitWebSocket.cpp \
            ../net/FakeSocket.cpp \
            ../net/Socket.cpp \
            ../wsd/ClientSession.cpp \
            ../wsd/DocumentBroker.cpp \
            ../wsd/LOOLWSD.cpp \
            ../wsd/ClientRequestDispatcher.cpp \
            ../wsd/RequestDetails.cpp \
            ../wsd/RequestVettingStation.cpp \
            ../wsd/Storage.cpp \
            ../wsd/TileCache.cpp \
            ../wsd/loolwsd-fork.cpp

mobile_SOURCES = mobile.cpp $(cmake_list)
