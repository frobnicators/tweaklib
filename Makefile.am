ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = -Wall -Wcast-qual -fvisibility=hidden

lib_LTLIBRARIES = libtweak.la
noinst_PROGRAMS = example

nobase_include_HEADERS = tweak/tweak.h tweak/version.h

libtweak_la_LDFLAGS = -version-info 0:0:0
libtweak_la_SOURCES = src/tweak.c src/http.c src/http.h src/server.c src/server.h src/log.c src/log.h

example_LDADD = libtweak.la
example_SOURCES = src/example.c
