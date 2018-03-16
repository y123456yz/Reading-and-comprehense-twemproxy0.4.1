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

#ifndef _NC_CONNECTION_H_
#define _NC_CONNECTION_H_

#include <nc_core.h>

typedef rstatus_t (*conn_recv_t)(struct context *, struct conn*);
typedef struct msg* (*conn_recv_next_t)(struct context *, struct conn *, bool);
typedef void (*conn_recv_done_t)(struct context *, struct conn *, struct msg *, struct msg *);

typedef rstatus_t (*conn_send_t)(struct context *, struct conn*);
typedef struct msg* (*conn_send_next_t)(struct context *, struct conn *);
typedef void (*conn_send_done_t)(struct context *, struct conn *, struct msg *);

typedef void (*conn_close_t)(struct context *, struct conn *);
typedef bool (*conn_active_t)(struct conn *);

typedef void (*conn_ref_t)(struct conn *, void *);
typedef void (*conn_unref_t)(struct conn *);

typedef void (*conn_msgq_t)(struct context *, struct conn *, struct msg *);
typedef void (*conn_post_connect_t)(struct context *ctx, struct conn *, struct server *server);
typedef void (*conn_swallow_msg_t)(struct conn *, struct msg *, struct msg *);

//创建空间和赋值见_conn_get
struct conn { //做客户端和服务端对应的相关回调见conn_get
    TAILQ_ENTRY(conn)   conn_tqe;        /* link in server_pool / server / free q */
    //client_ref  server_ref  proxy_ref中赋值  
    //如果本twemproxy为服务端,也就是做proxy,走proxy流程，则对端就是客户端，起连接的是配置文件中的listen在监听该客户端，因此owner指向大server server_pool
//如果本地twemproxy为客户端，走server流程，则对端为后端真实redis服务器，因此owner指向后端真实struct server
//client_ref  proxy_ref  server_ref中赋值
    void                *owner;          /* connection owner - server_pool / server */

    //和后端服务器真正建立连接在server_connect  做服务端，对待就是客户端，这时候表示bind服务端的fd，见proxy_listen
    int                 sd;              /* socket descriptor */

    //赋值见server_resolve  
    int                 family;          /* socket address family */
    socklen_t           addrlen;         /* socket length */
    struct sockaddr     *addr;           /* socket address (ref in server or server_pool) */

    /* enqueue_inq用来把接收到的客户端KV信息msg入队到c_conn->imsg_q，当该msg发送到后端服务器后，dequeue_inq把该msg从c_conn->imsg_q中摘除掉
     然后分别在req_forward和req_send_done通过enqueue_outq把该msg入队到c_conn->omsg_q和s_conn->omsg_q中等待后端应答该msg对应的ack，
     后端应答后会创建一个新的msg接收后端应答信息，再通过rsp_forward和rsp_send_done分别把dequeue_outq把该msg摘除，然后通过msg_put
     归还给free_msg。同时在rsp_forward中把后端应答的msg和读取客户端信息的msg关联起来。从而可以把后端数据发送给客户端
     
     因此统计信息中的in_queue代表的就是imsg_q中的msg信息，表示还没有发送到后端服务器的msg信息。统计信息中的out_queue代表的就是
     omsg_q中的msg信息，表示msg已经发送到后端服务器但是还没有得到后端服务器应答信息的msg, 
     */

    //req_server_enqueue_imsgq添加到队列尾部，req_server_enqueue_imsgq_head添加到队列头部  req_server_dequeue_imsgq移除
    //接收的客户端msg信息通过req_server_enqueue_imsgq添加到该队列 在req_send_next中取出发往后端真实服务器  队列写事件在req_forward->event_add_out中添加
    //imsg_q队列记录的是接收到的客户端还没有发送到后端服务器的msg信息，当发送到后端服务器后，会从imsq_q取出然后添加到omsg_q，omsq_q用于等待该请求msg对应的应答信息
    struct msg_tqh      imsg_q;          /* incoming request Q */ //在core_core中的写事件把imsg_q中的msg发送出去
    //可以参考req_client_enqueue_omsgq
    //imsg_q队列记录的是接收到的客户端还没有发送到后端服务器的msg信息，当发送到后端服务器后，会从imsq_q取出然后添加到omsg_q，omsq_q用于等待该请求msg对应的应答信息
    //req_client_enqueue_omsgq   req_server_enqueue_omsgq中入队
    struct msg_tqh      omsg_q;          /* outstanding request Q */
    //上次解析请求内容未解析完成的部分用rmgs暂存起来(例如一个KV过大，客户端发送了一部分过来，则下次客户端再次发送的时候还是使用该rmsg接受)，rmsg就是接受连接上数据的msg  
    //读取后端服务器数据在rsp_recv_next rsp_recv_done中赋值，读取客户端数据在req_recv_next和req_recv_done中赋值
    //说明之前某个KV数据没读取完毕，因此就不会往后端转发，这次还是使用该msg进行读取新来的数据，见req_recv_next
    //req_recv_next 和 req_recv_done中赋值，为NULL，说明解析到完整的KV，不为NULL，说明还需要等待数据来组成一个完整KV
    struct msg          *rmsg;           /* current message being rcvd */
    //解析成功的数据存到该smsg中，需要发往后端真实服务器，见req_send_next
    //req_send_next和rsp_send_next中赋值
    struct msg          *smsg;           /* current message being sent */

