php-evhttp
==========

php-evhttp扩展是PHP对 [libevent](http://libevent.org/) 中的evhttp API的包装，整体上与evhttp的调用方式相同，同时去掉了PHP原生支持的一些功能，如解析URL参数、URLEncode等。

安装
==========

安装需要libevent支持，需要确保系统中安装了libevent，在libevent-1.4.13下测试通过

<pre>
phpize
./configure
make && make install
</pre>

修改php.ini或增加evhttp.ini，并加入

<pre>
extension=evhttp.so
</pre>


Methods
=========

##event_init
##### *Description*

初始化libevent

##### *Example*
<pre>
event_init();
</pre>

##evhttp_start
##### *Description*

启动一个HTTP服务，并监听对应的IP地址和端口

##### *Parameters*

*host*: string. 监听的IP地址
*port*: int. 监听的端口号

##### *Return Value*
*RESOURCE*: HTTP服务对应的资源标识

##### *Example*
<pre>
event_init();
$httpd = evhttp_start("0.0.0.0", 80);
</pre>

##evhttp_set_timeout
##### *Description*

设定HTTP请求的超时时间，单位为秒

##### *Parameters*
*httpd*: resource. HTTP服务对应的资源标识
*timeout_in_secs*: int. HTTP请求的超时时间，单位为秒

##evhttp_free
##### *Description*

释放HTTP服务对应的资源

##### *Parameters*
*httpd*: resource. HTTP服务对应的资源标识

##event_dispatch
##### *Description*

启动事件循环

##### *Return Value*
*INT*: 状态，出错时返回-1

##evhttp_set_gencb
##### *Description*

设定接受到请求后的回调函数

##### *Parameters*
*httpd*: resource. HTTP服务对应的资源标识
*callback*: callback. 请求后的回调函数
*arg*: mixed. 传给回调函数的参数

##### *Example*
<pre>
event_init();
$httpd = evhttp_start("0.0.0.0", 80);
evhttp_set_gencb($http, function($req) {
    echo "Hello World!";
})
</pre>

##evbuffer_new
##### *Description*

创建一个libevent缓冲

##### *Return Value*
*RESOURCE*: libevent缓冲资源标识

##### *Example*
<pre>
$buf = evbuffer_new();
</pre>

##evbuffer_free
##### *Description*

释放一个libevent缓冲
##### *Return Value*
*buf*: resource. libevent缓冲资源标识

##evbuffer_add
##### *Description*

向缓冲中写入一段内容
##### *Parameters*
*buf*: resource. libevent缓冲资源标识
*data*: string. 写入的内容

##### *Example*
<pre>
$buf = evbuffer_new();
evbuffer_add($buf, "Hello World");
</pre>

##evhttp_send_reply
##### *Description*

向客户端发送返回的数据

##### *Parameters*
*req*: resource. 请求资源标识
*code*: int. HTTP状态代码
*reason*: string. HTTP状态说明
*buf*: resource. 缓冲资源标识

##### *Example*
<pre>
event_init();
$httpd = evhttp_start("0.0.0.0", 80);
evhttp_set_gencb($http, function($req) {
    $buf = evbuffer_new();
    evbuffer_add($buf, "Hello World");
    evhttp_send_reply($req, EV_HTTP_OK, "OK", $buf);
})
</pre>

##evhttp_request_uri
##### *Description*

获得请求的URI
##### *Parameters*
*req*: resource. 请求资源标识

##### *Return Value*
*STRING*: 请求的URI

##evhttp_find_header
##### *Description*

获取某个请求头的内容
##### *Parameters*
*req*: resource. 请求资源标识
*name*: string. 请求头名

##### *Return Value*
*MIXED*: 请求头内容，没有获取到返回false

##evhttp_add_header
##### *Description*

增加一个返回的请求头
##### *Parameters*
*req*: resource. 请求资源标识
*name*: string. 请求头名
*value*: string. 请求头的内容

##### *Return Value*
*INT*: 成功返回0，失败返回-1

##evhttp_remove_header
##### *Description*

删除一个返回的请求头
##### *Parameters*
*req*: resource. 请求资源标识
*name*: string. 请求头名

##### *Return Value*
*INT*: 成功返回0，失败返回-1
