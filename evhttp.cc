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
  | Author: Long Long <longlong2010@gmail.com>                           |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
extern "C" {
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_evhttp.h"
#include <event.h>
#include <evhttp.h>
}
/* If you declare any globals in php_evhttp.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(evhttp)
*/

/* True global resources - no need for thread safety here */
static int le_evhttp;
static int le_evbuffer;
static int le_evhttp_request;

typedef struct _php_evhttp_callback {
	zval* func;
	zval* arg;
} php_evhttp_callback;

typedef struct _php_evhttp {
	struct evhttp* httpd;
	php_evhttp_callback* callback;
} php_evhttp;

typedef struct _php_evbuffer {
	struct evbuffer* buf;
} php_evbuffer;

typedef struct _php_evhttp_request {
	struct evhttp_request* req;
} php_evhttp_request;
/* {{{ evhttp_functions[]
 *
 * Every user visible function must have an entry in evhttp_functions[].
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_evhttp_set_gencb, 0, 0, 2)
	ZEND_ARG_INFO(0, http)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(1, arg)
ZEND_END_ARG_INFO()

const zend_function_entry evhttp_functions[] = {
	PHP_FE(event_init,	NULL)		/* For testing, remove later. */
	PHP_FE(evhttp_start, NULL)
	PHP_FE(evhttp_set_timeout, NULL)
	PHP_FE(evhttp_set_gencb, arginfo_evhttp_set_gencb)
	PHP_FE(event_dispatch, NULL)
	PHP_FE(evhttp_free, NULL)
	PHP_FE(evbuffer_new, NULL)
	PHP_FE(evbuffer_free, NULL)
	PHP_FE(evbuffer_add, NULL)
	PHP_FE(evhttp_send_reply, NULL)
	PHP_FE(evhttp_request_uri, NULL)
	PHP_FE(evhttp_find_header, NULL)
	PHP_FE(evhttp_add_header, NULL)
	PHP_FE(evhttp_remove_header, NULL)
	PHP_FE_END	/* Must be the last line in evhttp_functions[] */
};
/* }}} */

/* {{{ evhttp_module_entry
 */
zend_module_entry evhttp_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"evhttp",
	evhttp_functions,
	PHP_MINIT(evhttp),
	PHP_MSHUTDOWN(evhttp),
	PHP_RINIT(evhttp),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(evhttp),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(evhttp),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_EVHTTP
ZEND_GET_MODULE(evhttp)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("evhttp.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_evhttp_globals, evhttp_globals)
    STD_PHP_INI_ENTRY("evhttp.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_evhttp_globals, evhttp_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_evhttp_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_evhttp_init_globals(zend_evhttp_globals *evhttp_globals)
{
	evhttp_globals->global_value = 0;
	evhttp_globals->global_string = NULL;
}
*/
/* }}} */
static void _evhttp_free(zend_rsrc_list_entry* rsrc TSRMLS_DC) {
	php_evhttp* evh = (php_evhttp*) rsrc->ptr;
	evhttp_free(evh->httpd);
	efree(evh);
}

static void _evbuffer_free(zend_rsrc_list_entry* rsrc TSRMLS_DC) {
	php_evbuffer* evb = (php_evbuffer*) rsrc->ptr;
	evbuffer_free(evb->buf);
	efree(evb);
}

static void _evhttp_request_free(zend_rsrc_list_entry* rsrc TSRMLS_DC) {
	php_evhttp_request* evr = (php_evhttp_request*) rsrc->ptr;
	efree(evr);
}
/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(evhttp)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	le_evhttp = zend_register_list_destructors_ex(_evhttp_free, NULL, "evhttp", module_number);
	le_evbuffer = zend_register_list_destructors_ex(_evbuffer_free, NULL, "evbuffer", module_number);
	le_evhttp_request = zend_register_list_destructors_ex(_evhttp_request_free, NULL, "evhttp_request", module_number);

	REGISTER_LONG_CONSTANT("EV_HTTP_BADREQUEST", HTTP_BADREQUEST, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EV_HTTP_MOVEPERM", HTTP_MOVEPERM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EV_HTTP_MOVETEMP", HTTP_MOVETEMP, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EV_HTTP_NOCONTENT", HTTP_NOCONTENT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EV_HTTP_NOTFOUND", HTTP_NOTFOUND, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EV_HTTP_NOTMODIFIED", HTTP_NOTMODIFIED, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EV_HTTP_OK", HTTP_OK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("EV_HTTP_SERVUNAVAIL", HTTP_SERVUNAVAIL, CONST_CS | CONST_PERSISTENT);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(evhttp)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(evhttp)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(evhttp)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(evhttp)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "evhttp support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_evhttp_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(event_init)
{
	event_init();
}

PHP_FUNCTION(evhttp_start) {
	const char* addr;
	int addr_len;
	int port;
	php_evhttp* evh;	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &addr, &addr_len, &port) == FAILURE) {
		return;
	}
	evh = (php_evhttp*) emalloc(sizeof(php_evhttp));
	evh->callback = NULL;
	evh->httpd = evhttp_start(addr, port);
	ZEND_REGISTER_RESOURCE(return_value, evh, le_evhttp);
}

static void _php_evhttp_callback(struct evhttp_request* req, void* arg) {
	zval* args[2];
	zval retval;
	php_evhttp_request* evr;

	php_evhttp_callback* callback = (php_evhttp_callback*) arg;
	evr = (php_evhttp_request*) emalloc(sizeof(php_evhttp_request));
	evr->req = req;

	MAKE_STD_ZVAL(args[0]);
	ZEND_REGISTER_RESOURCE(args[0], evr, le_evhttp_request);

	args[1] = callback->arg;
	Z_ADDREF_P(callback->arg);

	if (call_user_function(EG(function_table), NULL, callback->func, &retval, 2, args TSRMLS_CC) == SUCCESS) {
		zval_dtor(&retval);
	}
	zval_ptr_dtor(&(args[0]));
	zval_ptr_dtor(&(args[0]));
}

PHP_FUNCTION(evhttp_set_gencb) {
	php_evhttp* evh;
	zval* zevh;
	zval* zcallback;
	zval* zarg = NULL;
	char* func_name;
	php_evhttp_callback* callback;
	php_evhttp_callback* old_callback;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz|z", &zevh, &zcallback, &zarg) == FAILURE) {
		return;
	}
	
	if (!zend_is_callable(zcallback, 0, &func_name TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a valid callback", func_name);
		efree(func_name);
		RETURN_FALSE;
	}
	
	efree(func_name);
	zval_add_ref(&zcallback);
	
	if (zarg) {
		zval_add_ref(&zarg);
	} else {
		ALLOC_INIT_ZVAL(zarg);
	}
	
	ZEND_FETCH_RESOURCE(evh, php_evhttp*, &zevh, -1, "evhttp", le_evhttp);

	callback = (php_evhttp_callback*) emalloc(sizeof(php_evhttp_callback));
	callback->func = zcallback;
	callback->arg = zarg;

	old_callback = evh->callback;
	evh->callback = callback;

	evhttp_set_gencb(evh->httpd, _php_evhttp_callback, callback);
	
	if (old_callback) {
		zval_ptr_dtor(&old_callback->func);
		if (old_callback->arg) {
			zval_ptr_dtor(&old_callback->arg);
		}
		efree(old_callback);
	}
	RETURN_TRUE;
}

PHP_FUNCTION(event_dispatch) {
	RETURN_LONG(event_dispatch());
}

PHP_FUNCTION(evhttp_set_timeout) {
	php_evhttp* evh;
	zval* zevh;
	long timeout_in_secs;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &zevh, &timeout_in_secs) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(evh, php_evhttp*, &zevh, -1, "evhttp", le_evhttp);

	evhttp_set_timeout(evh->httpd, timeout_in_secs);
}

