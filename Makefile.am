ETAGS=ctags -e

if ENABLE_MOBILEAPP

if ENABLE_GTKAPP
SUBDIRS = gtk browser
else
if ENABLE_WASM
SUBDIRS = wasm browser
else !ENABLE_WASM
SUBDIRS = browser
endif !ENABLE_WASM
endif

else

SUBDIRS = . browser

if ENABLE_TESTS
SUBDIRS += test
endif

if ENABLE_CYPRESS
SUBDIRS += cypress_test
endif

export ENABLE_DEBUG

bin_PROGRAMS = \
	loolforkit \
	loolforkitns \
	loolmount \
        loolstress \
	loolconvert loolconfig

if ENABLE_LIBFUZZER
else
bin_PROGRAMS += loolwsd
if ENABLE_DEBUG
bin_PROGRAMS += loolwsd-inproc
endif
endif

dist_bin_SCRIPTS = loolwsd-systemplate-setup

man_MANS = man/loolwsd.1 \
           man/loolforkit.1 \
           man/loolconvert.1 \
           man/loolconfig.1 \
           man/loolstress.1 \
           man/loolwsd-systemplate-setup.1 \
           man/loolmount.1


dist_doc_DATA = wsd/README \
                wsd/README.vars \
                wsd/protocol.txt \
                wsd/reference.md \
                wsd/metrics.txt

loolwsddatadir = @LOOLWSD_DATADIR@

loolwsddata_DATA = discovery.xml \
                   favicon.ico

loolwsdconfigdir = @LOOLWSD_CONFIGDIR@

loolwsdconfig_DATA = loolwsd.xml \
		     loolkitconfig.xcu

apache2configdir = ${sysconfdir}/apache2/conf-available

apache2config_DATA = etc/apache2/loolwsd.conf

nginxconfigdir = ${sysconfdir}/nginx/snippets

nginxconfig_DATA = etc/nginx/loolwsd.conf

ACLOCAL_AMFLAGS = -I m4

# quick and easy for now.
include_paths = -I${top_srcdir}/common -I${top_srcdir}/net -I${top_srcdir}/wsd -I${top_srcdir}/kit ${ZLIB_CFLAGS} ${ZSTD_CFLAGS} ${PNG_CFLAGS}
if ENABLE_SSL
include_paths += ${OPENSSL_CFLAGS}
endif

AM_CPPFLAGS = -pthread \
	      ${include_paths}

if !ENABLE_DEBUG
AM_CPPFLAGS += -DNDEBUG
endif

AM_LDFLAGS = -Wl,-E -lpam $(ZLIB_LIBS) $(ZSTD_LIBS) ${PNG_LIBS}

# Clang's linker doesn't like -pthread.
if !HAVE_CLANG
AM_LDFLAGS += -pthread
endif

if ENABLE_SSL
AM_LDFLAGS += ${OPENSSL_LIBS}
else
loolwsd_LDADD = ${OPENSSL_LIBS}
loolconfig_LDADD = ${OPENSSL_LIBS}
endif

AM_ETAGSFLAGS = --c++-kinds=+p --fields=+iaS --extra=+q -R --totals=yes --exclude=browser/node_modules --exclude=browser/dist *
AM_CTAGSFLAGS = $(AM_ETAGSFLAGS)

shared_sources = common/FileUtil.cpp \
                 common/FileUtil-unix.cpp \
                 common/JailUtil.cpp \
                 common/Log.cpp \
                 common/Protocol.cpp \
                 common/StringVector.cpp \
                 common/Session.cpp \
                 common/Seccomp.cpp \
                 common/MobileApp.cpp \
                 common/RegexUtil.cpp \
                 common/SigUtil-server.cpp \
                 common/SpookyV2.cpp \
                 common/TraceEvent.cpp \
                 common/Unit.cpp \
                 common/Unit-server.cpp \
                 common/Uri.cpp \
                 common/Util.cpp \
                 common/Util-server.cpp \
                 common/Util-unix.cpp \
                 common/ConfigUtil.cpp \
                 common/Authorization.cpp \
                 common/CommandControl.cpp \
                 common/Simd.cpp \
                 common/LoolMount.cpp \
                 kit/KitQueue.cpp \
                 kit/LogUI.cpp \
                 net/DelaySocket.cpp \
                 net/HttpRequest.cpp \
                 net/HttpHelper.cpp \
                 net/NetUtil.cpp \
                 net/Socket.cpp \
                 wsd/Exceptions.cpp
