m4_define([__VERSION_MAJOR__], [0])
m4_define([__VERSION_MINOR__], [1])
m4_define([__VERSION_MICRO__], [0])

AC_INIT([tweaklib], m4_defn([__VERSION_MAJOR__]).m4_defn([__VERSION_MINOR__]).m4_defn([__VERSION_MICRO__]), [])

VERSION_MAJOR=__VERSION_MAJOR__
VERSION_MINOR=__VERSION_MINOR__
VERSION_MICRO=__VERSION_MICRO__

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.11 foreign color-tests subdir-objects -Wall -Werror])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
  tweak/version.h
  Makefile
])

AC_DEFINE_UNQUOTED([VERSION_MAJOR],  [${VERSION_MAJOR}], [Major version number of package])
AC_DEFINE_UNQUOTED([VERSION_MINOR],  [${VERSION_MINOR}], [Minor version number of package])
AC_DEFINE_UNQUOTED([VERSION_MICRO],  [${VERSION_MICRO}], [Micro version number of package])
AC_DEFINE([TWEAKLIB_EXPORT], [1], [Export symbols from library])

AM_PROG_CC_C_O
AC_PROG_CC_C99
AC_USE_SYSTEM_EXTENSIONS
LT_INIT

AC_OUTPUT
