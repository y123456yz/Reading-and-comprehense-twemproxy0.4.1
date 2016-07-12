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

#include <nc_core.h>
#include <nc_server.h>

//获取一个msg结构
struct msg *
req_get(struct conn *conn)
{
    struct msg *msg;

    ASSERT(conn->client && !conn->proxy);

    msg = msg_get(conn, true, conn->redis);
    if (msg == NULL) {
        conn->err = errno;
    }
    return msg;
}

//客户端请求日志打印
static void
req_log(struct msg *req)
{
    struct msg *rsp;           /* peer message (response) */
    int64_t req_time;          /* time cost for this request */
    char *peer_str;            /* peer client ip:port */
    uint32_t req_len, rsp_len; /* request and response length */
    struct string *req_type;   /* request type string */
    struct keypos *kpos;

    if (log_loggable(LOG_NOTICE) == 0) {
        return;
    }

    /* a fragment? */
    if (req->frag_id != 0 && req->frag_owner != req) {
        return;
    }

    /* conn close normally? */
    if (req->mlen == 0) {
        return;
    }
    /*
     * there is a race scenario where a requests comes in, the log level is not LOG_NOTICE,
     * and before the response arrives you modify the log level to LOG_NOTICE
     * using SIGTTIN OR SIGTTOU, then req_log() wouldn't have msg->start_ts set
     */
    if (req->start_ts == 0) {
        return;
    }

    req_time = nc_usec_now() - req->start_ts;

    rsp = req->peer;
    req_len = req->mlen;
    rsp_len = (rsp != NULL) ? rsp->mlen : 0;

    if (array_n(req->keys) < 1) {
        return;
    }

    kpos = array_get(req->keys, 0);
    if (kpos->end != NULL) {
        *(kpos->end) = '\0';
    }

    /*
     * FIXME: add backend addr here
     * Maybe we can store addrstr just like server_pool in conn struct
     * when connections are resolved
     */
    peer_str = nc_unresolve_peer_desc(req->owner->sd);

    req_type = msg_type_string(req->type);

     if(req_time < 10000) //10ms
        return;
    
    log_debug(LOG_NOTICE, "req %"PRIu64" done on c %d req_time %"PRIi64".%03"PRIi64
              " msec type %.*s narg %"PRIu32" req_len %"PRIu32" rsp_len %"PRIu32
              " key0 '%s' peer '%s' done %d error %d",
              req->id, req->owner->sd, req_time / 1000, req_time % 1000,
              req_type->len, req_type->data, req->narg, req_len, rsp_len,
              kpos->start, peer_str, req->done, req->error);
}

void
req_put(struct msg *msg)
{
    struct msg *pmsg; /* peer message (response) */

    ASSERT(msg->request);

    req_log(msg);

    pmsg = msg->peer;
    if (pmsg != NULL) {
        ASSERT(!pmsg->request && pmsg->peer == msg);
        msg->peer = NULL;
        pmsg->peer = NULL;
        rsp_put(pmsg);
    }

    msg_tmo_delete(msg);

    msg_put(msg);
}

/*
 * Return true if request is done, false otherwise
 *
 * A request is done, if we received response for the given request.
 * A request vector is done if we received responses for all its
 * fragments.
 */
bool
req_done(struct conn *conn, struct msg *msg)
{
    struct msg *cmsg, *pmsg; /* current and previous message */
    uint64_t id;             /* fragment id */
    uint32_t nfragment;      /* # fragment */

    ASSERT(conn->client && !conn->proxy);
    ASSERT(msg->request);

    if (!msg->done) {
        return false;
    }

    id = msg->frag_id;
    if (id == 0) {
        return true;
    }

    if (msg->fdone) {
        /* request has already been marked as done */
        return true;
    }

    if (msg->nfrag_done < msg->nfrag) {
        return false;
    }

    /* check all fragments of the given request vector are done */

    for (pmsg = msg, cmsg = TAILQ_PREV(msg, msg_tqh, c_tqe);
         cmsg != NULL && cmsg->frag_id == id;
         pmsg = cmsg, cmsg = TAILQ_PREV(cmsg, msg_tqh, c_tqe)) {

        if (!cmsg->done) {
            return false;
        }
    }

    for (pmsg = msg, cmsg = TAILQ_NEXT(msg, c_tqe);
         cmsg != NULL && cmsg->frag_id == id;
         pmsg = cmsg, cmsg = TAILQ_NEXT(cmsg, c_tqe)) {

        if (!cmsg->done) {
            return false;
        }
    }

    /*
     * At this point, all the fragments including the last fragment have
     * been received.
     *
     * Mark all fragments of the given request vector to be done to speed up
     * future req_done calls for any of fragments of this request
     */

    msg->fdone = 1;
    nfragment = 0;

    for (pmsg = msg, cmsg = TAILQ_PREV(msg, msg_tqh, c_tqe);
         cmsg != NULL && cmsg->frag_id == id;
         pmsg = cmsg, cmsg = TAILQ_PREV(cmsg, msg_tqh, c_tqe)) {
        cmsg->fdone = 1;
        nfragment++;
    }

    for (pmsg = msg, cmsg = TAILQ_NEXT(msg, c_tqe);
         cmsg != NULL && cmsg->frag_id == id;
         pmsg = cmsg, cmsg = TAILQ_NEXT(cmsg, c_tqe)) {
        cmsg->fdone = 1;
        nfragment++;
    }

    ASSERT(msg->frag_owner->nfrag == nfragment);

    msg->post_coalesce(msg->frag_owner);

    log_debug(LOG_DEBUG, "req from c %d with fid %"PRIu64" and %"PRIu32" "
              "fragments is done", conn->sd, id, nfragment);

    return true;
}


