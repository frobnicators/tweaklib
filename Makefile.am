ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -Wall -Wcast-qual -fvisibility=hidden
BUILT_SOURCES = src/static.c

lib_LTLIBRARIES = libtweak.la
noinst_PROGRAMS = example pack

nobase_include_HEADERS = tweak/tweak.h tweak/version.h

libtweak_la_LIBADD = -lcrypto
libtweak_la_LDFLAGS = -version-info 0:0:0
libtweak_la_SOURCES = src/tweak.c src/http.c src/http.h src/server.c src/server.h src/log.c src/log.h src/static.c

example_LDADD = libtweak.la
example_SOURCES = src/example.c

pack_SOURCES = src/pack.c

src/static.c: pack static/index.html static/tweaklib.js static/style.css
	$(AM_V_GEN)./pack $^ > $@
