#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([netserver], [0.1], [])
AC_CONFIG_SRCDIR([tester/test_main.cpp])
AC_CONFIG_HEADERS([config.h])
#AC_CONFIG_TESTDIR(test)

AM_INIT_AUTOMAKE()

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXX_C_O
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.

AC_CHECK_HEADERS(linux/futex.h sched.h fcntl.h sys/ioctl.h sys/mman.h)


# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([
				 Makefile
				 source/Makefile
				 include/Makefile
			   ])
			   
HAVE_TEST=no
			   
AC_SUBST(HAVE_TEST)
			   
AC_ENABLE( test, [
				HAVE_TEST=yes;			   
			   AC_CONFIG_FILES(tester/Makefile) 
			     ] )

AC_OUTPUT