/*
 * Return true if request is in error, false otherwise
 *
 * A request is in error, if there was an error in receiving response for the
 * given request. A multiget request is in error if there was an error in
 * receiving response for any its fragments.
 */
bool
req_error(struct conn *conn, struct msg *msg)
{
    struct msg *cmsg; /* current message */
    uint64_t id;
    uint32_t nfragment;

    ASSERT(msg->request && req_done(conn, msg));

    if (msg->error) { //返回的信息是错误信息
        return true;
    }

    id = msg->frag_id;
    if (id == 0) {
        return false;
    }

    if (msg->ferror) {
        /* request has already been marked to be in error */
        return true;
    }

    /* check if any of the fragments of the given request are in error */

    for (cmsg = TAILQ_PREV(msg, msg_tqh, c_tqe);
         cmsg != NULL && cmsg->frag_id == id;
         cmsg = TAILQ_PREV(cmsg, msg_tqh, c_tqe)) {

        if (cmsg->error) {
            goto ferror;
        }
    }

    for (cmsg = TAILQ_NEXT(msg, c_tqe);
         cmsg != NULL && cmsg->frag_id == id;
         cmsg = TAILQ_NEXT(cmsg, c_tqe)) {

        if (cmsg->error) {
            goto ferror;
        }
    }

    return false;

ferror:

    /*
     * Mark all fragments of the given request to be in error to speed up
     * future req_error calls for any of fragments of this request
     */

    msg->ferror = 1;
    nfragment = 1;

    for (cmsg = TAILQ_PREV(msg, msg_tqh, c_tqe);
         cmsg != NULL && cmsg->frag_id == id;
         cmsg = TAILQ_PREV(cmsg, msg_tqh, c_tqe)) {
        cmsg->ferror = 1;
        nfragment++;
    }

    for (cmsg = TAILQ_NEXT(msg, c_tqe);
         cmsg != NULL && cmsg->frag_id == id;
         cmsg = TAILQ_NEXT(cmsg, c_tqe)) {
        cmsg->ferror = 1;
        nfragment++;
    }

    log_debug(LOG_DEBUG, "req from c %d with fid %"PRIu64" and %"PRIu32" "
              "fragments is in error", conn->sd, id, nfragment);

    return true;
}

/* enqueue_inq用来把接收到的客户端KV信息msg入队到c_conn->imsg_q，当该msg发送到后端服务器后，dequeue_inq把该msg从c_conn->imsg_q中摘除掉
 然后分别在req_forward和req_send_done通过enqueue_outq把该msg入队到c_conn->omsg_q和s_conn->omsg_q中等待后端应答该msg对应的ack，
 后端应答后会创建一个新的msg接收后端应答信息，再通过rsp_forward和rsp_send_done分别把dequeue_outq把该msg摘除，然后通过msg_put
 归还给free_msg。同时在rsp_forward中把后端应答的msg和读取客户端信息的msg关联起来。从而可以把后端数据发送给客户端
 
 因此统计信息中的in_queue代表的就是imsg_q中的msg信息，表示还没有发送到后端服务器的msg信息。统计信息中的out_queue代表的就是
 omsg_q中的msg信息，表示msg已经发送到后端服务器但是还没有得到后端服务器应答信息的msg, 
 */


