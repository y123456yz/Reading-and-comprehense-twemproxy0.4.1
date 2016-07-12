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

#ifndef _NC_MESSAGE_H_
#define _NC_MESSAGE_H_

#include <nc_core.h>

typedef void (*msg_parse_t)(struct msg *);
typedef rstatus_t (*msg_add_auth_t)(struct context *ctx, struct conn *c_conn, struct conn *s_conn);
typedef rstatus_t (*msg_fragment_t)(struct msg *, uint32_t, struct msg_tqh *);
typedef void (*msg_coalesce_t)(struct msg *r);
typedef rstatus_t (*msg_reply_t)(struct msg *r);
typedef bool (*msg_failure_t)(struct msg *r);

typedef enum msg_parse_result {
    MSG_PARSE_OK,                         /* parsing ok */
    MSG_PARSE_ERROR,                      /* parsing error */
    MSG_PARSE_REPAIR,                     /* more to parse -> repair parsed & unparsed data */
    MSG_PARSE_AGAIN,                      /* incomplete -> parse again */
} msg_parse_result_t;

#define MSG_TYPE_CODEC(ACTION)                                                                      \
    ACTION( UNKNOWN )                                                                               \
    ACTION( REQ_MC_GET )                       /* memcache retrieval requests */                    \
    ACTION( REQ_MC_GETS )                                                                           \
    ACTION( REQ_MC_DELETE )                    /* memcache delete request */                        \
    ACTION( REQ_MC_CAS )                       /* memcache cas request and storage request */       \
    ACTION( REQ_MC_SET )                       /* memcache storage request */                       \
    ACTION( REQ_MC_ADD )                                                                            \
    ACTION( REQ_MC_REPLACE )                                                                        \
    ACTION( REQ_MC_APPEND )                                                                         \
    ACTION( REQ_MC_PREPEND )                                                                        \
    ACTION( REQ_MC_INCR )                      /* memcache arithmetic request */                    \
    ACTION( REQ_MC_DECR )                                                                           \
    ACTION( REQ_MC_TOUCH )                     /* memcache touch request */                         \
    ACTION( REQ_MC_QUIT )                      /* memcache quit request */                          \
    ACTION( RSP_MC_NUM )                       /* memcache arithmetic response */                   \
    ACTION( RSP_MC_STORED )                    /* memcache cas and storage response */              \
    ACTION( RSP_MC_NOT_STORED )                                                                     \
    ACTION( RSP_MC_EXISTS )                                                                         \
    ACTION( RSP_MC_NOT_FOUND )                                                                      \
    ACTION( RSP_MC_END )                                                                            \
    ACTION( RSP_MC_VALUE )                                                                          \
    ACTION( RSP_MC_DELETED )                   /* memcache delete response */                       \
    ACTION( RSP_MC_TOUCHED )                   /* memcache touch response */                        \
    ACTION( RSP_MC_ERROR )                     /* memcache error responses */                       \
    ACTION( RSP_MC_CLIENT_ERROR )                                                                   \
    ACTION( RSP_MC_SERVER_ERROR )                                                                   \
    ACTION( REQ_REDIS_DEL )                    /* redis commands - keys */                          \
    ACTION( REQ_REDIS_EXISTS )                                                                      \
    ACTION( REQ_REDIS_EXPIRE )                                                                      \
    ACTION( REQ_REDIS_EXPIREAT )                                                                    \
    ACTION( REQ_REDIS_PEXPIRE )                                                                     \
    ACTION( REQ_REDIS_PEXPIREAT )                                                                   \
    ACTION( REQ_REDIS_PERSIST )                                                                     \
    ACTION( REQ_REDIS_PTTL )                                                                        \
    ACTION( REQ_REDIS_SORT )                                                                        \
    ACTION( REQ_REDIS_TTL )                                                                         \
    ACTION( REQ_REDIS_TYPE )                                                                        \
    ACTION( REQ_REDIS_APPEND )                 /* redis requests - string */                        \
    ACTION( REQ_REDIS_BITCOUNT )                                                                    \
    ACTION( REQ_REDIS_BITPOS )                                                                    \
    ACTION( REQ_REDIS_DECR )                                                                        \
    ACTION( REQ_REDIS_DECRBY )                                                                      \
    ACTION( REQ_REDIS_DUMP )                                                                        \
    ACTION( REQ_REDIS_GET )                                                                         \
    ACTION( REQ_REDIS_GETBIT )                                                                      \
    ACTION( REQ_REDIS_GETRANGE )                                                                    \
    ACTION( REQ_REDIS_GETSET )                                                                      \
    ACTION( REQ_REDIS_INCR )                                                                        \
    ACTION( REQ_REDIS_INCRBY )                                                                      \
    ACTION( REQ_REDIS_INCRBYFLOAT )                                                                 \
    ACTION( REQ_REDIS_MGET )                                                                        \
    ACTION( REQ_REDIS_MSET )                                                                        \
    ACTION( REQ_REDIS_PSETEX )                                                                      \
    ACTION( REQ_REDIS_RESTORE )                                                                     \
    ACTION( REQ_REDIS_SET )                                                                         \
    ACTION( REQ_REDIS_SETBIT )                                                                      \
    ACTION( REQ_REDIS_SETEX )                                                                       \
    ACTION( REQ_REDIS_SETNX )                                                                       \
    ACTION( REQ_REDIS_SETRANGE )                                                                    \
    ACTION( REQ_REDIS_STRLEN )                                                                      \
    ACTION( REQ_REDIS_HDEL )                   /* redis requests - hashes */                        \
    ACTION( REQ_REDIS_HEXISTS )                                                                     \
    ACTION( REQ_REDIS_HGET )                                                                        \
    ACTION( REQ_REDIS_HGETALL )                                                                     \
    ACTION( REQ_REDIS_HINCRBY )                                                                     \
    ACTION( REQ_REDIS_HINCRBYFLOAT )                                                                \
    ACTION( REQ_REDIS_HKEYS )                                                                       \
    ACTION( REQ_REDIS_HLEN )                                                                        \
    ACTION( REQ_REDIS_HMGET )                                                                       \
    ACTION( REQ_REDIS_HMSET )                                                                       \
    ACTION( REQ_REDIS_HSET )                                                                        \
    ACTION( REQ_REDIS_HSETNX )                                                                      \
    ACTION( REQ_REDIS_HSCAN)                                                                        \
    ACTION( REQ_REDIS_HVALS )                                                                       \
    ACTION( REQ_REDIS_LINDEX )                 /* redis requests - lists */                         \
    ACTION( REQ_REDIS_LINSERT )                                                                     \
    ACTION( REQ_REDIS_LLEN )                                                                        \
    ACTION( REQ_REDIS_LPOP )                                                                        \
    ACTION( REQ_REDIS_LPUSH )                                                                       \
    ACTION( REQ_REDIS_LPUSHX )                                                                      \
    ACTION( REQ_REDIS_LRANGE )                                                                      \
    ACTION( REQ_REDIS_LREM )                                                                        \
    ACTION( REQ_REDIS_LSET )                                                                        \
    ACTION( REQ_REDIS_LTRIM )                                                                       \
    ACTION( REQ_REDIS_PFADD )                  /* redis requests - hyperloglog */                   \
    ACTION( REQ_REDIS_PFCOUNT )                                                                     \
    ACTION( REQ_REDIS_PFMERGE )                                                                     \
    ACTION( REQ_REDIS_RPOP )                                                                        \
    ACTION( REQ_REDIS_RPOPLPUSH )                                                                   \
    ACTION( REQ_REDIS_RPUSH )                                                                       \
    ACTION( REQ_REDIS_RPUSHX )                                                                      \
    ACTION( REQ_REDIS_SADD )                   /* redis requests - sets */                          \
    ACTION( REQ_REDIS_SCARD )                                                                       \
    ACTION( REQ_REDIS_SDIFF )                                                                       \
    ACTION( REQ_REDIS_SDIFFSTORE )                                                                  \
    ACTION( REQ_REDIS_SINTER )                                                                      \
    ACTION( REQ_REDIS_SINTERSTORE )                                                                 \
    ACTION( REQ_REDIS_SISMEMBER )                                                                   \
    ACTION( REQ_REDIS_SMEMBERS )                                                                    \
    ACTION( REQ_REDIS_SMOVE )                                                                       \
    ACTION( REQ_REDIS_SPOP )                                                                        \
    ACTION( REQ_REDIS_SRANDMEMBER )                                                                 \
    ACTION( REQ_REDIS_SREM )                                                                        \
    ACTION( REQ_REDIS_SUNION )                                                                      \
    ACTION( REQ_REDIS_SUNIONSTORE )                                                                 \
    ACTION( REQ_REDIS_SSCAN)                                                                        \
    ACTION( REQ_REDIS_ZADD )                   /* redis requests - sorted sets */                   \
    ACTION( REQ_REDIS_ZCARD )                                                                       \
    ACTION( REQ_REDIS_ZCOUNT )                                                                      \
    ACTION( REQ_REDIS_ZINCRBY )                                                                     \
    ACTION( REQ_REDIS_ZINTERSTORE )                                                                 \
    ACTION( REQ_REDIS_ZLEXCOUNT )                                                                   \
    ACTION( REQ_REDIS_ZRANGE )                                                                      \
    ACTION( REQ_REDIS_ZRANGEBYLEX )                                                                 \
    ACTION( REQ_REDIS_ZRANGEBYSCORE )                                                               \
    ACTION( REQ_REDIS_ZRANK )                                                                       \
    ACTION( REQ_REDIS_ZREM )                                                                        \
    ACTION( REQ_REDIS_ZREMRANGEBYRANK )                                                             \
    ACTION( REQ_REDIS_ZREMRANGEBYLEX )                                                              \
    ACTION( REQ_REDIS_ZREMRANGEBYSCORE )                                                            \
    ACTION( REQ_REDIS_ZREVRANGE )                                                                   \
    ACTION( REQ_REDIS_ZREVRANGEBYSCORE )                                                            \
    ACTION( REQ_REDIS_ZREVRANK )                                                                    \
    ACTION( REQ_REDIS_ZSCORE )                                                                      \
    ACTION( REQ_REDIS_ZUNIONSTORE )                                                                 \
    ACTION( REQ_REDIS_ZSCAN)                                                                        \
    ACTION( REQ_REDIS_EVAL )                   /* redis requests - eval */                          \
    ACTION( REQ_REDIS_EVALSHA )                                                                     \
    ACTION( REQ_REDIS_PING )                   /* redis requests - ping/quit */                     \
    ACTION( REQ_REDIS_QUIT)                                                                         \
    ACTION( REQ_REDIS_AUTH)                                                                         \
    ACTION( REQ_REDIS_SELECT)                  /* only during init */                               \
    ACTION( RSP_REDIS_STATUS )                 /* redis response */                                 \
    ACTION( RSP_REDIS_ERROR )                                                                       \
    ACTION( RSP_REDIS_ERROR_ERR )                                                                   \
    ACTION( RSP_REDIS_ERROR_OOM )                                                                   \
    ACTION( RSP_REDIS_ERROR_BUSY )                                                                  \
    ACTION( RSP_REDIS_ERROR_NOAUTH )                                                                \
    ACTION( RSP_REDIS_ERROR_LOADING )                                                               \
    ACTION( RSP_REDIS_ERROR_BUSYKEY )                                                               \
    ACTION( RSP_REDIS_ERROR_MISCONF )                                                               \
    ACTION( RSP_REDIS_ERROR_NOSCRIPT )                                                              \
    ACTION( RSP_REDIS_ERROR_READONLY )                                                              \
    ACTION( RSP_REDIS_ERROR_WRONGTYPE )                                                             \
    ACTION( RSP_REDIS_ERROR_EXECABORT )                                                             \
    ACTION( RSP_REDIS_ERROR_MASTERDOWN )                                                            \
    ACTION( RSP_REDIS_ERROR_NOREPLICAS )                                                            \
    ACTION( RSP_REDIS_INTEGER )                                                                     \
    ACTION( RSP_REDIS_BULK )                                                                        \
    ACTION( RSP_REDIS_MULTIBULK )                                                                   \
    ACTION( SENTINEL )                                                                              \


