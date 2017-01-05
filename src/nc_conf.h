/*
 * twemproxy - A fast and lightweight proxy for memcached protocol.
 * Copyright (C) 2011 Twitter, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _NC_CONF_H_
#define _NC_CONF_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <yaml.h>

#include <nc_core.h>
#include <hashkit/nc_hashkit.h>

#define CONF_OK             (void *) NULL
#define CONF_ERROR          (void *) "has an invalid value"

#define CONF_ROOT_DEPTH     1
#define CONF_MAX_DEPTH      CONF_ROOT_DEPTH + 1

#define CONF_DEFAULT_ARGS       3
#define CONF_DEFAULT_POOL       8
#define CONF_DEFAULT_SERVERS    8

#define CONF_UNSET_NUM  -1
#define CONF_UNSET_PTR  NULL
#define CONF_UNSET_HASH (hash_type_t) -1
#define CONF_UNSET_DIST (dist_type_t) -1

#define CONF_DEFAULT_HASH                    HASH_FNV1A_64
#define CONF_DEFAULT_DIST                    DIST_KETAMA //ketama
#define CONF_DEFAULT_TIMEOUT                 -1
#define CONF_DEFAULT_LISTEN_BACKLOG          512
#define CONF_DEFAULT_CLIENT_CONNECTIONS      0
#define CONF_DEFAULT_REDIS                   false
#define CONF_DEFAULT_REDIS_DB                0
#define CONF_DEFAULT_PRECONNECT              false
#define CONF_DEFAULT_AUTO_EJECT_HOSTS        false
#define CONF_DEFAULT_SERVER_RETRY_TIMEOUT    30 * 1000      /* in msec */
#define CONF_DEFAULT_SERVER_FAILURE_LIMIT    2
#define CONF_DEFAULT_SERVER_CONNECTIONS      1
#define CONF_DEFAULT_KETAMA_PORT             11211
#define CONF_DEFAULT_TCPKEEPALIVE            false

struct conf_listen {
    struct string   pname;   /* listen: as "hostname:port" */
    struct string   name;    /* hostname:port */
    int             port;    /* port */
    mode_t          perm;    /* socket permissions */
    struct sockinfo info;    /* listen socket info */
    unsigned        valid:1; /* valid? */
};

//配置相关的结构体路径instance->context->conf->conf_pool(conf_server)->server_pool(server)
struct conf_server { //解析在conf_add_server
    //- 127.0.0.1:11211:1 server1中的- 127.0.0.1:11211:1
    struct string   pname;      /* server: as "hostname:port:weight" */
    //- 127.0.0.1:11211:1 server1中的server1   
    //注意如果后端服务器配置为- 127.0.0.1:11211:1，没有serverx这样的标识，并且端口为11211，则name不会带有11211:1  
    //这对多进程stats的求和会有影响，见conf_add_server
    struct string   name;       /* hostname:port or [name] */
    struct string   addrstr;    /* hostname */
    int             port;       /* port */
    int             weight;     /* weight */
    struct sockinfo info;       /* connect socket info */
    unsigned        valid:1;    /* valid? */
};

//配置相关的结构体路径instance->context->conf->conf_pool(conf_server)->server_pool(server)
//解析配置项在conf_handler  ,最终存入到conf_pool，参考命令conf_commands
//配置文件中每一个pool对应的配置信息   存储在conf->pool中

//conf_pool和server_pool中很多项是一样的，为什么要用两个呢?因为conf_pool就是配置文件的一份拷贝，
//而server_pool还会用于统计各种统计信息以及做hash用，可以参考conf_dump