//接收的客户端msg信息通过req_server_enqueue_imsgq添加到该队列 在req_send_next中发往后端真实服务器
//msg数据添加到conn->imsg_q队列入队，在req_send_next中发往后端真实服务器
void   //req_server_enqueue_imsgq添加到队列尾部，req_server_enqueue_imsgq_head添加到队列头部
req_server_enqueue_imsgq(struct context *ctx, struct conn *conn, struct msg *msg)
{//req_forward中执行  req_forward->req_server_enqueue_imsgq
    ASSERT(msg->request);
    ASSERT(!conn->client && !conn->proxy);

    /*
     * timeout clock starts ticking the instant the message is enqueued into
     * the server in_q; the clock continues to tick until it either expires
     * or the message is dequeued from the server out_q
     *
     * noreply request are free from timeouts because client is not intrested
     * in the response anyway!
     */
    if (!msg->noreply) { //该msg需要得到应答，添加到红黑树超时定时器
        //该定时器在core_timeout中删除
        msg_tmo_insert(msg, conn); //添加到tmo_rbt   该msg需要发往后端真实服务器，并且需要等待对方应答
    }

    TAILQ_INSERT_TAIL(&conn->imsg_q, msg, s_tqe);//在core_core中的写事件把imsg_q中的msg发送出去

    stats_server_incr(ctx, conn->owner, in_queue); 
    stats_server_incr_by(ctx, conn->owner, in_queue_bytes, msg->mlen);
}

/* enqueue_inq用来把接收到的客户端KV信息msg入队到c_conn->imsg_q，当该msg发送到后端服务器后，dequeue_inq把该msg从c_conn->imsg_q中摘除掉
 然后分别在req_forward和req_send_done通过enqueue_outq把该msg入队到c_conn->omsg_q和s_conn->omsg_q中等待后端应答该msg对应的ack，
 后端应答后会创建一个新的msg接收后端应答信息，再通过rsp_forward和rsp_send_done分别把dequeue_outq把该msg摘除，然后通过msg_put
 归还给free_msg。同时在rsp_forward中把后端应答的msg和读取客户端信息的msg关联起来。从而可以把后端数据发送给客户端
 
 因此统计信息中的in_queue代表的就是imsg_q中的msg信息，表示还没有发送到后端服务器的msg信息。统计信息中的out_queue代表的就是
 omsg_q中的msg信息，表示msg已经发送到后端服务器但是还没有得到后端服务器应答信息的msg, 
 */

void  ////req_server_enqueue_imsgq添加到队列尾部，req_server_enqueue_imsgq_head添加到队列头部
req_server_enqueue_imsgq_head(struct context *ctx, struct conn *conn, struct msg *msg)
{
    ASSERT(msg->request);
    ASSERT(!conn->client && !conn->proxy);

    /*
     * timeout clock starts ticking the instant the message is enqueued into
     * the server in_q; the clock continues to tick until it either expires
     * or the message is dequeued from the server out_q
     *
     * noreply request are free from timeouts because client is not intrested
     * in the response anyway!
     */
    if (!msg->noreply) {
        msg_tmo_insert(msg, conn);
    }

    TAILQ_INSERT_HEAD(&conn->imsg_q, msg, s_tqe);

    stats_server_incr(ctx, conn->owner, in_queue);
    stats_server_incr_by(ctx, conn->owner, in_queue_bytes, msg->mlen);
}


/* enqueue_inq用来把接收到的客户端KV信息msg入队到c_conn->imsg_q，当该msg发送到后端服务器后，dequeue_inq把该msg从c_conn->imsg_q中摘除掉
 然后分别在req_forward和req_send_done通过enqueue_outq把该msg入队到c_conn->omsg_q和s_conn->omsg_q中等待后端应答该msg对应的ack，
 后端应答后会创建一个新的msg接收后端应答信息，再通过rsp_forward和rsp_send_done分别把dequeue_outq把该msg摘除，然后通过msg_put
 归还给free_msg。同时在rsp_forward中把后端应答的msg和读取客户端信息的msg关联起来。从而可以把后端数据发送给客户端
 
 因此统计信息中的in_queue代表的就是imsg_q中的msg信息，表示还没有发送到后端服务器的msg信息。统计信息中的out_queue代表的就是
 omsg_q中的msg信息，表示msg已经发送到后端服务器但是还没有得到后端服务器应答信息的msg, 
 */

void
req_server_dequeue_imsgq(struct context *ctx, struct conn *conn, struct msg *msg)
{//req_send_done中执行  把msg从imsg_q队列中脱离开来，不在是imsg_q队列中的成员。表示该msg已经发送给了后端服务器
    ASSERT(msg->request);
    ASSERT(!conn->client && !conn->proxy);

    TAILQ_REMOVE(&conn->imsg_q, msg, s_tqe);

    stats_server_decr(ctx, conn->owner, in_queue);
    stats_server_decr_by(ctx, conn->owner, in_queue_bytes, msg->mlen);
}
/* enqueue_inq用来把接收到的客户端KV信息msg入队到c_conn->imsg_q，当该msg发送到后端服务器后，dequeue_inq把该msg从c_conn->imsg_q中摘除掉
 然后分别在req_forward和req_send_done通过enqueue_outq把该msg入队到c_conn->omsg_q和s_conn->omsg_q中等待后端应答该msg对应的ack，
 后端应答后会创建一个新的msg接收后端应答信息，再通过rsp_forward和rsp_send_done分别把dequeue_outq把该msg摘除，然后通过msg_put
 归还给free_msg。同时在rsp_forward中把后端应答的msg和读取客户端信息的msg关联起来。从而可以把后端数据发送给客户端
 
 因此统计信息中的in_queue代表的就是imsg_q中的msg信息，表示还没有发送到后端服务器的msg信息。统计信息中的out_queue代表的就是
 omsg_q中的msg信息，表示msg已经发送到后端服务器但是还没有得到后端服务器应答信息的msg, 
 */


