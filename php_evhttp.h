/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_EVHTTP_H
#define PHP_EVHTTP_H

extern zend_module_entry evhttp_module_entry;
#define phpext_evhttp_ptr &evhttp_module_entry

#ifdef PHP_WIN32
#	define PHP_EVHTTP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_EVHTTP_API __attribute__ ((visibility("default")))
#else
#	define PHP_EVHTTP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(evhttp);
PHP_MSHUTDOWN_FUNCTION(evhttp);
PHP_RINIT_FUNCTION(evhttp);
PHP_RSHUTDOWN_FUNCTION(evhttp);
PHP_MINFO_FUNCTION(evhttp);

PHP_FUNCTION(event_init);	/* For testing, remove later. */
PHP_FUNCTION(evhttp_start);
PHP_FUNCTION(evhttp_set_timeout);
PHP_FUNCTION(evhttp_set_gencb);
PHP_FUNCTION(event_dispatch);
PHP_FUNCTION(evhttp_free);
PHP_FUNCTION(evbuffer_new);
PHP_FUNCTION(evbuffer_free);
PHP_FUNCTION(evbuffer_add);
PHP_FUNCTION(evhttp_send_reply);
PHP_FUNCTION(evhttp_request_uri);
PHP_FUNCTION(evhttp_find_header);
PHP_FUNCTION(evhttp_add_header);
PHP_FUNCTION(evhttp_remove_header);
/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(evhttp)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(evhttp)
*/

/* In every utility function you add that needs to use variables 
   in php_evhttp_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as EVHTTP_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define EVHTTP_G(v) TSRMG(evhttp_globals_id, zend_evhttp_globals *, v)
#else
#define EVHTTP_G(v) (evhttp_globals.v)
#endif

#endif	/* PHP_EVHTTP_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
