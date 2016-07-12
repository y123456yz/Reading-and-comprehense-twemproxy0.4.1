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

#ifndef _NC_STATS_H_
#define _NC_STATS_H_

#include <nc_core.h>

//可以参考stats_pool_field的用法    
#define STATS_POOL_CODEC(ACTION)                                                                                 \
    /* client behavior */                                                                                           \
    ACTION( client_eof,             STATS_COUNTER,      "# eof on client connections")                              \
    ACTION( client_err,             STATS_COUNTER,      "# errors on client connections")                           \
    ACTION( client_connections,     STATS_GAUGE,        "# active client connections")                              \
    /* pool behavior */                                                                                             \
    ACTION( server_ejects,          STATS_COUNTER,      "# times backend server was ejected")                       \
    /* forwarder behavior */                                                                                        \
    ACTION( forward_error,          STATS_COUNTER,      "# times we encountered a forwarding error")                \
    ACTION( fragments,              STATS_COUNTER,      "# fragments created from a multi-vector request")          \

//可以参考stats_server_field的用法   
#define STATS_SERVER_CODEC(ACTION)                                                                                  \
    /* server behavior */                                                                                           \
    ACTION( server_eof,             STATS_COUNTER,      "# eof on server connections")                              \
    ACTION( server_err,             STATS_COUNTER,      "# errors on server connections")                           \
    ACTION( server_timedout,        STATS_COUNTER,      "# timeouts on server connections")                         \
    ACTION( server_connections,     STATS_GAUGE,        "# active server connections")                              \
    ACTION( server_ejected_at,      STATS_TIMESTAMP,    "timestamp when server was ejected in usec since epoch")    \
    /* data behavior */                                                                                             \
    /*客户端请求数  参考 req_forward_stats //客户端请求字节数  参考 req_forward_stats*/         \
    ACTION( requests,               STATS_COUNTER,      "# requests")                                               \
    ACTION( request_bytes,          STATS_COUNTER,      "total request bytes")                                      \
    ACTION( responses,              STATS_COUNTER,      "# responses")                                              \
    ACTION( response_bytes,         STATS_COUNTER,      "total response bytes")                                     \
    /* req_server_dequeue_imsgq */   \
    ACTION( in_queue,               STATS_GAUGE,        "# requests in incoming queue")                             \
    ACTION( in_queue_bytes,         STATS_GAUGE,        "current request bytes in incoming queue")                  \
    ACTION( out_queue,              STATS_GAUGE,        "# requests in outgoing queue")                             \
    ACTION( out_queue_bytes,        STATS_GAUGE,        "current request bytes in outgoing queue")                  \

/*
[root@s10-2-20-2 twemproxy]# telnet 127.0.0.1 22222
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
{
"service":"nutcracker", "source":"s10-2-20-2.hx", "version":"0.4.1", "uptime":25, "timestamp":1467272574, 
"total_connections":1, "curr_connections":1, 
"alpha": {
        "client_eof":0, "client_err":0, "client_connections":0, "server_ejects":0, "forward_error":0, "fragments":0, 
        "server1": {
            "server_eof":0, "server_err":0, "server_timedout":0, "server_connections":0, "server_ejected_at":0, 
            "requests":0, "request_bytes":0, "responses":0, "response_bytes":0, 
            "in_queue":0, "in_queue_bytes":0, "out_queue":0, "out_queue_bytes":0
        },
        "server2": {
            "server_eof":0, "server_err":0,  "server_timedout":0, "server_connections":0, "server_ejected_at":0, "requests":0, 
            "request_bytes":0, "responses":0, "response_bytes":0, "in_queue":0, "in_queue_bytes":0, "out_queue":0, "out_queue_bytes":0
        },
        "server3": {
            "server_eof":0, "server_err":0, "server_timedout":0, "server_connections":0, "server_ejected_at":0,
            "requests":0, "request_bytes":0, "responses":0, "response_bytes":0, "in_queue":0, "in_queue_bytes":0,
            "out_queue":0, "out_queue_bytes":0
        },
        "server4": {
            "server_eof":0, "server_err":0, "server_timedout":0, "server_connections":0, 
            "server_ejected_at":0, "requests":0, "request_bytes":0, "responses":0, "response_bytes":0, "in_queue":0, 
            "in_queue_bytes":0, "out_queue":0, "out_queue_bytes":0
        }
    }
}
Connection closed by foreign host.
*/
#define STATS_ADDR      "0.0.0.0"
#define STATS_PORT      22222
#define STATS_INTERVAL  (30 * 1000) /* in msec */