if ENABLE_SSL
shared_sources += net/Ssl.cpp
endif

loolwsd_sources = common/Crypto.cpp \
                  wsd/Admin.cpp \
                  wsd/AdminModel.cpp \
                  wsd/Auth.cpp \
                  wsd/CacheUtil.cpp \
                  wsd/LOOLWSD.cpp \
                  wsd/ClientRequestDispatcher.cpp \
                  wsd/ClientSession.cpp \
                  wsd/DocumentBroker.cpp \
                  wsd/FileServer.cpp \
                  wsd/FileServerUtil.cpp \
                  wsd/HostUtil.cpp \
                  wsd/ProofKey.cpp \
                  wsd/ProxyProtocol.cpp \
                  wsd/ProxyRequestHandler.cpp \
                  wsd/QuarantineUtil.cpp \
                  wsd/RemoteConfig.cpp \
                  wsd/RequestDetails.cpp \
                  wsd/RequestVettingStation.cpp \
                  wsd/ServerAuditUtil.cpp \
                  wsd/SpecialBrokers.cpp \
                  wsd/Storage.cpp \
                  wsd/TileCache.cpp \
                  wsd/wopi/CheckFileInfo.cpp \
                  wsd/wopi/StorageConnectionManager.cpp \
                  wsd/wopi/WopiProxy.cpp \
                  wsd/wopi/WopiStorage.cpp

loolwsd_SOURCES = $(loolwsd_sources) \
                  $(shared_sources) \
                  wsd/loolwsd-fork.cpp

EXTRA_loolwsd_DEPENDENCIES = browser/node_modules

noinst_PROGRAMS = clientnb \
                  connect \
                  lokitclient \
                  loolmap \
                  loolbench \
                  loolsocketdump

if ENABLE_LIBFUZZER
noinst_PROGRAMS += \
		   admin_fuzzer \
		   clientsession_fuzzer \
		   httpresponse_fuzzer \
		   httpecho_fuzzer
endif

connect_SOURCES = tools/Connect.cpp \
                  common/DummyTraceEventEmitter.cpp \
                  common/Log.cpp \
                  common/Protocol.cpp \
                  common/StringVector.cpp \
                  common/Util.cpp \
                  common/Util-server.cpp \
                  common/Util-unix.cpp

lokitclient_SOURCES = common/Log.cpp \
                      common/DummyTraceEventEmitter.cpp \
                      tools/KitClient.cpp \
                      common/Protocol.cpp \
                      common/StringVector.cpp \
                      common/TraceEvent.cpp \
                      common/Util.cpp \
                      common/Util-server.cpp \
                      common/Util-unix.cpp

noinst_LIBRARIES = libsimd.a
libsimd_a_SOURCES = kit/DeltaSimd.c
libsimd_a_CFLAGS = @SIMD_CFLAGS@

loolforkit_sources = kit/ChildSession.cpp \
                     kit/ForKit.cpp \
                     kit/Kit.cpp \
                     kit/KitWebSocket.cpp

loolforkit_SOURCES = $(loolforkit_sources) \
                     $(shared_sources) \
                     kit/forkit-main.cpp

loolforkit_LDADD = libsimd.a

loolforkitns_SOURCES = $(loolforkit_sources) \
                       $(shared_sources) \
                       kit/forkit-main.cpp

loolforkitns_LDADD = libsimd.a

if ENABLE_DEBUG
loolwsd_inproc_SOURCES = $(loolwsd_sources) \
                         $(shared_sources) \
                         $(loolforkit_sources) \
                         wsd/loolwsd-inproc.cpp

loolwsd_inproc_LDADD = ${loolwsd_LDADD} \
                       ${loolforkit_LDADD}
endif

if ENABLE_LIBFUZZER
loolforkit_SOURCES += \
		       common/DummyTraceEventEmitter.cpp
loolforkitns_SOURCES += \
		       common/DummyTraceEventEmitter.cpp