void
req_client_enqueue_omsgq(struct context *ctx, struct conn *conn, struct msg *msg)
{
    ASSERT(msg->request);
    ASSERT(conn->client && !conn->proxy);

    TAILQ_INSERT_TAIL(&conn->omsg_q, msg, c_tqe);
}
/* enqueue_inq用来把接收到的客户端KV信息msg入队到c_conn->imsg_q，当该msg发送到后端服务器后，dequeue_inq把该msg从c_conn->imsg_q中摘除掉
 然后分别在req_forward和req_send_done通过enqueue_outq把该msg入队到c_conn->omsg_q和s_conn->omsg_q中等待后端应答该msg对应的ack，
 后端应答后会创建一个新的msg接收后端应答信息，再通过rsp_forward和rsp_send_done分别把dequeue_outq把该msg摘除，然后通过msg_put
 归还给free_msg。同时在rsp_forward中把后端应答的msg和读取客户端信息的msg关联起来。从而可以把后端数据发送给客户端
 
 因此统计信息中的in_queue代表的就是imsg_q中的msg信息，表示还没有发送到后端服务器的msg信息。统计信息中的out_queue代表的就是
 omsg_q中的msg信息，表示msg已经发送到后端服务器但是还没有得到后端服务器应答信息的msg, 
 */

void
req_server_enqueue_omsgq(struct context *ctx, struct conn *conn, struct msg *msg)
{
    ASSERT(msg->request);
    ASSERT(!conn->client && !conn->proxy);

    TAILQ_INSERT_TAIL(&conn->omsg_q, msg, s_tqe);

    stats_server_incr(ctx, conn->owner, out_queue);
    stats_server_incr_by(ctx, conn->owner, out_queue_bytes, msg->mlen);
}

void
req_client_dequeue_omsgq(struct context *ctx, struct conn *conn, struct msg *msg)
{
    ASSERT(msg->request);
    ASSERT(conn->client && !conn->proxy);

    TAILQ_REMOVE(&conn->omsg_q, msg, c_tqe);
}

void
req_server_dequeue_omsgq(struct context *ctx, struct conn *conn, struct msg *msg)
{
    ASSERT(msg->request);
    ASSERT(!conn->client && !conn->proxy);

    msg_tmo_delete(msg);

    TAILQ_REMOVE(&conn->omsg_q, msg, s_tqe);

    stats_server_decr(ctx, conn->owner, out_queue);
    stats_server_decr_by(ctx, conn->owner, out_queue_bytes, msg->mlen);
}
/*
*             Client+             Proxy           Server+
*                              (nutcracker)
*                                   .
*       msg_recv {read event}       .       msg_recv {read event}
*         +                         .                         +
*         |                         .                         |
*         \                         .                         /
*         req_recv_next             .             rsp_recv_next
*           +                       .                       +
*           |                       .                       |       Rsp
*           req_recv_done           .           rsp_recv_done      <===
*             +                     .                     +
*             |                     .                     |
*    Req      \                     .                     /
*    ===>     req_filter*           .           *rsp_filter
*               +                   .                   +
*               |                   .                   |
*               \                   .                   /
*               req_forward-//  (a) . (c)  \\-rsp_forward
*                                   .
*                                   .
*       msg_send {write event}      .      msg_send {write event}
*         +                         .                         +
*         |                         .                         |
*    Rsp' \                         .                         /     Req'
*   <===  rsp_send_next             .             req_send_next     ===>
*           +                       .                       +
*           |                       .                       |
*           \                       .                       /
*           rsp_send_done-//    (d) . (b)    //-req_send_done
*
*
* (a) -> (b) -> (c) -> (d) is the normal flow of transaction consisting
* of a single request response, where (a) and (b) handle request from
* client, while (c) and (d) handle the corresponding response from the
* server.
*/

