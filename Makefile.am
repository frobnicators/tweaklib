ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -Wall -Wcast-qual -fvisibility=hidden -I${top_srcdir}/src
BUILT_SOURCES = src/static.c

lib_LTLIBRARIES = libtweak.la
noinst_PROGRAMS = example pack

nobase_include_HEADERS = tweak/tweak.h tweak/version.h

libtweak_la_LIBADD = -lcrypto ${json_LIBS}
libtweak_la_LDFLAGS = -version-info 0:0:0
libtweak_la_CFLAGS = ${AM_CFLAGS} ${json_CFLAGS}
libtweak_la_SOURCES = \
	src/dt_int.c \
	src/http.c src/http.h \
	src/list.c src/list.h \
	src/log.c src/log.h \
	src/server.c src/server.h \
	src/static.c src/static.h \
	src/tweak.c \
	src/websocket.c src/websocket.h \
	src/worker.h

example_LDADD = libtweak.la
example_LDFLAGS = -pthread
example_SOURCES = src/example.c

pack_SOURCES = src/pack.c

src/static.c: pack static/index.html static/tweaklib.js static/style.css
	$(AM_V_GEN)./pack $^ > $@