common_fuzzer_sources = \
		       fuzzer/Common.cpp \
		       common/DummyTraceEventEmitter.cpp \
		       $(loolwsd_sources) \
		       wsd/loolwsd-inproc.cpp \
		       $(loolforkit_sources) \
		       $(shared_sources)

admin_fuzzer_CPPFLAGS = \
			$(AM_CPPFLAGS)
admin_fuzzer_SOURCES = \
		       $(common_fuzzer_sources) \
		       fuzzer/Admin.cpp
admin_fuzzer_LDFLAGS = -fsanitize=fuzzer $(AM_LDFLAGS)
admin_fuzzer_LDADD = libsimd.a

clientsession_fuzzer_CPPFLAGS = \
				$(AM_CPPFLAGS)
clientsession_fuzzer_SOURCES = \
			       $(common_fuzzer_sources) \
			       fuzzer/ClientSession.cpp
clientsession_fuzzer_LDFLAGS = -fsanitize=fuzzer $(AM_LDFLAGS)
clientsession_fuzzer_LDADD = libsimd.a

httpresponse_fuzzer_CPPFLAGS = \
				$(AM_CPPFLAGS)
httpresponse_fuzzer_SOURCES = \
			       $(common_fuzzer_sources) \
			       fuzzer/HttpResponse.cpp
httpresponse_fuzzer_LDFLAGS = -fsanitize=fuzzer $(AM_LDFLAGS)
httpresponse_fuzzer_LDADD = libsimd.a

httpecho_fuzzer_CPPFLAGS = \
				$(AM_CPPFLAGS) \
        -I${top_srcdir}/test
httpecho_fuzzer_SOURCES = \
			       $(common_fuzzer_sources) \
			       fuzzer/HttpEcho.cpp
httpecho_fuzzer_LDFLAGS = -fsanitize=fuzzer $(AM_LDFLAGS)
httpecho_fuzzer_LDADD = libsimd.a

endif # ENABLE_LIBFUZZER

clientnb_SOURCES = net/clientnb.cpp \
                   common/DummyTraceEventEmitter.cpp \
                   common/Log.cpp \
                   common/StringVector.cpp \
                   common/Util.cpp \
                   common/Util-server.cpp \
                   common/Util-unix.cpp

loolmount_SOURCES = tools/mount.cpp \
                    common/LoolMount.cpp

loolmap_SOURCES = tools/map.cpp

loolbench_SOURCES = tools/Benchmark.cpp \
                    common/DummyTraceEventEmitter.cpp \
                    common/Log.cpp \
                    common/StringVector.cpp \
                    common/Util.cpp \
                    common/Util-server.cpp \
                    common/Util-unix.cpp \
                    common/Simd.cpp

loolbench_LDADD = libsimd.a

loolconvert_SOURCES = tools/Tool.cpp

loolstress_TDOC_CPPFLAGS = -DTDOC=\"$(abs_top_srcdir)/test/data\"
loolstress_CPPFLAGS = ${loolstress_TDOC_CPPFLAGS} ${include_paths} \
			$(AM_CPPFLAGS)
loolstress_SOURCES = tools/Stress.cpp \
                     common/DummyTraceEventEmitter.cpp \
                     $(shared_sources)

loolconfig_SOURCES = tools/Config.cpp \
                    common/ConfigUtil.cpp \
                    common/Crypto.cpp \
                    common/DummyTraceEventEmitter.cpp \
                    common/Log.cpp \
                    common/StringVector.cpp \
                    common/Util-server.cpp \
                    common/Util-unix.cpp \
                    common/Util.cpp

loolsocketdump_SOURCES = tools/WebSocketDump.cpp \
			 common/DummyTraceEventEmitter.cpp \
			 $(shared_sources)