typedef enum stats_type { //参考stats_metric_init
    STATS_INVALID,
    STATS_COUNTER,    /* monotonic accumulator */
    STATS_GAUGE,      /* non-monotonic accumulator */
    STATS_TIMESTAMP,  /* monotonic timestamp (in nsec) */
    STATS_SENTINEL
} stats_type_t;

//该结构内容在stats_make_rsp中会组包发送给客户端

//创建空间和赋值见stats_pool_metric_init
struct stats_metric { //stats_pool->metric中的成员           可以参考stats_server_to_metric
    stats_type_t  type;         /* type */
    struct string name;         /* name (ref) */
    union {
        int64_t   counter;      /* accumulating counter */
        int64_t   timestamp;    /* monotonic timestamp */
    } value;
};

//创建空间和赋值见stats_server_map->stats_server_init
struct stats_server {
    //server_pool->name
    struct string name;   /* server name (ref) */ //
    //数组成员为stats_metric
    struct array  metric; /* stats_metric[] for server codec */
};

//多个大的server，例如alpha delta等各自对应一个stats_poll结构存在于stats->current,stats->shadow,stats->sum中，
//创建空间和赋值见stats_pool_map
struct stats_pool {
    //也就是server_pool->name中的大server名，例如alpha
    struct string name;   /* pool name (ref) */ //大server名赋值见stats_begin_nesting
    //创建空间和赋值见stats_pool_init->stats_pool_metric_init，成员类型为stats_metric ,该数组的内容来自stats_pool_codec
    struct array  metric; /* stats_metric[] for pool codec */ //该数组中的数据来源是stats_pool_codec
    //创建空间和赋值见stats_pool_init->stats_server_map，成员类型为stats_server
    //大server中的server:配置列表有多少个， 
    struct array  server; /* stats_server[] */ //该数组中的来源是nutcracker.yul配置文件中的server列表信息
};

struct stats_buffer {
    size_t   len;   /* buffer length */
    uint8_t  *data; /* buffer data */
    size_t   size;  /* buffer alloc size */
};

/*
[root@s10-2-20-2 twemproxy]# telnet 127.0.0.1 22222
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
{
"service":"nutcracker", "source":"s10-2-20-2.hx", "version":"0.4.1", "uptime":25, "timestamp":1467272574, 
"total_connections":1, "curr_connections":1, 
"alpha": {
        "client_eof":0, "client_err":0, "client_connections":0, "server_ejects":0, "forward_error":0, "fragments":0, 
        "server1": {
            "server_eof":0, "server_err":0, "server_timedout":0, "server_connections":0, "server_ejected_at":0, 
            "requests":0, "request_bytes":0, "responses":0, "response_bytes":0, 
            "in_queue":0, "in_queue_bytes":0, "out_queue":0, "out_queue_bytes":0
        },
        "server2": {
            "server_eof":0, "server_err":0,  "server_timedout":0, "server_connections":0, "server_ejected_at":0, "requests":0, 
            "request_bytes":0, "responses":0, "response_bytes":0, "in_queue":0, "in_queue_bytes":0, "out_queue":0, "out_queue_bytes":0
        },
        "server3": {
            "server_eof":0, "server_err":0, "server_timedout":0, "server_connections":0, "server_ejected_at":0,
            "requests":0, "request_bytes":0, "responses":0, "response_bytes":0, "in_queue":0, "in_queue_bytes":0,
            "out_queue":0, "out_queue_bytes":0
        },
        "server4": {
            "server_eof":0, "server_err":0, "server_timedout":0, "server_connections":0, 
            "server_ejected_at":0, "requests":0, "request_bytes":0, "responses":0, "response_bytes":0, "in_queue":0, 
            "in_queue_bytes":0, "out_queue":0, "out_queue_bytes":0
        }
    }
}
Connection closed by foreign host.
\
*/

