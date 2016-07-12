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

#ifndef _NC_SERVER_H_
#define _NC_SERVER_H_

#include <nc_core.h>

/*
 * server_pool is a collection of servers and their continuum. Each
 * server_pool is the owner of a single proxy connection and one or
 * more client connections. server_pool itself is owned by the current
 * context.
 *
 * Each server is the owner of one or more server connections. server
 * itself is owned by the server_pool.
 *
 *  +-------------+
 *  |             |<---------------------+
 *  |             |<------------+        |
 *  |             |     +-------+--+-----+----+--------------+
 *  |   pool 0    |+--->|          |          |              |
 *  |             |     | server 0 | server 1 | ...     ...  |
 *  |             |     |          |          |              |--+
 *  |             |     +----------+----------+--------------+  |
 *  +-------------+                                             //
 *  |             |
 *  |             |
 *  |             |
 *  |   pool 1    |
 *  |             |
 *  |             |
 *  |             |
 *  +-------------+
 *  |             |
 *  |             |
 *  .             .
 *  .    ...      .
 *  .             .
 *  |             |
 *  |             |
 *  +-------------+
 *            |
 *            |
 *            //
 */

typedef uint32_t (*hash_t)(const char *, size_t);

// 以下数据结构就是continuum环上的结点，换上的每个点其实代表了一个ip地址，该结构把点和ip地址一一对应起来。
struct continuum {
    uint32_t index;  /* server index */ //后端server服务器索引
    uint32_t value;  /* hash value */   //在环上的位置
};

//配置相关的结构体路径instance->context->conf->conf_pool(conf_server)->server_pool(server)
//配置文件中每个大的server对应的servers: 配置， servers:列表中的每一个- 192.168.1.111:7000:1对应一个server,起owner为其大server，例如alpha
//一个后端真实redis服务器对应一个server
struct server { //server_pool->server数组成员为该类型，创建空间和赋值见stats_pool_init->stats_server_map
    uint32_t           idx;           /* server index */
    //该server由谁管理，赋值见server_each_set_owner，也就是所属的server pool
    struct server_pool *owner;        /* owner pool */ //例如alpha:下面的servers:的

    //127.0.0.1:11211:1 server1中的字符串127.0.0.1:11211:1
    struct string      pname;         /* hostname:port:weight (ref in conf_server) */
    struct string      name;          /* hostname:port or [name] (ref in conf_server) */
    struct string      addrstr;       /* hostname (ref in conf_server) */
    uint16_t           port;          /* port */
    uint32_t           weight;        /* weight */
    struct sockinfo    info;          /* server socket info */

    //server_ref中自增  表示本twemproxy进程和该后端server的连接数
    uint32_t           ns_conn_q;     /* # server connection */
    //服务器conn连接队列，见server_ref  
    struct conn_tqh    s_conn_q;      /* server connection q */
    //可以防止下线后有重新上线了，但是就是选举不到的问题，下线后过这么多ms可以继续选举该服务器
    int64_t            next_retry;    /* next retry time in usec */
    //failure_count和server_failure_limit配合，见server_failure
    uint32_t           failure_count; /* # consecutive failures */ //连续读写失败次数，见server_failure
};
//配置相关的结构体路径instance->context->conf->conf_pool(conf_server)->server_pool(server)

//存储在context->pool数组中
//server_pool中的值从conf pool中拷贝过来，见conf_pool_each_transform
//解析配置项在conf_handler  ,最终存入到conf_pool，参考命令conf_commands

//conf_pool和server_pool中很多项是一样的，为什么要用两个呢?因为conf_pool就是配置文件的一份拷贝，
//而server_pool还会用于统计各种统计信息以及做hash用，可以参考conf_dump

//server_pool存储在context->pool   conf_pool结构存在conf->pool数组中  参考server_pool_init(&ctx->pool, &ctx->cf->pool, ctx);
struct server_pool { //里面的相关信息实际是最终来源还是配置文件   一个大server对应一个server_pool结构，例如大server alpha:
    uint32_t           idx;                  /* pool index */ //idx表示这是第几个pool，例如alpha  beta是第几个
    //赋值见server_pool_each_set_owner
    struct context     *ctx;                 /* owner context */

    //listen监听的时候对应的conn
    struct conn        *p_conn;              /* proxy connection (listener) */
    uint32_t           nc_conn_q;            /* # client connection */
    //对于每一个server_pool，都会有多个client连接上来，每一个client连接都放在server_pool->c_conn_q当中。
    struct conn_tqh    c_conn_q;             /* client connection q */