wsd_headers = wsd/Admin.hpp \
              wsd/AdminModel.hpp \
              wsd/Auth.hpp \
              wsd/CacheUtil.hpp \
              wsd/LOOLWSD.hpp \
              wsd/ClientRequestDispatcher.hpp \
              wsd/ClientSession.hpp \
              wsd/ContentSecurityPolicy.hpp \
              wsd/DocumentBroker.hpp \
              wsd/Exceptions.hpp \
              wsd/FileServer.hpp \
              wsd/HostUtil.hpp \
              wsd/PresetsInstall.hpp \
              wsd/Process.hpp \
              wsd/ProofKey.hpp \
              wsd/ProxyProtocol.hpp \
              wsd/ProxyRequestHandler.hpp \
              wsd/QuarantineUtil.hpp \
              wsd/RemoteConfig.hpp \
              wsd/RequestDetails.hpp \
              wsd/RequestVettingStation.hpp \
              wsd/SenderQueue.hpp \
              wsd/ServerAuditUtil.hpp \
              wsd/ServerURL.hpp \
              wsd/SpecialBrokers.hpp \
              wsd/Storage.hpp \
              wsd/TileCache.hpp \
              wsd/TileDesc.hpp \
              wsd/TraceFile.hpp \
              wsd/UserMessages.hpp \
              wsd/wopi/CheckFileInfo.hpp \
              wsd/wopi/StorageConnectionManager.hpp \
              wsd/wopi/WopiProxy.hpp \
              wsd/wopi/WopiStorage.hpp

shared_headers = common/Anonymizer.hpp \
                 common/Common.hpp \
                 common/CharacterConverter.hpp \
                 common/Clipboard.hpp \
                 common/Crypto.hpp \
                 common/Globals.hpp \
                 common/HexUtil.hpp \
                 common/JsonUtil.hpp \
                 common/FileUtil.hpp \
                 common/JailUtil.hpp \
                 common/LangUtil.hpp \
                 common/Log.hpp \
                 common/Protocol.hpp \
                 common/RegexUtil.hpp \
                 common/StateEnum.hpp \
                 common/StaticLogHelper.hpp \
                 common/StringVector.hpp \
                 common/Seccomp.hpp \
                 common/Session.hpp \
                 common/Unit.hpp \
                 common/Uri.hpp \
                 common/Util.hpp \
                 common/ConfigUtil.hpp \
                 common/Authorization.hpp \
                 common/Message.hpp \
                 common/MobileApp.hpp \
                 common/Png.hpp \
                 common/TraceEvent.hpp \
                 common/Rectangle.hpp \
                 common/RenderTiles.hpp \
                 common/SigUtil.hpp \
                 common/SigHandlerTrap.hpp \
                 common/security.h \
                 common/SpookyV2.h \
                 common/CommandControl.hpp \
                 common/Simd.hpp \
                 common/ThreadPool.hpp \
                 common/Watchdog.hpp \
                 kit/KitQueue.hpp \
                 net/AsyncDNS.hpp \
                 net/Buffer.hpp \
                 net/DelaySocket.hpp \
                 net/FakeSocket.hpp \
                 net/HttpRequest.hpp \
                 net/HttpServer.hpp \
                 net/HttpHelper.hpp \
                 net/NetUtil.hpp \
                 net/ServerSocket.hpp \
                 net/Socket.hpp \
                 net/Uri.hpp \
                 net/WebSocketHandler.hpp \
                 tools/Replay.hpp \
		 wasm/base64.hpp
if ENABLE_SSL
shared_headers += net/Ssl.hpp \
                  net/SslSocket.hpp
endif

kit_headers = kit/ChildSession.hpp \
              kit/Delta.hpp \
              kit/DeltaSimd.h \
              kit/DummyLibreOfficeKit.hpp \
              kit/Kit.hpp \
              kit/KitHelper.hpp \
              kit/KitWebSocket.hpp \
              kit/SetupKitEnvironment.hpp \
	      kit/SlideCompressor.hpp \
              kit/StateRecorder.hpp \
              kit/Watermark.hpp

noinst_HEADERS = $(wsd_headers) $(shared_headers) $(kit_headers) \
                 tools/LOOLWebSocket.hpp \
                 test/HttpTestServer.hpp \
                 test/KitPidHelpers.hpp \
                 test/UnitHTTP.hpp \
                 test/UnitTimeoutBase.hpp \
                 test/UnitWSDClient.hpp \
                 test/WOPIUploadConflictCommon.hpp \
                 test/WebSocketSession.hpp \
                 test/WopiTestServer.hpp \
                 test/helpers.hpp \
                 test/lokassert.hpp \
                 test/test.hpp \
                 test/testlog.hpp

GIT_BRANCH := $(shell git symbolic-ref --short HEAD)
GIT_HASH := $(shell git log -1 --format=%h)

