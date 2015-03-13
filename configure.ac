#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.53)

AC_INIT
AC_CONFIG_SRCDIR([src/brute.c])
AM_INIT_AUTOMAKE([brute], [`cat VERSION`])
AM_CONFIG_HEADER(include/config.h)
AM_MAINTAINER_MODE

# Checks for programs.
AC_CANONICAL_HOST
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_SUBST(opt_options)

# Cheks for host_os 
case "$host_os" in
linux*)     	;;
*)          	AC_MSG_ERROR(${host_os} not supported)
	    	;;
esac

# Checks for host_cpu 
case ${host_cpu} in
	*586)
		opt_options="-O3 -DCONFIG_586 -fomit-frame-pointer -march=pentium"
		;;
	*686)
		opt_options="-O3 -DCONFIG_686 -fomit-frame-pointer -march=pentiumpro" 
		;;
        *x86_64)
                opt_options="-O3 -DCONFIG_X86_64 -fomit-frame-pointer -march=athlon64"
                ;;
        *hppa2.0)
                opt_options="-O3 -DCONFIG_PA -fomit-frame-pointer"
		;;
	*)
		AC_MSG_ERROR(${host_cpu} architecture not supported yet);;
esac

# Defines

AH_VERBATIM([_GNU_SOURCE],
[/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif])

# Checks for --enable-x
AC_MSG_CHECKING([if --enable-setuid option is specified])
AC_ARG_ENABLE(setuid,[  --enable-setuid         permit brute to run setuid root],
[
	AC_DEFINE(USE_SETUID,1,allow brute to run as setuid root)
  	AC_MSG_RESULT(yes)
],
 	AC_MSG_RESULT(no. (default))
)


# Checks for files.
AC_CHECK_FILE("/dev/urandom", [
	AC_DEFINE(DEV_URANDOM,"/dev/urandom",urandom device)
], 
[
	AC_MSG_ERROR(/dev/urandom not found)	
])

AC_CHECK_FILE("/proc/cpuinfo", [
        AC_DEFINE(PROC_CPUINFO,"/proc/cpuinfo",cpuinfo proc)
],
[
        AC_MSG_ERROR(/proc/cpuinfo not found)
])


KERNELPATH=/lib/modules/`uname -r`/build

AC_CHECK_FILE($KERNELPATH, 
,
[
        AC_MSG_ERROR(kernel headers not found. use make KERNELPATH=... )
])

# Checks for libraries.
AC_CHECK_LIB(dl, dlopen,)
AC_CHECK_LIB(m, log,)

# Checks for non standard libraries.
AC_REPLACE_FUNCS([strlcpy])
AC_REPLACE_FUNCS([strlcat])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h \ 
		  string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([gethostbyname inet_ntoa memset socket strcasecmp strchr strdup strerror])
AC_CONFIG_FILES([Makefile src/Makefile include/Makefile])
AC_OUTPUT

echo "Brute has been configured with the following options:"
echo "   compiler       : $CC"
echo "   opt_options    : $opt_options"
echo "   host_cpu       : $host_cpu"
echo "   host_vendor    : $host_vendor"
echo "   host_os        : $host_os"
echo "   CFLAGS         : $CFLAGS" 
echo "   CPPFLAGS       : $CPPFLAGS"
echo "   lib            : $LIBS"
echo "   defs           : $DEFS"
echo "   include        : $INCLS"
echo "   prefix         : $prefix"
echo "   exec_prefix    : $exec_prefix"
echo "   bindir         : $bindir" 
echo "   sbindir        : $sbindir"
echo "   libexecdir     : $libexecdir"
echo "   sysconfdir     : $sysconfdir"
echo "   libdir         : $libdir"
echo "   infodir        : $infodir"
echo "   mandir         : $mandir"
echo "   datadir        : $datadir"