//该结构空间存在于context->stats
//创建空间和赋值见stats_create
struct stats {//存储在context->stats成员中，见stats_create
    //监听端口  //监听地址和端口 可以通过-s设置，赋值见stats_create
    uint16_t            port;            /* stats monitoring port */
    int                 interval;        /* stats aggregation interval */ //-i参数设置
    struct string       addr;            /* stats monitoring address */

    //最近一次客户端获取统计信息的时间戳，目的计算客户端两次统计之间间隔了多少
    int64_t             start_ts;        /* start timestamp of nutcracker */
    //创建空间和赋值见stats_create_buf 应答给telnet xxx.x.x.x 22222的报文内容都放在stats->buf中的
    struct stats_buffer buf;             /* output buffer */

    //多个大的server，例如alpha delta等各自对应一个stats_poll结构存在于stats->current,stats->shadow,stats->sum中，

    /* 所有的统计和都会放入到sum中，一段时间内的统计放在current中,curren中的统计实际上是上次获取信息和
    当前这次获取信息这段时间差内的统计信息,这次获取到客户端连接22222stat端口的时候，stats_swap会把curren中的内容拷贝到
    shadow中，stats_aggregate然后把shadow和sum求和放入sum，最后应答给客户端 */
    //成员类型stats_pool，见stats_create->stats_pool_map
    //下面三个的数组成员类型都是stats_pool，见stats_create->stats_pool_map
    //current shadow sum的区别联系可以参考stats_swap stats_aggregate
    struct array        current;         /* stats_pool[] (a) */
    struct array        shadow;          /* stats_pool[] (b) */
    //sum是对shadow中的相关信息进行求和，见stats_aggregate  
    /* 所有的统计和都会放入到sum中，一段时间内的统计放在current中,curren中的统计实际上是上次获取信息和
    当前这次获取信息这段时间差内的统计信息,这次获取到客户端连接22222stat端口的时候，stats_swap会把curren中的内容拷贝到
    shadow中，stats_aggregate然后把shadow和sum求和放入sum，最后应答给客户端 */
    struct array        sum;             /* stats_pool[] (c = a + b) */

    pthread_t           tid;             /* stats aggregator thread */
    //套接字见stats_listen  epoll触发事件见event_loop_stats  接受客户端连接及发送stats回应在stats_send_rsp
    int                 sd;              /* stats descriptor */

    //stat组织格式可以参考stats_create_buf
    struct string       service_str;     /* service string */
    struct string       service;         /* service */
    struct string       source_str;      /* source string */
    struct string       source;          /* source */
    struct string       version_str;     /* version string */
    struct string       version;         /* version */
    struct string       uptime_str;      /* uptime string */
    struct string       timestamp_str;   /* timestamp string */
    struct string       ntotal_conn_str; /* total connections string */
    struct string       ncurr_conn_str;  /* curr connections string */

    //stats_swap中置1  只有客户端发送命令来获取stats信息的时候在stats_aggregate统计完后置0，
    volatile int        aggregate;       /* shadow (b) aggregate? */
    //stats_server_to_metric中置1  只要有统计信息进行了更新这里就会置1
    volatile int        updated;         /* current (a) updated? */
};

#define DEFINE_ACTION(_name, _type, _desc) STATS_POOL_##_name,
typedef enum stats_pool_field {
    /*
    转换后该enum成员为 
    enum stats_pool_field {
        STATS_POOL_client_eof,
        STATS_POOL_client_err,
        ......
        STATS_POOL_out_queue_bytes,
        STATS_POOL_NFIELD
    }
    */
    STATS_POOL_CODEC(DEFINE_ACTION) 
    STATS_POOL_NFIELD
} stats_pool_field_t;
#undef DEFINE_ACTION

#define DEFINE_ACTION(_name, _type, _desc) STATS_SERVER_##_name,
typedef enum stats_server_field {
    /*
    转换后该enum成员为 
    enum stats_pool_field {
        STATS_SERVER_server_eof,
        STATS_SERVER_server_err,
        ......
        STATS_SERVER_out_queue_bytes,
        STATS_SERVER_NFIELD
    }
    */
    STATS_SERVER_CODEC(DEFINE_ACTION)
    STATS_SERVER_NFIELD
} stats_server_field_t;
#undef DEFINE_ACTION