dist-hook:
	git log -1 --format=%h > $(distdir)/dist_git_hash 2> /dev/null || rm $(distdir)/dist_git_hash
	mkdir -p $(distdir)/bundled/include/LibreOfficeKit/
	cp @LOKIT_PATH@/LibreOfficeKit/LibreOfficeKit.h \
	   @LOKIT_PATH@/LibreOfficeKit/LibreOfficeKit.hxx \
	   @LOKIT_PATH@/LibreOfficeKit/LibreOfficeKitEnums.h \
	   @LOKIT_PATH@/LibreOfficeKit/LibreOfficeKitInit.h \
	   @LOKIT_PATH@/LibreOfficeKit/LibreOfficeKitTypes.h \
	   $(distdir)/bundled/include/LibreOfficeKit/

EXTRA_DIST = discovery.xml \
             favicon.ico \
             loolkitconfig.xcu \
             loolwsd.spec \
             loolwsd.xml.in \
             loolwsd.service \
             debian/compat \
             debian/copyright \
             debian/loolwsd.preinst \
             debian/loolwsd.service \
             debian/rules \
             debian/source/format \
             debian/changelog \
             debian/control \
             debian/loolwsd.install \
             debian/loolwsd-deprecated.install \
             debian/loolwsd-deprecated.postinst \
             debian/loolwsd.postinst \
             debian/loolwsd.postrm \
             etc/key.pem \
             etc/cert.pem \
             etc/ca-chain.cert.pem \
             etc/apache2/loolwsd.conf \
             etc/nginx/loolwsd.conf \
             scripts/unocommands.py \
             scripts/refresh-git-hooks \
             README.md \
             $(man_MANS)

if ENABLE_LIBFUZZER
CLEANUP_COMMAND=true
else
# Use loolwsd to cleanup jails, if any. If it fails, we may have a broken/old loolwsd binary, remove it to rebuild.
# A CI box may switch branches without cleaning up the binaries, if loolwsd from a broken branch is used here
# it will fail all subsequent builds, until it's rebuilt from the new branch. So removing loolwsd after failing is needed.
CLEANUP_COMMAND=if test -s ./loolwsd -a -x ./loolwsd; then echo "Cleaning up..." && ./loolwsd --disable-lool-user-checking --cleanup --o:logging.level=trace || rm -f ./loolwsd; fi
endif

CLEANUP_COVERAGE=rm -rf ${abs_top_srcdir}/gcov; find . -iname "*.gcda" -delete

if HAVE_LO_PATH

SYSTEM_STAMP = @SYSTEMPLATE_PATH@/system_stamp
RUN_GDB = $(if $(GDB_FRONTEND),$(GDB_FRONTEND),gdb --tui --args)

if ENABLE_LIBFUZZER
CLEANUP_DEPS=
else
CLEANUP_DEPS=loolwsd loolmount loolforkit loolforkitns
endif

# Build loolwsd and loolmount first, so we can cleanup before updating
# the systemplate directory, which we can't rm if it's mounted.
$(SYSTEM_STAMP): ${top_srcdir}/loolwsd-systemplate-setup $(CLEANUP_DEPS)
	$(CLEANUP_COMMAND)
	if test "z@SYSTEMPLATE_PATH@" != "z"; then \
		chmod u+w -R --silent "@SYSTEMPLATE_PATH@"; \
		(rm -rf "@SYSTEMPLATE_PATH@" || \
			echo "WARNING: failed to remove the systemplate") && \
		${top_srcdir}/loolwsd-systemplate-setup "@SYSTEMPLATE_PATH@" "@LO_PATH@" && \
		touch $@; \
	fi

@JAILS_PATH@:
	@$(CLEANUP_COMMAND)
	mkdir -p $@

@CACHE_PATH@:
	@$(CLEANUP_COMMAND)
	mkdir -p $@

presets-dir:
	@rm -rf $(abs_top_srcdir)/test/data/presets
	@cp -r $(abs_top_srcdir)/test/data/presets-default $(abs_top_srcdir)/test/data/presets

cleanup:
	$(CLEANUP_COMMAND)