//msg_recv中执行，获取一个msg结构
struct msg *
req_recv_next(struct context *ctx, struct conn *conn, bool alloc)
{
    struct msg *msg;

    ASSERT(conn->client && !conn->proxy);

    if (conn->eof) {
        msg = conn->rmsg;

        /* client sent eof before sending the entire request */
        if (msg != NULL) {
            conn->rmsg = NULL;

            ASSERT(msg->peer == NULL);
            ASSERT(msg->request && !msg->done);

            log_error("eof c %d discarding incomplete req %"PRIu64" len "
                      "%"PRIu32"", conn->sd, msg->id, msg->mlen);

            req_put(msg);
        }

        /*
         * TCP half-close enables the client to terminate its half of the
         * connection (i.e. the client no longer sends data), but it still
         * is able to receive data from the proxy. The proxy closes its
         * half (by sending the second FIN) when the client has no
         * outstanding requests
         */
        if (!conn->active(conn)) {
            conn->done = 1;
            log_debug(LOG_INFO, "c %d is done", conn->sd);
        }
        return NULL;
    }

    msg = conn->rmsg;
    if (msg != NULL) {
        ASSERT(msg->request);
        return msg;
    }

    if (!alloc) {
        return NULL;
    }

    msg = req_get(conn);
    if (msg != NULL) {
        conn->rmsg = msg;
    }

    return msg;
}

//对客户端进行应答
static rstatus_t
req_make_reply(struct context *ctx, struct conn *conn, struct msg *req) 
//req为接收客户端数据的时候分配的msg，rsp为客户端应答使用的msg
{
    struct msg *rsp;

    rsp = msg_get(conn, false, conn->redis); /* replay */
    if (rsp == NULL) {
        conn->err = errno;
        return NC_ENOMEM;
    }

    req->peer = rsp;
    rsp->peer = req;
    rsp->request = 0;

    req->done = 1;
    conn->enqueue_outq(ctx, conn, req);

    return NC_OK;
}

/*
*             Client+             Proxy           Server+
*                              (nutcracker)
*                                   .
*       msg_recv {read event}       .       msg_recv {read event}
*         +                         .                         +
*         |                         .                         |
*         \                         .                         /
*         req_recv_next             .             rsp_recv_next
*           +                       .                       +
*           |                       .                       |       Rsp
*           req_recv_done           .           rsp_recv_done      <===
*             +                     .                     +
*             |                     .                     |
*    Req      \                     .                     /
*    ===>     req_filter*           .           *rsp_filter
*               +                   .                   +
*               |                   .                   |
*               \                   .                   /
*               req_forward-//  (a) . (c)  \\-rsp_forward
*                                   .
*                                   .
*       msg_send {write event}      .      msg_send {write event}
*         +                         .                         +
*         |                         .                         |
*    Rsp' \                         .                         /     Req'
*   <===  rsp_send_next             .             req_send_next     ===>
*           +                       .                       +
*           |                       .                       |
*           \                       .                       /
*           rsp_send_done-//    (d) . (b)    //-req_send_done
*
*
* (a) -> (b) -> (c) -> (d) is the normal flow of transaction consisting
* of a single request response, where (a) and (b) handle request from
* client, while (c) and (d) handle the corresponding response from the
* server.
*/

static bool
req_filter(struct context *ctx, struct conn *conn, struct msg *msg)
{
    ASSERT(conn->client && !conn->proxy);

    if (msg_empty(msg)) {
        ASSERT(conn->rmsg == NULL);
        log_debug(LOG_VERB, "filter empty req %"PRIu64" from c %d", msg->id,
                  conn->sd);
        req_put(msg);
        return true;
    }

    /*
     * Handle "quit\r\n" (memcache) or "*1\r\n$4\r\nquit\r\n" (redis), which
     * is the protocol way of doing a passive close. The connection is closed
     * as soon as all pending replies have been written to the client.
     */
    if (msg->quit) {
        log_debug(LOG_INFO, "filter quit req %"PRIu64" from c %d", msg->id,
                  conn->sd);
        if (conn->rmsg != NULL) { //XXX
            log_debug(LOG_INFO, "discard invalid req %"PRIu64" len %"PRIu32" "
                      "from c %d sent after quit req", conn->rmsg->id,
                      conn->rmsg->mlen, conn->sd);
        }
        conn->eof = 1; 
        conn->recv_ready = 0;
        req_put(msg);
        return true;
    }

    /*
     * If this conn is not authenticated, we will mark it as noforward,
     * and handle it in the redis_reply handler.
     */
    if (!conn_authenticated(conn)) {
        msg->noforward = 1;
    }

    return false;
}

static void
req_forward_error(struct context *ctx, struct conn *conn, struct msg *msg)
{
    rstatus_t status;

    ASSERT(conn->client && !conn->proxy);

    log_debug(LOG_INFO, "forward req %"PRIu64" len %"PRIu32" type %d from "
              "c %d failed: %s", msg->id, msg->mlen, msg->type, conn->sd,
              strerror(errno));

    msg->done = 1;
    msg->error = 1;
    msg->err = errno;

    /* noreply request don't expect any response */
    if (msg->noreply) {
        req_put(msg);
        return;
    }

    if (req_done(conn, TAILQ_FIRST(&conn->omsg_q))) {
        status = event_add_out(ctx->evb, conn); //该错误msg会通过一次epoll写事件发送出去
        if (status != NC_OK) {
            conn->err = errno;
        }
    }
}

