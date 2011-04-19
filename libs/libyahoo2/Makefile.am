SUBDIRS = windows

lib_LTLIBRARIES = libyahoo2.la

EXTRA_DIST = sample_client.c autoresponder.c

noinst_HEADERS = md5.h yahoo_debug.h yahoo_util.h sha1.h yahoo_fn.h

AM_CFLAGS = $(GLIB_CFLAGS) -g -Wall
LIBS = @LIBS@ $(GLIB_LIBS)

libyahoo2_la_SOURCES = libyahoo2.c crypt.c md5.c sha1.c \
	yahoo_httplib.c yahoo_util.c yahoo_list.c yahoo_fn.c

libyahoo2_la_LDFLAGS = -module -version-info $(YAHOO2_VERSION)

pkginclude_HEADERS = yahoo2.h yahoo2_callbacks.h yahoo2_types.h \
	yahoo_httplib.h yahoo_list.h yahoo_fn.h

if SAMPLE_CLIENT
noinst_PROGRAMS = yahoo autoresponder

yahoo_SOURCES = sample_client.c
yahoo_LDADD = libyahoo2.la
yahoo_LDFLAGS = -lssl

autoresponder_SOURCES = autoresponder.c
autoresponder_LDADD = libyahoo2.la
autoresponder_LDFLAGS = -lssl

endif