    //msg_recv或者 proxy_recv
    conn_recv_t         recv;            /* recv (read) handler */
    /* req_recv_next; rsp_recv_next; */
    conn_recv_next_t    recv_next;       /* recv next message handler */
    //req_recv_done  rsp_recv_done             msg_parsed中执行
    conn_recv_done_t    recv_done;       /* read done handler */
    //msg_send   core_send中执行
    conn_send_t         send;            /* send (write) handler */
    //发往客户端用rsp_send_next 发往后端服务器用req_send_next    msg_send或者msg_send_chain中执行
    conn_send_next_t    send_next;       /* write next message handler */
    //req_send_done  rsp_send_done  msg_send_chain中执行
    conn_send_done_t    send_done;       /* write done handler */
    //client_close  server_close  proxy_close
    conn_close_t        close;           /* close handler */ //在发生网络异常的时候，会在core_close中执行该函数功能
    //client_active  server_active
    conn_active_t       active;          /* active? handler */
    //redis_post_connect或者memcache_post_connect
    conn_post_connect_t post_connect;    /* post connect handler */
    //redis_swallow_msg  memcache_swallow_msg
    conn_swallow_msg_t  swallow_msg;     /* react on messages to be swallowed */

    //server_ref client_ref proxy_ref  conn_get中执行该ref函数
    conn_ref_t          ref;             /* connection reference handler */
    //client_unref server_unref proxy_unref
    conn_unref_t        unref;           /* connection unreference handler */

    /* enqueue_inq用来把接收到的客户端KV信息msg入队到c_conn->imsg_q，当该msg发送到后端服务器后，dequeue_inq把该msg从c_conn->imsg_q中摘除掉
     然后分别在req_forward和req_send_done通过enqueue_outq把该msg入队到c_conn->omsg_q和s_conn->omsg_q中等待后端应答该msg对应的ack，
     后端应答后会创建一个新的msg接收后端应答信息，再通过rsp_forward和rsp_send_done通过dequeue_outq把该msg摘除，然后通过msg_put
     归还给free_msg。同时在rsp_forward中把后端应答的msg和读取客户端信息的msg关联起来。从而可以把后端数据发送给客户端
     
     因此统计信息中的in_queue代表的就是imsg_q中的msg信息，表示还没有发送到后端服务器的msg信息。统计信息中的out_queue代表的就是
     omsg_q中的msg信息，表示msg已经发送到后端服务器但是还没有得到后端服务器应答信息的msg, 
     */
    //req_forward中执行  req_server_enqueue_imsgq       req_forward或者redis_add_auth中执行
    conn_msgq_t         enqueue_inq;     /* connection inq msg enqueue handler */
    //req_server_dequeue_imsgq  在req_send_done把该队列的msg发送到后端服务器
    conn_msgq_t         dequeue_inq;     /* connection inq msg dequeue handler */

    /* enqueue_inq用来把接收到的客户端KV信息msg入队到c_conn->imsg_q，当该msg发送到后端服务器后，dequeue_inq把该msg从c_conn->imsg_q中摘除掉
     然后分别在req_forward和req_send_done通过enqueue_outq把该msg入队到c_conn->omsg_q和s_conn->omsg_q中等待后端应答该msg对应的ack，
     后端应答后会创建一个新的msg接收后端应答信息，再通过rsp_forward和rsp_send_done通过dequeue_outq把该msg摘除，然后通过msg_put
     归还给free_msg。同时在rsp_forward中把后端应答的msg和读取客户端信息的msg关联起来。从而可以把后端数据发送给客户端
     
     因此统计信息中的in_queue代表的就是imsg_q中的msg信息，表示还没有发送到后端服务器的msg信息。统计信息中的out_queue代表的就是
     omsg_q中的msg信息，表示msg已经发送到后端服务器但是还没有得到后端服务器应答信息的msg, 
     */
    