#if defined NC_STATS && NC_STATS == 1

#define stats_pool_incr(_ctx, _pool, _name) do {                        \
    _stats_pool_incr(_ctx, _pool, STATS_POOL_##_name);                  \
} while (0)

#define stats_pool_decr(_ctx, _pool, _name) do {                        \
    _stats_pool_decr(_ctx, _pool, STATS_POOL_##_name);                  \
} while (0)

#define stats_pool_incr_by(_ctx, _pool, _name, _val) do {               \
    _stats_pool_incr_by(_ctx, _pool, STATS_POOL_##_name, _val);         \
} while (0)

#define stats_pool_decr_by(_ctx, _pool, _name, _val) do {               \
    _stats_pool_decr_by(_ctx, _pool, STATS_POOL_##_name, _val);         \
} while (0)

#define stats_pool_set_ts(_ctx, _pool, _name, _val) do {                \
    _stats_pool_set_ts(_ctx, _pool, STATS_POOL_##_name, _val);          \
} while (0)

#define stats_server_incr(_ctx, _server, _name) do {                    \
    _stats_server_incr(_ctx, _server, STATS_SERVER_##_name);            \
} while (0)

#define stats_server_decr(_ctx, _server, _name) do {                    \
    _stats_server_decr(_ctx, _server, STATS_SERVER_##_name);            \
} while (0)

#define stats_server_incr_by(_ctx, _server, _name, _val) do {           \
    _stats_server_incr_by(_ctx, _server, STATS_SERVER_##_name, _val);   \
} while (0)

#define stats_server_decr_by(_ctx, _server, _name, _val) do {           \
    _stats_server_decr_by(_ctx, _server, STATS_SERVER_##_name, _val);   \
} while (0)

#define stats_server_set_ts(_ctx, _server, _name, _val) do {            \
     _stats_server_set_ts(_ctx, _server, STATS_SERVER_##_name, _val);   \
} while (0)

#else

#define stats_pool_incr(_ctx, _pool, _name)

#define stats_pool_decr(_ctx, _pool, _name)

#define stats_pool_incr_by(_ctx, _pool, _name, _val)

#define stats_pool_decr_by(_ctx, _pool, _name, _val)

#define stats_server_incr(_ctx, _server, _name)

#define stats_server_decr(_ctx, _server, _name)

#define stats_server_incr_by(_ctx, _server, _name, _val)

#define stats_server_decr_by(_ctx, _server, _name, _val)

#endif

#define stats_enabled   NC_STATS

void stats_describe(void);

void _stats_pool_incr(struct context *ctx, struct server_pool *pool, stats_pool_field_t fidx);
void _stats_pool_decr(struct context *ctx, struct server_pool *pool, stats_pool_field_t fidx);
void _stats_pool_incr_by(struct context *ctx, struct server_pool *pool, stats_pool_field_t fidx, int64_t val);
void _stats_pool_decr_by(struct context *ctx, struct server_pool *pool, stats_pool_field_t fidx, int64_t val);
void _stats_pool_set_ts(struct context *ctx, struct server_pool *pool, stats_pool_field_t fidx, int64_t val);

void _stats_server_incr(struct context *ctx, struct server *server, stats_server_field_t fidx);
void _stats_server_decr(struct context *ctx, struct server *server, stats_server_field_t fidx);
void _stats_server_incr_by(struct context *ctx, struct server *server, stats_server_field_t fidx, int64_t val);
void _stats_server_decr_by(struct context *ctx, struct server *server, stats_server_field_t fidx, int64_t val);
void _stats_server_set_ts(struct context *ctx, struct server *server, stats_server_field_t fidx, int64_t val);

struct stats *stats_create(uint16_t stats_port, char *stats_ip, int stats_interval, char *source, struct array *server_pool);
void stats_destroy(struct stats *stats);
void stats_swap(struct stats *stats);

#endif