# Always cleanup before removing the binaries.
# Note: this can break with -j option!
clean-am: cleanup clean-binPROGRAMS clean-generic clean-libtool clean-local clean-noinstPROGRAMS mostlyclean-am

clean-local:
	$(CLEANUP_COMMAND)
	if test "z@SYSTEMPLATE_PATH@" != "z"; then rm -rf "@SYSTEMPLATE_PATH@" || echo "WARNING: failed to remove the systemplate"; fi
	if test "z@JAILS_PATH@" != "z"; then rm -rf "@JAILS_PATH@" || echo "WARNING: failed to remove all jails cleanly"; fi
	rm -rf "${top_srcdir}/loleaflet"
	rm -rf loolconfig loolconvert loolforkit loolmap loolmount # kill old binaries
	rm -rf loolwsd loolwsd_fuzzer loolstress loolsocketdump
	$(CLEANUP_COVERAGE)

if ENABLE_DEBUG
# can write to /tmp/loolwsd.log
  OUTPUT_TO_FILE=true
else
# can't write to /var/log/loolwsd.log
  OUTPUT_TO_FILE=false
endif

QUIET_SAMPLES = $(QUIET_SAMPLES_$(V))
QUIET_SAMPLES_ = $(QUIET_SAMPLES_$(AM_DEFAULT_VERBOSITY))
QUIET_SAMPLES_0 = @echo "  SAMPLES " $@;

setup-wsd: all @JAILS_PATH@ @CACHE_PATH@ presets-dir
	@echo "Launching loolwsd"
	@fc-cache "@LO_PATH@"/share/fonts/truetype
	@mkdir -p $(abs_top_builddir)/test/samples
	$(QUIET_SAMPLES) $(abs_top_srcdir)/scripts/build-samples.py $(abs_top_srcdir)/test/samples $(abs_top_srcdir)/test/samples
	@echo

COMMON_PARAMS = \
	--o:sys_template_path="@SYSTEMPLATE_PATH@" \
	--o:child_root_path="@JAILS_PATH@" --o:storage.filesystem[@allow]=true \
	--o:ssl.cert_file_path="$(abs_top_srcdir)/etc/cert.pem" \
	--o:ssl.key_file_path="$(abs_top_srcdir)/etc/key.pem" \
	--o:ssl.ca_file_path="$(abs_top_srcdir)/etc/ca-chain.cert.pem" \
	--o:admin_console.username=admin --o:admin_console.password=admin

run: setup-wsd
	./loolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=${OUTPUT_TO_FILE} --o:logging.level=trace \
			--o:trace_event[@enable]=true

if ENABLE_DEBUG
run-one: setup-wsd
	./loolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=true --o:logging.level=trace \
			--singlekit

run-inproc: setup-wsd
	@echo "Launching loolwsd-inproc"
	./loolwsd-inproc $(COMMON_PARAMS) \
			--o:logging.file[@enable]=true --o:logging.level=trace

run-massif-inproc: setup-wsd
	@echo "Launching loolwsd under valgrind for single process"
	valgrind --tool=massif --num-callers=64 --trace-children=no -v \
		./loolwsd-inproc $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=trace

run-heaptrack-inproc: setup-wsd
	@echo "Launching loolwsd under heaptrack for single process"
	heaptrack \
		./loolwsd-inproc $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=trace
endif

sync-writer:
	browser-sync start --config browsersync-config.js --startPath "browser/96c23f663/debug.html?file_path=/$(abs_top_srcdir)/test/samples/writer-edit.fodt"

sync-calc:
	browser-sync start --config browsersync-config.js --startPath "browser/96c23f663/debug.html?file_path=/$(abs_top_srcdir)/test/samples/calc-edit.ods"

sync-impress:
	browser-sync start --config browsersync-config.js --startPath "browser/96c23f663/debug.html?file_path=/$(abs_top_srcdir)/test/samples/impress-edit.fodp"

run-trace: setup-wsd
	./loolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=trace \
			--o:trace[@enable]=true --o:trace.path=${builddir}/trace.txt.gz \
			--o:trace.outgoing.record=false

run-valgrind: setup-wsd
	@echo "Launching loolwsd under valgrind (but not forkit/loolkit, yet)"
	valgrind --tool=memcheck --trace-children=no -v --read-var-info=yes \
		./loolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=trace