//server_pool存储在context->pool   conf_pool结构存在conf->pool数组中  参考server_pool_init(&ctx->pool, &ctx->cf->pool, ctx);
struct conf_pool { //该结构里面的相关项的默认值为conf_validate_pool         conf_pool结构存在conf->pool数组中
    struct string      name;                  /* pool name (root node)  */
    //参考conf_commands  解析配置项在conf_handler 
    struct conf_listen listen;                /* listen: */
    /*
    hash  可以选择的key值的hash算法：
>one_at_a_time >md5 >crc16  
>crc32 (crc32 implementation compatible with libmemcached) 
>crc32a (correct crc32 implementation as per the spec) 
>fnv1_64 >fnv1a_64 >fnv1_32 >fnv1a_32 >hsieh >murmur >jenkins   如果没选择，默认是fnv1a_64。
    */
    hash_type_t        hash;                  /* hash: */  //默认值CONF_DEFAULT_HASH
    /*
    比如一个商品有：商品基本信息(p:id:)、商品介绍(d:id:)、颜色尺码(c:id:)等，假设我们存储时不采用HashTag将
    会导致这些数据不会存储到一个分片，而是分散到多个分片，这样获取时将需要从多个分片获取数据进行合并，无法
    进行mget；那么如果有了HashTag，那么可以使用“::”中间的数据做分片逻辑，这样id一样的将会分到一个分片。
    */ //hash_tag: "{}"表示计算hash值时，只取key中包含在{}里面的那部分来计算 
    struct string      hash_tag;              /* hash_tag: */
    //存在ketama、modula和random3种可选的配置  取值见DIST_CODEC
    dist_type_t        distribution;          /* distribution: */
    //单位是毫秒，是连接到server的超时值。默认是永久等待。  单位ms   //如果配置中配置了timeout，则单位默认是ms
    //配置了超时选项会不会对后容易引起阻塞的命令造成误判，从而关闭连接??????
    //如果不配置超时时间，当twemproxy和redis的网络异常后，twemproxy检测不出来，客户端可能会一直阻塞等待
    int                timeout;               /* timeout: */ //如果不配置，默认无限大 CONF_DEFAULT_TIMEOUT
    //监听TCP 的backlog（连接等待队列）的长度，默认是512。
    int                backlog;               /* backlog: */
    //赋值见proxy_accept增加，客户端连接数。client_close_stats中减少。
    int                client_connections;    /* client_connections: */
    //是否启用内核SO_KEEPALIVE
    int                tcpkeepalive;          /* tcpkeepalive: */ //最终赋值给server_pool->tcpkeepalive，见conf_pool_each_transform
    //是否连接到redis
    int                redis;                 /* redis: */
    //如果配置了redis_auth，则必须redis配置为true,见conf_validate_pool
    struct string      redis_auth;            /* redis_auth: redis auth password (matches requirepass on redis) */
    int                redis_db;              /* redis_db: redis db */ //默认db0
    //是一个boolean值，指示twemproxy是否应该预连接pool中的server。默认是false。
    int                preconnect;            /* preconnect: */
    //是一个boolean值，用于控制twemproxy是否应该根据server的连接状态重建群集。这个连接状态是由server_failure_limit阀值来控制。  默认是false。
    //是否在节点故障无法响应时自动摘除该节点  真正生效的地方在函数server_failure和server_pool_update
    int                auto_eject_hosts;      /* auto_eject_hosts: */
    //每个server可以被打开的连接数。默认，每个服务器开一个连接。
    int                server_connections;    /* server_connections: */
    //单位是毫秒，控制服务器连接的时间间隔，在auto_eject_host被设置为true的时候产生作用。默认是30000 毫秒。
    //重试时间（毫秒），重新连接一个临时摘掉的故障节点的间隔，如果判断节点正常会自动加到一致性Hash环上
    int                server_retry_timeout;  /* server_retry_timeout: in msec */
    //控制连接服务器的次数，在auto_eject_host被设置为true的时候产生作用。默认是2。  节点故障无法响应多少次从一致性Hash环临时摘掉它，默认是2
    int                server_failure_limit;  /* server_failure_limit: */ 
    /*
    一个pool中的服务器的地址、端口和权重的列表，包括一个可选的服务器的名字，如果提供服务器的名字，将会使用它决定server
    的次序，从而提供对应的一致性hash的hash ring。否则，将使用server被定义的次序。
    */ //最终server_init conf_pool_each_transform拷贝到server_pool->server中
    struct array       server;                /* servers: conf_server[] */
    //标记该pool是否有效
    unsigned           valid:1;               /* valid? */
};

//配置相关的结构体路径instance->context->conf->conf_pool(conf_server)->server_pool(server)
//该结构存放在instance->cf中
struct conf { //创建空间和初始化见conf_open   配置真正存储在conf->pool中的数组成员conf_pool中
    //-c参数指定配置文件路径
    char          *fname;           /* file name (ref in argv[]) */
    //打开fname文件产生的文件句柄
    FILE          *fh;              /* file handle */ 
    struct array  arg;              /* string[] (parsed {key, value} pairs) */

    /*
    alpha:
  listen: 127.0.0.1:22121
  hash: fnv1a_64
  distribution: ketama
  auto_eject_hosts: true
  redis: true
  server_retry_timeout: 2000
  server_failure_limit: 1
  servers:
   - 127.0.0.1:6379:1

beta:
  listen: 127.0.0.1:22122
  hash: fnv1a_64
  hash_tag: "{}"
  distribution: ketama
  auto_eject_hosts: false
  timeout: 400
  redis: true
  servers:
   - server1 127.0.0.1:6380:1
   - server2 127.0.0.1:6381:1
   - server3 127.0.0.1:6382:1
   - server4 127.0.0.1:6383:1
    */ //alpha和beta各自对应一个conf_pool结构
    //成员类型conf_pool  真正的配置存放在数组成员conf_pool中
    struct array  pool;             /* conf_pool[] (parsed pools) */  
    uint32_t      depth;            /* parsed tree depth */
    yaml_parser_t parser;           /* yaml parser */
    yaml_event_t  event;            /* yaml event */
    yaml_token_t  token;            /* yaml token */
    unsigned      seq:1;            /* sequence? */
    unsigned      valid_parser:1;   /* valid parser? */
    unsigned      valid_event:1;    /* valid event? */
    unsigned      valid_token:1;    /* valid token? */
    unsigned      sound:1;          /* sound? */
    unsigned      parsed:1;         /* parsed? */
    unsigned      valid:1;          /* valid? */
};

struct command {
    struct string name;
    char          *(*set)(struct conf *cf, struct command *cmd, void *data);
    int           offset;
};

#define null_command { null_string, NULL, 0 }

char *conf_set_string(struct conf *cf, struct command *cmd, void *conf);
char *conf_set_listen(struct conf *cf, struct command *cmd, void *conf);
char *conf_add_server(struct conf *cf, struct command *cmd, void *conf);
char *conf_set_num(struct conf *cf, struct command *cmd, void *conf);
char *conf_set_bool(struct conf *cf, struct command *cmd, void *conf);
char *conf_set_hash(struct conf *cf, struct command *cmd, void *conf);
char *conf_set_distribution(struct conf *cf, struct command *cmd, void *conf);
char *conf_set_hashtag(struct conf *cf, struct command *cmd, void *conf);

rstatus_t conf_server_each_transform(void *elem, void *data);
rstatus_t conf_pool_each_transform(void *elem, void *data);

struct conf *conf_create(char *filename);
void conf_destroy(struct conf *cf);

#endif
