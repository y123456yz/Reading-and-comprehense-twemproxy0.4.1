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

#ifndef _NC_CORE_H_
#define _NC_CORE_H_

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_DEBUG_LOG
# define NC_DEBUG_LOG 1
#endif

#ifdef HAVE_ASSERT_PANIC
# define NC_ASSERT_PANIC 1
#endif

#ifdef HAVE_ASSERT_LOG
# define NC_ASSERT_LOG 1
#endif

#ifdef HAVE_STATS
# define NC_STATS 1
#else
# define NC_STATS 0
#endif

#ifdef HAVE_EPOLL
# define NC_HAVE_EPOLL 1
#elif HAVE_KQUEUE
# define NC_HAVE_KQUEUE 1
#elif HAVE_EVENT_PORTS
# define NC_HAVE_EVENT_PORTS 1
#else
# error missing scalable I/O event notification mechanism
#endif

#ifdef HAVE_LITTLE_ENDIAN
# define NC_LITTLE_ENDIAN 1
#endif

#ifdef HAVE_BACKTRACE
# define NC_HAVE_BACKTRACE 1
#endif

#define NC_OK        0
#define NC_ERROR    -1
#define NC_EAGAIN   -2
#define NC_ENOMEM   -3

/* reserved fds for std streams, log, stats fd, epoll etc. */
#define RESERVED_FDS 32

typedef int rstatus_t; /* return type */
typedef int err_t;     /* error type */

struct array;
struct string;
struct context;
struct conn;
struct conn_tqh;
struct msg;
struct msg_tqh;
struct server;
struct server_pool;
struct mbuf;
struct mhdr;
struct conf;
struct stats;
struct instance;
struct event_base;

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>

#include <nc_array.h>
#include <nc_string.h>
#include <nc_queue.h>
#include <nc_rbtree.h>
#include <nc_log.h>
#include <nc_util.h>
#include <event/nc_event.h>
#include <nc_stats.h>
#include <nc_mbuf.h>
#include <nc_message.h>
#include <nc_connection.h>
#include <nc_server.h>

//配置相关的结构体路径instance->context->conf->conf_pool(conf_server)->server_pool(server)
//创建空间和初始化见core_ctx_create
struct context { //存放于instance->ctx    
    uint32_t           id;          /* unique context id */
    //存放配置文件中的配置信息 ctx->cf = conf_create(nci->conf_filename);  配置信息相关
    struct conf        *cf;         /* configuration */
    struct stats       *stats;      /* stats */
    //数组成员类型为server_pool，创建空间和赋值见server_pool_init
    struct array       pool;        /* server_pool[] */
    //创建空间和赋值见core_ctx_create->event_base_create   所有的连接信息都通过event_add_conn把conn加入到该evb中，可以参考proxy_accept
    struct event_base  *evb;        /* event base */
    //从instance取出，见core_ctx_create  数据来源为instance->stats_interval  最终用在epoll_wait的超时时间，见core_loop core_timeout
    int                max_timeout; /* max timeout in msec */
    //最终用在epoll_wait的超时时间，见core_loop  数据来源为instance->stats_interval  最终用在epoll_wait的超时时间，见core_loop
    int                timeout;     /* timeout in msec */ //也可能在core_timeout中重设置，设置为离当前最近的时间

    //总fd数目
    uint32_t           max_nfd;     /* max # files */
    //总的减去twemproxy bind的以及内部使用的文件fd，如log等  参考core_calc_connections
    uint32_t           max_ncconn;  /* max # client connections */
    //赋值见server_pool_each_calc_connections   和后端服务器总的连接数
    uint32_t           max_nsconn;  /* max # server connections */
};

//配置相关的结构体路径instance->context->conf->conf_pool(conf_server)->server_pool(server)

//所有配置信息、资源信息的源头就是该结构指向，最终源头在main中的struct instance nci;
struct instance { //全局通用配置，初始化在nc_set_default_options，所有内存信息的源头就在这里
    //赋值见core_start
    struct context  *ctx;                        /* active context */
    //值越大，打印的日志越多
    int             log_level;                   /* log level */ //最终赋值给了logger->level，见nc_pre_run
    char            *log_filename;               /* log filename */ //默认没有日志，通过运行程序的时候加上-O参数来设置，见nci->log_filename = optarg;
    //-c参数指定配置文件路径
    char            *conf_filename;              /* configuration filename */

    /*
     默认值
     STATS_ADDR      "0.0.0.0"   这里的IP地址和端口是为获取stats使用的
     STATS_PORT      22222
    */ //监听地址和端口 可以通过-s设置  注意这个和配置文件中的listen配置是不一样的，stats_port是通过-s参数在程序命令行中设置
    //最终被赋值给stats->port,见stats_create
    uint16_t        stats_port;                  /* stats monitoring port */
    //最终被赋值给stats->interval，见stats_create
    //最终被赋值给stats->interval，见stats_create  最终用在epoll_wait的超时时间，见core_loop
    int             stats_interval;              /* stats aggregation interval */
    //最终被赋值给stats->addr，见stats_create
    char            *stats_addr;                 /* stats monitoring addr */
    //通过nc_gethostname获取主机名
    char            hostname[NC_MAXHOSTNAMELEN]; /* hostname */ 
    /*
    read, writev and mbuf
   所有的请求和响应都在mbuf里面，mbuf默认大小是16K（512b-16M）,可以使用
-m or -mbuf-size=N来配置，每一个连接都会获得至少一个mbuf，这意味着nutcracker支持的
并发的连接数依赖于mbuf的大小，小的mbuf可以控制更多的连接，大的mbuf可以让我们
读或者写更多的数据导socker buffer。如果并发量很大的场景，推荐使用比较小的mbuf（512 or 1K）

  每一个客户端连接最好需要一个mbuf，一个服务请求最少是两个连接（client->proxy、proxy->server）所以最少需要两个mbufs
  1000个客户端连接的场景计算：1000*2*mbuf=32M,如果每个连接有10个操作，这个值将会是320M,假设连接是10000，那么将会
消耗3.2G内存！这种场景下最好调小mbuf值比如512b，1000*2*512*10=10M这个就是当并发量高的场景下使用小的mbuf的原因
    */ 
//注意连接上来后分配一个msg，该msg是不会释放的，会放入重用队列，所以该值在高并发条件下不能太多，实际消耗的内存为最大连接情况下的内存，即使连接断开，内存也不释放
    size_t          mbuf_chunk_size;             /* mbuf chunk size */ //mbuf大小  默认值MBUF_SIZE
    pid_t           pid;                         /* process id */ //进程号
    char            *pid_filename;               /* pid filename */ //-p参数指定
    //标识是否创建了pid文件
    unsigned        pidfile:1;                   /* pid file created? */
};

struct context *core_start(struct instance *nci);
void core_stop(struct context *ctx);
rstatus_t core_core(void *arg, uint32_t events);
rstatus_t core_loop(struct context *ctx);

#endif