run-gdb: setup-wsd
	@echo "Launching loolwsd under gdb"
	$(RUN_GDB) \
		./loolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=error

run-callgrind: setup-wsd
	@echo "Launching loolwsd under valgrind's callgrind"
	valgrind --tool=callgrind --simulate-cache=yes --dump-instr=yes --num-callers=50 --error-limit=no --trace-children=yes \
		./loolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=error

run-strace: setup-wsd
	@echo "Launching loolwsd under strace"
	strace -o strace.log -f -tt -s 256 \
		./loolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=false --o:logging.level=error

run-tcmalloc: setup-wsd
	@echo "Launching loolwsd under tcmalloc"
	if ! ldconfig -p|grep -q libtcmalloc.so.4; then \
		echo Install your distro provided tcmalloc packages. Fedora: gperftools-libs, Debian: libgoogle-perftools4; \
		exit 1; \
	fi
	@echo see https://gperftools.github.io/gperftools/heapprofile.html for Modifying Runtime Behavior via env variables
	$(ENABLE_NAMESPACE) LD_PRELOAD=libtcmalloc.so.4 ./loolwsd $(COMMON_PARAMS) \
			--o:logging.file[@enable]=${OUTPUT_TO_FILE} --o:logging.level=trace \
			--o:logging_ui_cmd.file[@enable]=${OUTPUT_TO_FILE} \
			--o:trace_event[@enable]=true

else

SYSTEM_STAMP =

endif

.PHONY: cleanup setup-wsd

if ENABLE_LIBFUZZER
ALL_LOCAL_DEPS=
else
ALL_LOCAL_DEPS=loolwsd
endif

all-local: $(ALL_LOCAL_DEPS) @JAILS_PATH@ $(SYSTEM_STAMP)
	@$(abs_top_srcdir)/scripts/refresh-git-hooks

# just run the build without any tests
build-nocheck: all-am

endif

define file_target
$(1): $(2)

endef

define file_targets
$(foreach file,$(1),$(call file_target,$(file),$(patsubst %.cmd,%.cpp,$(file))))
endef

CLANGXX_COMPILE_FLAGS=clang++ $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)