    //req_client_enqueue_omsgq  req_server_enqueue_omsgq中入队  req_send_done中会入队       enqueue_outq入队，dequeue_outp出对
    //enquee_outq的目的应该是记录发送到后端的命令，等后端应答后才知道是对那条命令的应答
    conn_msgq_t         enqueue_outq;    /* connection outq msg enqueue handler */
    //enqueue_outq入队，dequeue_outp出对  取值为req_client_dequeue_omsgq req_server_dequeue_omsgq  rsp_send_done中执行 rsp_forward中执行
    //rsp_send_done从客户端连接conn->dequeue_outq中出对  rsp_forward从服务端连接s_conn->dequeue_outq中出对
    conn_msgq_t         dequeue_outq;    /* connection outq msg dequeue handler */

    size_t              recv_bytes;      /* received (read) bytes */
    size_t              send_bytes;      /* sent (written) bytes */

    uint32_t            events;          /* connection io events */
    //如果是后端应答超时，该值为ETIMEDOUT，见core_timeout  
    //err置1，则在core_core中会关闭连接
    //eof是正常的触发close流程，例如quit的时候;而err置1表示程序有异常，需要触发close流程
    err_t               err;             /* connection errno */ 
    //proxy接收客户端连接，在event_add_conn添加读事件， client conn在event_add_in添加事件
    unsigned            recv_active:1;   /* recv active? */ //记录是否已经把该conn读事件添加到epoll
    //表示epoll检测到read事件，有数据可读或者有客户端连接  每次读取数据完毕，置0，表示数据读取完毕 
    //proxy_accept中把所有客户端连接都接收完毕后会置recv_ready为0   如果是连接后的读事件，则表示连接上有数据到来
    //读取完协议栈到来的数据后，表示协议栈数据已经读取完毕，会把recv_ready置0，见conn_recv
    unsigned            recv_ready:1;    /* recv ready? */ //记录是否已经把写conn读事件添加到epoll  为1说明有数据到来
    unsigned            send_active:1;   /* send active? */
    //msg_send中置1
    unsigned            send_ready:1;    /* send ready? */

    //本连接是客户端还是服务器，如果为客户端则为1，否则为0  当服务端accept返回后，会返回一个新的套接字，该套接字对应的conn的client为1
    unsigned            client:1;        /* client? or server? */
    //标识该conn为proxy连接，等待接收客户端连接，当接收到新的客户端连接后，会返回新的fd，新的conn
    unsigned            proxy:1;         /* proxy? */ //为1说明是接收客户端连接的bind对应的套接字,见conn_get_proxy
    //server_connect中connect直接返回但是没有建立连接成功  req_send_next->server_connected判断出连接建立成功
    unsigned            connecting:1;    /* connecting? */ //正在进行3次握手过程中 
    unsigned            connected:1;     /* connected? */ //connect成功  可以参考server_connected
    //置1表示需要关闭套接字，可以参考req_filter   例如req_filter客户端发送quit,或者后端进程挂了，读出错，见conn_recv置eof rsp_recv_next中关闭连接 
    //eof为1，数据发完的时候会职位done标记，在core_core中检测到done标记会关闭连接，见rsp_send_next
    //eof是正常的触发close流程，例如quit的时候;而err置1表示程序有异常，需要触发close流程
    unsigned            eof:1;           /* eof? aka passive close? */
    //如果eof为1，数据发完或者接受完的时候会职位done标记，在core_core中检测到done标记会关闭连接，见rsp_send_next  req_recv_next
    //req_recv_next中会检查发往客户端队列上的数据是否发送完成，只有发送完成后才会置done为1 ，然后在core_core中关闭连接

    //如果是直接把conn->err置位errno，则会立马关闭连接
    unsigned            done:1;          /* done? aka close? */
    //redis为1，非redis服务器为0
    unsigned            redis:1;         /* redis? */
    //是否已经认真成功
    unsigned            authenticated:1; /* authenticated? */
};

TAILQ_HEAD(conn_tqh, conn);

struct context *conn_to_ctx(struct conn *conn);
struct conn *conn_get(void *owner, bool client, bool redis);
struct conn *conn_get_proxy(void *owner);
void conn_put(struct conn *conn);
ssize_t conn_recv(struct conn *conn, void *buf, size_t size);
ssize_t conn_sendv(struct conn *conn, struct array *sendv, size_t nsend);
void conn_init(void);
void conn_deinit(void);
uint32_t conn_ncurr_conn(void);
uint64_t conn_ntotal_conn(void);
uint32_t conn_ncurr_cconn(void);
bool conn_authenticated(struct conn *conn);
uint32_t conn_ncurr_conn_zero(void);
uint64_t conn_ntotal_conn_zero(void);

#endif