#define DEFINE_ACTION(_name) MSG_##_name,
typedef enum msg_type {
    MSG_TYPE_CODEC(DEFINE_ACTION)
} msg_type_t;
#undef DEFINE_ACTION

struct keypos {
    uint8_t             *start;           /* key start pos */
    uint8_t             *end;             /* key end pos */
};

//msg创建空间和赋值见msg_get
struct msg {
    TAILQ_ENTRY(msg)     c_tqe;           /* link in client q */
    TAILQ_ENTRY(msg)     s_tqe;           /* link in server q */
    TAILQ_ENTRY(msg)     m_tqe;           /* link in send q / free q */

    uint64_t             id;              /* message id */ //每次msg_get自增
    struct msg           *peer;           /* message peer */
    //所属的conn,见msg_get
    struct conn          *owner;          /* message owner - client | server */

    //通过该成员加入到红黑树tmo_rbt
    struct rbnode        tmo_rbe;         /* entry in rbtree */

    //mhdr和mbuf的关系参考mbuf_insert  mlen为mbuf的数据长度
    //该链中存储的是接收数据用的mbuf，有可能数据很大，一个mbuf不够用，所以会有多个mbuf添加到该mhdr链中
    struct mhdr          mhdr;            /* message mbuf header */
    uint32_t             mlen;            /* message length */
    int64_t              start_ts;        /* request start timestamp in usec */

