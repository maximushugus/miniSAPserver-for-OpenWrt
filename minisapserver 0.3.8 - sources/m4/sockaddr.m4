# sockaddr.m4
# Copyright © 2003-2006 Rémi Denis-Courmont
# <rdenis (at) simphalempin (dot) com>.
# This file (sockaddr.m4) is free software; unlimited permission to
# copy and/or distribute it , with or without modifications, as long
# as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

dnl SHOULD check <sys/socket.h>, <winsock2.h> before that
AC_DEFUN([RDC_STRUCT_SOCKADDR_LEN],
[AC_LANG_ASSERT(C)
AH_TEMPLATE(HAVE_SA_LEN, [Define to 1 if `struct sockaddr' has a `sa_len' member.])
AC_CACHE_CHECK([if struct sockaddr has a sa_len member],
rdc_cv_struct_sockaddr_len,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
[#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <sys/socket.h>]], [[struct sockaddr addr; addr.sa_len = 0;]])],
rdc_cv_struct_sockaddr_len=yes,
rdc_cv_struct_sockaddr_len=no)])
AS_IF([test $rdc_cv_struct_sockaddr_len = yes],
 [AC_DEFINE(HAVE_SA_LEN)])
]) 

