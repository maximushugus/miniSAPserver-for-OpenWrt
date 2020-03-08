dnl Autoconf settings for mini-SAP-server
dnl $Id: configure.ac 368 2012-03-14 15:19:40Z courmisch $

AC_COPYRIGHT([Copyright (C) the VideoLAN team 1998-2012])

AC_INIT(miniSAPserver,0.3.8,videolan@videolan.org)

AC_CONFIG_SRCDIR(configure.ac)
AC_CONFIG_AUX_DIR(admin)
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_HEADERS(config.h)

dnl ****************************
dnl        General checks
dnl ****************************
AC_PROG_CC
AC_PROG_CXX
AC_C_CONST
AC_HEADER_STDBOOL

AC_CHECK_HEADERS([sys/sockio.h])
AC_CHECK_FUNCS([clearenv])
RDC_REPLACE_FUNC_GETOPT_LONG


dnl ****************************
dnl        Features
dnl ****************************

AM_INIT_AUTOMAKE
AM_SILENT_RULES([yes])

dnl On the fly charset transcoding
AM_ICONV

AC_SEARCH_LIBS([socket], [socket])
AC_SEARCH_LIBS([inet_pton], [nsl])
AC_SEARCH_LIBS([nanosleep], [rt])

RDC_STRUCT_SOCKADDR_LEN
AC_CHECK_FUNCS([daemon])

dnl Enable/Disable slp
AS_IF([test "x${enable_slp}" == "x"],
	[enable_slp="no"])
AC_ARG_ENABLE(slp,
	[AC_HELP_STRING([--enable-slp],
			[enable SLP announcing (default disabled)])],
	[AS_IF([test "${enable_slp}" != "no"],
		[AC_CHECK_HEADERS(slp.h,,
			[AC_MSG_ERROR(["Unable to find SLP headers"])])
		 AC_DEFINE(CONFIG_SLP,,[SLP announcing support])
        ])
	])
AM_CONDITIONAL(CONFIG_SLP, [test "${enable_slp}" != "no"])

dnl Debug Mode
AC_ARG_ENABLE(debug,
	AC_HELP_STRING([--enable-debug],
			[enable debug mode (default enabled)]))

AS_IF([test "${enable_debug}" != "no"],[CFLAGS="${CFLAGS} -g"])


dnl ****************************
dnl       Output Files
dnl ****************************

AC_SUBST(CONFIG_SLP)
AC_SUBST(CFLAGS)

AC_OUTPUT(Makefile)
