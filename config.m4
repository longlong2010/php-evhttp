dnl $Id$
dnl config.m4 for extension evhttp

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(evhttp, for evhttp support,
Make sure that the comment is aligned:
[  --with-evhttp             Include evhttp support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(evhttp, whether to enable evhttp support,
dnl Make sure that the comment is aligned:
dnl [  --enable-evhttp           Enable evhttp support])

if test -z "$PHP_DEBUG"; then
    AC_ARG_ENABLE(debug,
    [  --enable-debug      compile with debugging symbols],[
       PHP_DEBUG=$enableval
    ],[    PHP_DEBUG=no])
fi

if test "$PHP_EVHTTP" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-evhttp -> check with-path
  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="/include/evhttp.h"
  if test -r $PHP_EVHTTP/$SEARCH_FOR; then
    EVHTTP_DIR=$PHP_EVHTTP
  else # search default path list
    AC_MSG_CHECKING([for evhttp files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        EVHTTP_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$EVHTTP_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the evhttp distribution])
  fi

  dnl # --with-evhttp -> add include path
  PHP_ADD_INCLUDE($EVHTTP_DIR/include)

  dnl # --with-evhttp -> check for lib and symbol presence
  LIBNAME=event
  LIBSYMBOL=event_base_new

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $EVHTTP_DIR/lib, EVHTTP_SHARED_LIBADD)
    AC_DEFINE(HAVE_EVHTTPLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong evhttp lib version or lib not found])
  ],[
    -L$EVHTTP_DIR/lib -lm
  ])
  
  PHP_SUBST(EVHTTP_SHARED_LIBADD)
  PHP_ADD_LIBRARY(stdc++, "", EXTERN_NAME_LIBADD)
  PHP_REQUIRE_CXX
  PHP_NEW_EXTENSION(evhttp, evhttp.cc, $ext_shared)
fi