    int                  state;           /* current parser state */
    //执行mbuf->pos，见msg_parsed
    uint8_t              *pos;            /* parser position marker */
    uint8_t              *token;          /* token marker */


    /*
    if (request) {
        msg->parser = memcache_parse_req;
    } else {
        msg->parser = memcache_parse_rsp;
    }
    msg->add_auth = memcache_add_auth;
    msg->fragment = memcache_fragment;
    msg->failure = memcache_failure;
    msg->pre_coalesce = memcache_pre_coalesce;
    msg->post_coalesce = memcache_post_coalesce;
    */

    //redis_parse_req  redis_parse_rsp  memcache_parse_req  memcache_parse_rsp
    msg_parse_t          parser;          /* message parser */ //msg_parse中执行
    //解析客户端发送过来的报文内容，成功MSG_PARSE_OK
    msg_parse_result_t   result;          /* message parsing result */
    // memcache_fragment  redis_fragment  在req_recv_done中执行        计算分片
    //分片  mget mset等批处理命令中的不同KV可能分布在后端不同服务器上因此需要拆分  mget gets涉及到分发与合并，参考http://www.codesec.net/view/218217.html
    msg_fragment_t       fragment;        /* message fragment */
    //redis_reply或者memcache_reply
    msg_reply_t          reply;           /* generate message reply (example: ping) */
    //redis_add_auth  memcache_add_auth      req_forward中执行
    msg_add_auth_t       add_auth;        /* add auth message when we forward msg */
    msg_failure_t        failure;         /* transient failure response? */