PHP_FUNCTION(evhttp_free) {
	zval* zevh;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zevh) == FAILURE) {
		return;
	}
	zend_list_delete(Z_LVAL_P(zevh));
	RETURN_TRUE;
}

PHP_FUNCTION(evbuffer_new) {
	php_evbuffer* evb;
	evb = (php_evbuffer*) emalloc(sizeof(php_evbuffer));
	evb->buf = evbuffer_new();
	ZEND_REGISTER_RESOURCE(return_value, evb, le_evbuffer);
}

PHP_FUNCTION(evbuffer_free) {
	zval* zevb;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zevb) == FAILURE) {
		return;
	}
	zend_list_delete(Z_LVAL_P(zevb));
	RETURN_TRUE;
}

PHP_FUNCTION(evbuffer_add) {
	php_evbuffer* evb;
	zval* zevb;
	const char* data;
	int data_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &zevb, &data, &data_len) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(evb, php_evbuffer*, &zevb, -1, "evbuffer", le_evbuffer);
	RETURN_LONG(evbuffer_add(evb->buf, data, data_len));
}

PHP_FUNCTION(evhttp_send_reply) {
	php_evhttp_request* evr;
	zval* zevr;

	php_evbuffer* evb;
	zval* zevb;

	long code;
	const char* reason;
	int reason_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlsr", &zevr, &code, &reason, &reason_len, &zevb) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(evb, php_evbuffer*, &zevb, -1, "evbuffer", le_evbuffer);
	ZEND_FETCH_RESOURCE(evr, php_evhttp_request*, &zevr, -1, "evhttp_request", le_evhttp_request);
	evhttp_send_reply(evr->req, code, reason, evb->buf);
	RETURN_TRUE;
}

PHP_FUNCTION(evhttp_request_uri) {
	php_evhttp_request* evr;
	zval* zevr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zevr) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(evr, php_evhttp_request*, &zevr, -1, "evhttp_request", le_evhttp_request);
	const char* uri = evhttp_request_uri(evr->req);
	RETURN_STRING(uri, strlen(uri));
}

PHP_FUNCTION(evhttp_find_header) {
	php_evhttp_request* evr;
	zval* zevr;
	const char* name;
	int name_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &zevr, &name, &name_len) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(evr, php_evhttp_request*, &zevr, -1, "evhttp_request", le_evhttp_request);
	const char* header = evhttp_find_header(evr->req->input_headers, name);
	if (header) {
		RETURN_STRING(header, strlen(header));
	} else {
		RETURN_FALSE;
	}
}

PHP_FUNCTION(evhttp_add_header) {
	php_evhttp_request* evr;
	zval* zevr;
	const char* name;
	int name_len;
	const char* value;
	int value_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &zevr, &name, &name_len, &value, &value_len) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(evr, php_evhttp_request*, &zevr, -1, "evhttp_request", le_evhttp_request);
	RETURN_LONG(evhttp_add_header(evr->req->output_headers, name, value));
}

PHP_FUNCTION(evhttp_remove_header) {
	php_evhttp_request* evr;
	zval* zevr;
	const char* name;
	int name_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &zevr, &name, &name_len) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(evr, php_evhttp_request*, &zevr, -1, "evhttp_request", le_evhttp_request);
	RETURN_LONG(evhttp_remove_header(evr->req->output_headers, name));
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