JSON_COMPILE_FLAGS=$(subst ",\",$(subst \,\\,$(CLANGXX_COMPILE_FLAGS) $(loolstress_TDOC_CPPFLAGS)))

JSON_COMPILE_SRC = $(patsubst %.cpp,%.cmd,$(wildcard **/*.cpp))

$(eval $(call file_targets,$(JSON_COMPILE_SRC)))

.cpp.cmd:
	@echo -n "{\"directory\":\"$(abs_builddir)\",\"command\":\"" > $@
	@echo -n $(JSON_COMPILE_FLAGS) >> $@
	@echo -n " -o $(patsubst %.cpp,%.o,$<) -c $<\",\"file\":\"$<\"}" >> $@

$(abs_srcdir)/compile_commands.json: $(JSON_COMPILE_SRC)
	@echo -n "[" > $@
	@for file in $(JSON_COMPILE_SRC) ; \
	do \
		cat $$file; \
		echo ","; \
		rm -f $$file; \
	done >> $@
	@echo -n "]" >> $@

compile_commands: $(abs_srcdir)/compile_commands.json

$(abs_srcdir)/.clangd: config.d/clangd.am
	@sed -e 's|@LOOL_DIR@|${abs_srcdir}|g' -e 's|@LOKIT_DIR@|${LOKIT_PATH}|g' $< > $@

dot_clangd: $(abs_srcdir)/.clangd

$(abs_srcdir)/.vscode/settings.json: config.d/vscode/settings.json.am
	mkdir -p $(abs_srcdir)/.vscode
	sed -e 's|@LOOL_DIR@|${abs_srcdir}|g' -e 's|@LOKIT_DIR@|${LOKIT_PATH}|g' $< > $@

$(abs_srcdir)/.vscode/online.code-workspace: config.d/vscode/online.code-workspace.am
	mkdir -p $(abs_srcdir)/.vscode
	sed -e 's|@LOOL_DIR@|${abs_srcdir}|g' -e 's|@LOKIT_DIR@|${LOKIT_PATH}|g' $< > $@

gen-clangd: $(abs_srcdir)/compile_commands.json $(abs_srcdir)/.clangd

gen-vscode: gen-clangd $(abs_srcdir)/.vscode/settings.json $(abs_srcdir)/.vscode/online.code-workspace

clean-clangd:
	cd $(abs_srcdir) && rm -f .clangd

clean-vscode: clean-clangd
	cd $(abs_srcdir) && rm -rf .vscode

browser/node_modules: browser/package.json browser/node_shrinkpack
	@cd browser && npm install

eslint: browser/node_modules
	@$(MAKE) -C browser eslint

prettier: browser/node_modules
	@$(MAKE) -C browser prettier

prettier-write: browser/node_modules
	@$(MAKE) -C browser prettier-write

stress_file=$(abs_top_srcdir)/test/data/hello-world.odt
trace_dir=$(abs_top_srcdir)/test/traces
stress:
	@cp $(abs_top_srcdir)/test/data/hello.odt $(stress_file)
	@echo "Watch at: https://localhost:9980/browser/12345/lool.html?file_path=$(stress_file)"
	./loolstress wss://localhost:9980 \
		$(stress_file) $(trace_dir)/writer-quick.txt \
		$(stress_file) $(trace_dir)/writer-add-bullet.txt \
		$(stress_file) $(trace_dir)/writer-mash-text-table.txt \
		$(stress_file) $(trace_dir)/writer-document-edit.txt \
		$(stress_file) $(trace_dir)/writer-mash-text-table.txt \
		$(stress_file) $(trace_dir)/writer-hello-shape.txt \
		$(stress_file) $(trace_dir)/writer-quick.txt

if ENABLE_CODE_COVERAGE
GEN_COVERAGE_COMMAND=mkdir -p ${abs_top_srcdir}/gcov && \
lcov --no-external --capture --rc 'lcov_excl_line=' --rc 'lcov_excl_br_line=LOG_|TST_|LOK_|WSD_|TRANSITION|assert' \
--compat libtool=on --directory ${abs_top_srcdir}/. --output-file ${abs_top_srcdir}/gcov/lool.coverage.test.info && \
genhtml --prefix ${abs_top_srcdir}/. --ignore-errors source ${abs_top_srcdir}/gcov/lool.coverage.test.info \
--legend --title "${GIT_BRANCH} @ ${GIT_HASH}" --output-directory=${abs_top_srcdir}/gcov/html && \
echo "Code-Coverage report generated in ${abs_top_srcdir}/gcov/html"
else
GEN_COVERAGE_COMMAND=true
endif

# typically the system nss won't work in a jail and fail to init which is fatal for
# exporting to pdf, which is something that gets tested
check-for-system-nss:
	@if test -e "@LO_PATH@/../config_host.mk"; then \
		if grep -q SYSTEM_NSS=TRUE "@LO_PATH@/../config_host.mk"; then \
			echo make check will fail unless core is configured using --without-system-nss; \
			exit 1; \
		fi \
	fi

check-recursive: eslint

check: check-for-system-nss prettier eslint presets-dir check-recursive
	$(GEN_COVERAGE_COMMAND)

coverage-report:
	$(GEN_COVERAGE_COMMAND)

coverage-clean:
	$(CLEANUP_COVERAGE)

# a) -D/usr/include, don't care about system headers
# b) -xProtocolHandlerInterface, this class poses a problem for pahole 1.29
# (https://github.com/acmel/dwarves/issues/38 might be relevant)
# c) Redirect stderr to /dev/null because there are a lot of dwarf parse
# warnings: 'template_parameter_pack__load_params: DW_TAG_template_value_parameter'
# possibly related to https://gcc.gnu.org/wiki/TemplateParmsDwarf
check-struct-packing:
	@echo Note: results only reliable with clang++ and CXXFLAGS=-fno-eliminate-unused-debug-types
	@if pahole -D/usr/include -x ProtocolHandlerInterface -P loolwsd 2> /dev/null | grep .; then \
		echo you can examine each candidate with pahole -S -R --class_name=CANDIDATE loolwsd; \
		exit 1; \
	fi
	@echo loolwsd struct packing is acceptably optimal

czech: all-local
	@echo "Running unit-tests only"
	$(MAKE) check -C test