    //memcache_pre_coalesce   redis_pre_coalesce
    msg_coalesce_t       pre_coalesce;    /* message pre-coalesce */
    //memcache_post_coalesce  redis_post_coalesce
    msg_coalesce_t       post_coalesce;   /* message post-coalesce */

    //type类型见msg_type_strings
    msg_type_t           type;            /* message type */

    //数组成员类型为keypos  memcache_parse_req  redis_parse_req解析的命令行放入该数组中
    struct array         *keys;           /* array of keypos, for req */ //set key value中的key放入该数组

    uint32_t             vlen;            /* value length (memcache) */
    uint8_t              *end;            /* end marker (memcache) */

    uint8_t              *narg_start;     /* narg start (redis) */
    uint8_t              *narg_end;       /* narg end (redis) */
    uint32_t             narg;            /* # arguments (redis) */
    uint32_t             rnarg;           /* running # arg used by parsing fsa (redis) */
    uint32_t             rlen;            /* running length in parsing fsa (redis) */
    uint32_t             integer;         /* integer reply value (redis) */

    struct msg           *frag_owner;     /* owner of fragment message */
    uint32_t             nfrag;           /* # fragment */
    uint32_t             nfrag_done;      /* # fragment done */
    //msg_gen_frag_id产生
    uint64_t             frag_id;         /* id of fragmented message */
    //该数组里面记录了指向同一个后端服务器的msg信息
    struct msg           **frag_seq;      /* sequence of fragment message, map from keys to fragments*/