static void
req_forward_stats(struct context *ctx, struct server *server, struct msg *msg)
{
    ASSERT(msg->request);

    stats_server_incr(ctx, server, requests); //STATS_SERVER_requests自增
    stats_server_incr_by(ctx, server, request_bytes, msg->mlen);//STATS_SERVER_requests自增msg->mlen字节 客户端请求字节数
}

/*
*             Client+             Proxy           Server+
*                              (nutcracker)
*                                   .
*       msg_recv {read event}       .       msg_recv {read event}
*         +                         .                         +
*         |                         .                         |
*         \                         .                         /
*         req_recv_next             .             rsp_recv_next
*           +                       .                       +
*           |                       .                       |       Rsp
*           req_recv_done           .           rsp_recv_done      <===
*             +                     .                     +
*             |                     .                     |
*    Req      \                     .                     /
*    ===>     req_filter*           .           *rsp_filter
*               +                   .                   +
*               |                   .                   |
*               \                   .                   /
*               req_forward-//  (a) . (c)  \\-rsp_forward
*                                   .
*                                   .
*       msg_send {write event}      .      msg_send {write event}
*         +                         .                         +
*         |                         .                         |
*    Rsp' \                         .                         /     Req'
*   <===  rsp_send_next             .             req_send_next     ===>
*           +                       .                       +
*           |                       .                       |
*           \                       .                       /
*           rsp_send_done-//    (d) . (b)    //-req_send_done
*
*
* (a) -> (b) -> (c) -> (d) is the normal flow of transaction consisting
* of a single request response, where (a) and (b) handle request from
* client, while (c) and (d) handle the corresponding response from the
* server.
*/

//转发到后端服务器
static void
req_forward(struct context *ctx, struct conn *c_conn, struct msg *msg)
{
    rstatus_t status;
    struct conn *s_conn;
    struct server_pool *pool;
    uint8_t *key;
    uint32_t keylen;
    struct keypos *kpos;

    ASSERT(c_conn->client && !c_conn->proxy);

    /* enqueue message (request) into client outq, if response is expected */
    if (!msg->noreply) {
        c_conn->enqueue_outq(ctx, c_conn, msg);
        //req_forward把msg入队到客户端连接c_conn->enqueue_outq    req_send_done把msg入队到服务端连接s_conn->enqueue_outq
    }

    pool = c_conn->owner;

    ASSERT(array_n(msg->keys) > 0);
    kpos = array_get(msg->keys, 0);
    key = kpos->start;
    keylen = (uint32_t)(kpos->end - kpos->start);

    //选举后端服务器并建立连接
    s_conn = server_pool_conn(ctx, c_conn->owner, key, keylen);
    if (s_conn == NULL) {
        req_forward_error(ctx, c_conn, msg);
        return;
    }
    ASSERT(!s_conn->client && !s_conn->proxy);

    /* enqueue the message (request) into server inq */
    if (TAILQ_EMPTY(&s_conn->imsg_q)) { 
    //现在队列上面没有msg,但是下面会往该队列加msg,往后端的队列上面有msg,则添加些事件，通过epoll触发发送出去
        status = event_add_out(ctx->evb, s_conn); //该写事件触发在core_core中的写事件把imsg_q中的msg发送出去
        if (status != NC_OK) {
            req_forward_error(ctx, c_conn, msg);
            s_conn->err = errno;
            return;
        }
    }

    if (!conn_authenticated(s_conn)) { //现在还没有认证成功，则先进行认证
        status = msg->add_auth(ctx, c_conn, s_conn);
        if (status != NC_OK) {
            req_forward_error(ctx, c_conn, msg);
            s_conn->err = errno;
            return;
        }
    }
 
    //req_server_enqueue_imsgq
    s_conn->enqueue_inq(ctx, s_conn, msg);//在core_core中的写事件把imsg_q中的msg发送出去

    req_forward_stats(ctx, s_conn->owner, msg);

    log_debug(LOG_VERB, "forward from c %d to s %d req %"PRIu64" len %"PRIu32
              " type %d with key '%.*s'", c_conn->sd, s_conn->sd, msg->id,
              msg->mlen, msg->type, keylen, key);
}
/*
*             Client+             Proxy           Server+
*                              (nutcracker)
*                                   .
*       msg_recv {read event}       .       msg_recv {read event}
*         +                         .                         +
*         |                         .                         |
*         \                         .                         /
*         req_recv_next             .             rsp_recv_next
*           +                       .                       +
*           |                       .                       |       Rsp
*           req_recv_done           .           rsp_recv_done      <===
*             +                     .                     +
*             |                     .                     |
*    Req      \                     .                     /
*    ===>     req_filter*           .           *rsp_filter
*               +                   .                   +
*               |                   .                   |
*               \                   .                   /
*               req_forward-//  (a) . (c)  \\-rsp_forward
*                                   .
*                                   .
*       msg_send {write event}      .      msg_send {write event}
*         +                         .                         +
*         |                         .                         |
*    Rsp' \                         .                         /     Req'
*   <===  rsp_send_next             .             req_send_next     ===>
*           +                       .                       +
*           |                       .                       |
*           \                       .                       /
*           rsp_send_done-//    (d) . (b)    //-req_send_done
*
*
* (a) -> (b) -> (c) -> (d) is the normal flow of transaction consisting
* of a single request response, where (a) and (b) handle request from
* client, while (c) and (d) handle the corresponding response from the
* server.
*/