    //创建空间见server_init和赋值见conf_pool_each_transform      从conf_pool->server中拷贝数据过来的
    //这个配置文件中每个大server中对应的servers: 
    struct array       server;               /* server[] */ //数组成员类型为struct server
    //环上的点数  所有服务器在环上的点数总和
    uint32_t           ncontinuum;           /* # continuum points */
    //一致性hash后端有效服务器数+ KETAMA_CONTINUUM_ADDITION
    uint32_t           nserver_continuum;    /* # servers - live and dead on continuum (const) */
    //一致性hash环上面的点数和后端服务器索引的对应关系数组，可以参考ketama_dispatch  
    struct continuum   *continuum;           /* continuum */ //创建空间见nc_realloc(pool->continuum
    //后端在线的服务器数量
    uint32_t           nlive_server;         /* # live server */
    int64_t            next_rebuild;         /* next distribution rebuild time in usec */

    /*beta:
  listen: 0.0.0.0:22122
  hash: fnv1a_64  name打印为上面的beta
    */ //赋值见conf_pool_each_transform，含义可以参考conf_commands  conf_pool
    struct string      name;                 /* pool name (ref in conf_pool) */
    struct string      addrstr;              /* pool address - hostname:port (ref in conf_pool) */
    uint16_t           port;                 /* port */
    struct sockinfo    info;                 /* listen socket info */
    mode_t             perm;                 /* socket permission */
    int                dist_type;            /* distribution type (dist_type_t) */
    int                key_hash_type;        /* key hash type (hash_type_t) */
    //hash方法，见hash_algos
    hash_t             key_hash;             /* key hasher */ 
    struct string      hash_tag;             /* key hash tag (ref in conf_pool) */
    //如果配置中配置了timeout，则单位默认是ms //配置了超时选项会不会对后容易引起阻塞的命令造成误判，从而关闭连接??????
    //如果不配置超时时间，当twemproxy和redis的网络异常后，twemproxy检测不出来，客户端可能会一直阻塞等待
    int                timeout;              /* timeout in msec */ //如果不配置，默认为-1，无限大 CONF_DEFAULT_TIMEOUT
    int                backlog;              /* listen backlog */
    int                redis_db;             /* redis database to connect to */
    uint32_t           client_connections;   /* maximum # client connection */
    //后端每个server的连接数，默认为1，可以配置
    uint32_t           server_connections;   /* maximum # server connection */
    //检测到后端下线后，过这么多时间后又可以实验选择该后端
    int64_t            server_retry_timeout; /* server retry timeout in usec */
    //failure_count和server_failure_limit配合，见server_failure
    uint32_t           server_failure_limit; /* server failure limit */
    struct string      redis_auth;           /* redis_auth password (matches requirepass on redis) */
    //是否需要认真
    unsigned           require_auth;         /* require_auth? */
    //是一个boolean值，用于控制twemproxy是否应该根据server的连接状态重建群集。这个连接状态是由server_failure_limit阀值来控制。  默认是false。
    //是否在节点故障无法响应时自动摘除该节点 真正生效的地方在函数server_failure和server_pool_update
    unsigned           auto_eject_hosts:1;   /* auto_eject_hosts? */ //默认0
    unsigned           preconnect:1;         /* preconnect? */ //是否是启动起来就连接好后端服务器，还是等第一次有数据来在和后端服务器建立连接
    unsigned           redis:1;              /* redis? */
    unsigned           tcpkeepalive:1;       /* tcpkeepalive? */ //见conf_pool_each_transform
};

void server_ref(struct conn *conn, void *owner);
void server_unref(struct conn *conn);
int server_timeout(struct conn *conn);
bool server_active(struct conn *conn);
rstatus_t server_init(struct array *server, struct array *conf_server, struct server_pool *sp);
void server_deinit(struct array *server);
struct conn *server_conn(struct server *server);
rstatus_t server_connect(struct context *ctx, struct server *server, struct conn *conn);
void server_close(struct context *ctx, struct conn *conn);
void server_connected(struct context *ctx, struct conn *conn);
void server_ok(struct context *ctx, struct conn *conn);

uint32_t server_pool_idx(struct server_pool *pool, uint8_t *key, uint32_t keylen);
struct conn *server_pool_conn(struct context *ctx, struct server_pool *pool, uint8_t *key, uint32_t keylen);
rstatus_t server_pool_run(struct server_pool *pool);
rstatus_t server_pool_preconnect(struct context *ctx);
void server_pool_disconnect(struct context *ctx);
rstatus_t server_pool_init(struct array *server_pool, struct array *conf_pool, struct context *ctx);
void server_pool_deinit(struct array *server_pool);

#endif