    err_t                err;             /* errno on error? */ //获取网络异常的错误号
    unsigned             error:1;         /* error? */ //异常
    unsigned             ferror:1;        /* one or more fragments are in error? */
    //是否是客户端请求对应的msg
    unsigned             request:1;       /* request? or response? */ //请求还是应答
    //解析到客户端发送过来的字符串为quit，则置1
    unsigned             quit:1;          /* quit request? */
    //只有在memcache_parse_req会置1，只有客户端发送过来带有noreply的时候才置1
    unsigned             noreply:1;       /* noreply? */ //该msg需要得到应答，添加到红黑树超时定时器，参考req_server_enqueue_imsgq
    //如果和后端配置有认证过程，但是却没有认证成功，则这里置1，见req_filter
    unsigned             noforward:1;     /* not need forward (example: ping) */ 
    unsigned             done:1;          /* done? */ //标记该msg是否得到了后端的应答，见rsp_forward
    unsigned             fdone:1;         /* all fragments are done? */ //分片到后端多个服务器的请求是否得到了应答
    unsigned             swallow:1;       /* swallow response? */
    //后端是否redis服务器
    unsigned             redis:1;         /* redis? */
};

TAILQ_HEAD(msg_tqh, msg);

struct msg *msg_tmo_min(void);
void msg_tmo_insert(struct msg *msg, struct conn *conn);
void msg_tmo_delete(struct msg *msg);

void msg_init(void);
void msg_deinit(void);
struct string *msg_type_string(msg_type_t type);
struct msg *msg_get(struct conn *conn, bool request, bool redis);
void msg_put(struct msg *msg);
struct msg *msg_get_error(bool redis, err_t err);
void msg_dump(struct msg *msg, int level);
bool msg_empty(struct msg *msg);
rstatus_t msg_recv(struct context *ctx, struct conn *conn);
rstatus_t msg_send(struct context *ctx, struct conn *conn);
uint64_t msg_gen_frag_id(void);
uint32_t msg_backend_idx(struct msg *msg, uint8_t *key, uint32_t keylen);
struct mbuf *msg_ensure_mbuf(struct msg *msg, size_t len);
rstatus_t msg_append(struct msg *msg, uint8_t *pos, size_t n);
rstatus_t msg_prepend(struct msg *msg, uint8_t *pos, size_t n);
rstatus_t msg_prepend_format(struct msg *msg, const char *fmt, ...);

struct msg *req_get(struct conn *conn);
void req_put(struct msg *msg);
bool req_done(struct conn *conn, struct msg *msg);
bool req_error(struct conn *conn, struct msg *msg);
void req_server_enqueue_imsgq(struct context *ctx, struct conn *conn, struct msg *msg);
void req_server_enqueue_imsgq_head(struct context *ctx, struct conn *conn, struct msg *msg);
void req_server_dequeue_imsgq(struct context *ctx, struct conn *conn, struct msg *msg);
void req_client_enqueue_omsgq(struct context *ctx, struct conn *conn, struct msg *msg);
void req_server_enqueue_omsgq(struct context *ctx, struct conn *conn, struct msg *msg);
void req_client_dequeue_omsgq(struct context *ctx, struct conn *conn, struct msg *msg);
void req_server_dequeue_omsgq(struct context *ctx, struct conn *conn, struct msg *msg);
struct msg *req_recv_next(struct context *ctx, struct conn *conn, bool alloc);
void req_recv_done(struct context *ctx, struct conn *conn, struct msg *msg, struct msg *nmsg);
struct msg *req_send_next(struct context *ctx, struct conn *conn);
void req_send_done(struct context *ctx, struct conn *conn, struct msg *msg);

struct msg *rsp_get(struct conn *conn);
void rsp_put(struct msg *msg);
struct msg *rsp_recv_next(struct context *ctx, struct conn *conn, bool alloc);
void rsp_recv_done(struct context *ctx, struct conn *conn, struct msg *msg, struct msg *nmsg);
struct msg *rsp_send_next(struct context *ctx, struct conn *conn);
void rsp_send_done(struct context *ctx, struct conn *conn, struct msg *msg);

#endif