void
req_recv_done(struct context *ctx, struct conn *conn, struct msg *msg,
              struct msg *nmsg)
{
    rstatus_t status;
    struct server_pool *pool;
    struct msg_tqh frag_msgq;
    struct msg *sub_msg;
    struct msg *tmsg; 			/* tmp next message */

    ASSERT(conn->client && !conn->proxy);
    ASSERT(msg->request);
    ASSERT(msg->owner == conn);
    ASSERT(conn->rmsg == msg);
    ASSERT(nmsg == NULL || nmsg->request);

    /* enqueue next message (request), if any */
    conn->rmsg = nmsg;

    if (req_filter(ctx, conn, msg)) {
        return; //客户端发送了quit命令过来，则不用再处理KV对了
    }

    if (msg->noforward) { //不需要转到后端服务器，因为没有认证成功
        status = req_make_reply(ctx, conn, msg);
        if (status != NC_OK) {
            conn->err = errno;
            return;
        }

        status = msg->reply(msg);
        if (status != NC_OK) {
            conn->err = errno;
            return;
        }

        //通过core_core中的写事件触发写操作
        status = event_add_out(ctx->evb, conn);
        if (status != NC_OK) {
            conn->err = errno;
        }

        return;
    }

    /* do fragment */
    pool = conn->owner;
    TAILQ_INIT(&frag_msgq);
    //分片  mget mset等批处理命令中的不同KV可能分布在后端不同服务器上因此需要拆分
    status = msg->fragment(msg, pool->ncontinuum, &frag_msgq);//如果需要分发到多个后端服务器，则frag_msgq不为空
    if (status != NC_OK) { 
        if (!msg->noreply) {
            conn->enqueue_outq(ctx, conn, msg);
        }
        req_forward_error(ctx, conn, msg);
    }

    /* if no fragment happened */
    if (TAILQ_EMPTY(&frag_msgq)) {//如果需要分发到多个后端服务器，则frag_msgq不为空
        req_forward(ctx, conn, msg); //转到后端服务器
        return;
    }

    status = req_make_reply(ctx, conn, msg);
    if (status != NC_OK) {
        if (!msg->noreply) {
            conn->enqueue_outq(ctx, conn, msg);
        }
        req_forward_error(ctx, conn, msg);
    }

    for (sub_msg = TAILQ_FIRST(&frag_msgq); sub_msg != NULL; sub_msg = tmsg) {
        tmsg = TAILQ_NEXT(sub_msg, m_tqe);

        TAILQ_REMOVE(&frag_msgq, sub_msg, m_tqe);
        req_forward(ctx, conn, sub_msg); //
    }

    ASSERT(TAILQ_EMPTY(&frag_msgq));
    return;
}

/*
*             Client+             Proxy           Server+
*                              (nutcracker)
*                                   .
*       msg_recv {read event}       .       msg_recv {read event}
*         +                         .                         +
*         |                         .                         |
*         \                         .                         /
*         req_recv_next             .             rsp_recv_next
*           +                       .                       +
*           |                       .                       |       Rsp
*           req_recv_done           .           rsp_recv_done      <===
*             +                     .                     +
*             |                     .                     |
*    Req      \                     .                     /
*    ===>     req_filter*           .           *rsp_filter
*               +                   .                   +
*               |                   .                   |
*               \                   .                   /
*               req_forward-//  (a) . (c)  \\-rsp_forward
*                                   .
*                                   .
*       msg_send {write event}      .      msg_send {write event}
*         +                         .                         +
*         |                         .                         |
*    Rsp' \                         .                         /     Req'
*   <===  rsp_send_next             .             req_send_next     ===>
*           +                       .                       +
*           |                       .                       |
*           \                       .                       /
*           rsp_send_done-//    (d) . (b)    //-req_send_done
*
*
* (a) -> (b) -> (c) -> (d) is the normal flow of transaction consisting
* of a single request response, where (a) and (b) handle request from
* client, while (c) and (d) handle the corresponding response from the
* server.
*/

