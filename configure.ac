AC_INIT([marc], [0.0.1], [cliff.snyder@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_AR
LT_INIT
AC_PROG_CC
AC_CHECK_HEADERS(zlib.h)

AC_MSG_CHECKING([whether to enable use of zlib for reading gzip files])
AC_ARG_ENABLE([zlib],                                                                                                                
        AS_HELP_STRING([--disable-zlib],[disable gzip file support]))
AH_TEMPLATE([USE_ZLIB],
[Undefine if you do not want to include gzip file support. By default this is defined.])
if test x"$enable_zlib" != x"no"; then
    if test x"$ac_cv_header_zlib_h" = x"yes"; then
        AC_MSG_RESULT(yes)
        AC_SEARCH_LIBS(gzread, z,
            [AC_DEFINE(USE_ZLIB)],
            [AC_MSG_ERROR(zlib library not found)])
    else
        AC_MSG_RESULT(no)
        AC_MSG_ERROR(zlib header file not found)
    fi
else
    AC_MSG_RESULT(no)
fi

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIRS([m4])
AC_OUTPUT
