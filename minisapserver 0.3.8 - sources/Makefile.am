ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = 1.11 \
	-Wall \
	check-news \
	no-dist-gzip \
	dist-xz \
	std-options

bin_PROGRAMS = sapserver

AM_CPPFLAGS= -DSYSCONFDIR=\"@sysconfdir@\" $(INCICONV)
sapserver_SOURCES = broadcast.cpp broadcast.h \
			message.cpp message.h \
			parser.cpp parser.h \
			program.cpp program.h \
			sapserver.cpp sapserver.h
sapserver_LDADD=$(LIBICONV)

if CONFIG_SLP
sapserver_SOURCES += slp.cpp lslp.h
sapserver_LDADD += -lslp
endif

man1_MANS=sapserver.1
dist_sysconf_DATA=sap.cfg

EXTRA_DIST = $(man1_MANS) m4/getopt.m4 m4/sockaddr.m4 admin/config.rpath