//接收的客户端msg信息通过req_server_enqueue_imsgq添加到该队列 在req_send_next中发往后端真实服务器
//从imsg_q队列中取出msg然后发往后端真实服务器
struct msg *   //发往客户端用rsp_send_next 发往后端服务器用req_send_next    msg_send或者msg_send_chain中执行
req_send_next(struct context *ctx, struct conn *conn) //从imsg_q队列中取出需要发往后端的msg
{//从队列imsg_q中取出msg发送  //msg_send中执行
    rstatus_t status;
    struct msg *msg, *nmsg; /* current and next message */

    ASSERT(!conn->client && !conn->proxy);

    if (conn->connecting) { //走到这里说明在server_connect中建立连接，添加写事件后，触发走到这里，说明连接建立成功了
        server_connected(ctx, conn);
    }

    nmsg = TAILQ_FIRST(&conn->imsg_q);
    if (nmsg == NULL) {
        /* nothing to send as the server inq is empty */
        status = event_del_out(ctx->evb, conn); //没有数据需要发往后端了，去除写事件
        if (status != NC_OK) {
            conn->err = errno;
        }

        return NULL;
    }

    msg = conn->smsg;
    if (msg != NULL) { 
    //如果上次还有未发送的msg，则取出来返回。也就是如果conn->smsg上面有还有未发送出去的数据，则从该smsg取出一个msg发送，否则从imsg_q队列上面取出来发送
        ASSERT(msg->request && !msg->done);
        nmsg = TAILQ_NEXT(msg, s_tqe);  //指向imsg_q的下一个msg
    }

    conn->smsg = nmsg; //需要发送往后端真实服务器的真实msg全部保存到conn->smsg中

    if (nmsg == NULL) {
        return NULL;
    }

    ASSERT(nmsg->request && !nmsg->done);

    log_debug(LOG_VVERB, "send next req %"PRIu64" len %"PRIu32" type %d on "
              "s %d", nmsg->id, nmsg->mlen, nmsg->type, conn->sd);

    return nmsg;
}

/*
*             Client+             Proxy           Server+
*                              (nutcracker)
*                                   .
*       msg_recv {read event}       .       msg_recv {read event}
*         +                         .                         +
*         |                         .                         |
*         \                         .                         /
*         req_recv_next             .             rsp_recv_next
*           +                       .                       +
*           |                       .                       |       Rsp
*           req_recv_done           .           rsp_recv_done      <===
*             +                     .                     +
*             |                     .                     |
*    Req      \                     .                     /
*    ===>     req_filter*           .           *rsp_filter
*               +                   .                   +
*               |                   .                   |
*               \                   .                   /
*               req_forward-//  (a) . (c)  \\-rsp_forward
*                                   .
*                                   .
*       msg_send {write event}      .      msg_send {write event}
*         +                         .                         +
*         |                         .                         |
*    Rsp' \                         .                         /     Req'
*   <===  rsp_send_next             .             req_send_next     ===>
*           +                       .                       +
*           |                       .                       |
*           \                       .                       /
*           rsp_send_done-//    (d) . (b)    //-req_send_done
*
*
* (a) -> (b) -> (c) -> (d) is the normal flow of transaction consisting
* of a single request response, where (a) and (b) handle request from
* client, while (c) and (d) handle the corresponding response from the
* server.
*/

//msg_send_chain中执行，表示数据已经发送完毕
void
req_send_done(struct context *ctx, struct conn *conn, struct msg *msg) //这里的conn是和服务端的连接
{//数据在该函数的前面已经发送出去了
    ASSERT(!conn->client && !conn->proxy);
    ASSERT(msg != NULL && conn->smsg == NULL);
    ASSERT(msg->request && !msg->done);
    ASSERT(msg->owner != conn);

    log_debug(LOG_VVERB, "send done req %"PRIu64" len %"PRIu32" type %d on "
              "s %d", msg->id, msg->mlen, msg->type, conn->sd);

    /* dequeue the message (request) from server inq */
    conn->dequeue_inq(ctx, conn, msg);

    /*
     * noreply request instructs the server not to send any response. So,
     * enqueue message (request) in server outq, if response is expected.
     * Otherwise, free the noreply request
     */
    if (!msg->noreply) {
    //来自客户端的msg已经发送到后端服务器了，该msg并不会释放，而是添加到s_conn->omsg_q队列，用于说明该msg等待后端对应的应答
        conn->enqueue_outq(ctx, conn, msg);
    } else {
        req_put(msg);
    }
}
