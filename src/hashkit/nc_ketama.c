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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <nc_core.h>
#include <nc_server.h>
#include <nc_hashkit.h>

//按ketama的算法，在这个环上创建服务器数*160个点，这些点把环分成了同等数量的段。
#define KETAMA_CONTINUUM_ADDITION   10  /* # extra slots to build into continuum */
#define KETAMA_POINTS_PER_SERVER    160 /* 40 points per hash */
#define KETAMA_MAX_HOSTLEN          86

//计算某个主机，某个point的hash值
//alignment的值固定是4，ketama_hash是对由server名+索引组成的md5签名，从第16位开始取值，再重组一个32位值。
static uint32_t
ketama_hash(const char *key, size_t key_length, uint32_t alignment) //YANG ADD XXXXXXXXXX TODO，分配这里需要改为何redis一致
{
    unsigned char results[16];

    md5_signature((unsigned char*)key, key_length, results);

    return ((uint32_t) (results[3 + alignment * 4] & 0xFF) << 24)
        | ((uint32_t) (results[2 + alignment * 4] & 0xFF) << 16)
        | ((uint32_t) (results[1 + alignment * 4] & 0xFF) << 8)
        | (results[0 + alignment * 4] & 0xFF);
}

//比较两个连续区的值，用于在ketama_update 方法中排序
static int
ketama_item_cmp(const void *t1, const void *t2)
{
    const struct continuum *ct1 = t1, *ct2 = t2;

    if (ct1->value == ct2->value) {
        return 0;
    } else if (ct1->value > ct2->value) {
        return 1;
    } else {
        return -1;
    }
}

/*
ketama对一致性hash算法的实现思路是：
(1) 通过配置文件，建立一个服务器列表，其形式如：(1.1.1.1:11211, 2.2.2.2:11211,9.8.7.6:11211...)
(2) 对每个服务器列表中的字符串，通过Hash算法，hash成几个无符号型整数。
    注意：如何通过hash算法来计算呢？
(3) 把这几个无符号型整数放到一个环上，这个换被称为continuum。（我们可以想象，一个从0到2^32的钟表）
(4) 可以建立一个数据结构，把每个数和服务器的ip地址对应在一起，这样，每个服务器就出现在这个环上的这几个位置上。
    注意：这几个数，不能随着服务器的增加和删除而变化，这样才能保证集群增加/删除机器后，以前的那些key都映射到同样的ip地址上。后面将会详细说明怎么做。
(5) 为了把一个key映射到一个服务器上，先要对key做hash，形成一个无符号型整数un，然后在环continuum上查找大于un的下一个数值。若找到，就把key保存到这台服务器上。
(6) 若你的hash(key)值超过continuum上的最大整数值，就直接回饶到continuum环的开始位置。
    这样，添加或删除集群中的结点，就只会影响一少部分key的分布。
    注意：这里说的会影响一部分key是相对的。其实影响的key的多少，由该ip地址占的权重大小决定的。在ketama
    的配置文件中，需要指定每个ip地址的权重。权重大的在环上占的点就多。
*/
//ketama一致性hash参考:http://www.cnblogs.com/basecn/p/4288456.html  http://blog.chinaunix.net/uid-20498361-id-4303232.html
//该函数是创建continuum的核心函数，它先从配置文件中读取集群服务器ip和端口，以及权重信息。创建continuum环，并把这些服务器信息和环上的数组下标对应起来。


/*
根据配置信息中的服务器权重，创建在环上的点结构，例如三个服务器权重值分别为1 2 3，则创建的对应在环(环的取值范围是0-0XFFFFFFFF)上
的struct continuum结构数目为(160*3) * (1/6),(160*3) * (2/6), (160*3) * (3/6),然后通过server做关键字做hash运算，算出在环上的
(160*3) * (1/6),(160*3) * (2/6), (160*3) * (3/6)个点对应的位置，一般都是比较均匀的分布。并对struct continuum对应的value(也就是在还上的位置)
进行排序。
*/
rstatus_t  //更新server-pool的分配策略
ketama_update(struct server_pool *pool) //server_pool_server真正选举后端服务器，ketama_update为一致性hash相关的ketama算法
{
    // 该变量来记录共从配置文件中共读取了多少个服务器
    uint32_t nserver;             /* # server - live and dead */
    //有效服务器总数
    uint32_t nlive_server;        /* # live server */
    //如果服务器的权重刚好平均值的话，这个值就是160，
    uint32_t pointer_per_server;  /* pointers per server proportional to weight */
    uint32_t pointer_per_hash;    /* pointers per hash */
    uint32_t pointer_counter;     /* # pointers on continuum */
    uint32_t pointer_index;       /* pointer index */
    uint32_t points_per_server;   /* points per server */ //KETAMA_POINTS_PER_SERVER
    //无符号型整数放到一个环上，这个换被称为continuum
    uint32_t continuum_index;     /* continuum index */
    uint32_t continuum_addition;  /* extra space in the continuum */
    uint32_t server_index;        /* server index */
    uint32_t value;               /* continuum value */
    // 该变量是配置文件中所有服务器权重值得总和
    uint32_t total_weight;        /* total live server weight */
    int64_t now;                  /* current timestamp in usec */

    ASSERT(array_n(&pool->server) > 0);

    now = nc_usec_now();
    if (now < 0) {
        return NC_ERROR;
    }

    /*
     * Count live servers and total weight, and also update the next time to
     * rebuild the distribution
     */
    nserver = array_n(&pool->server);
    nlive_server = 0;
    total_weight = 0; //所有服务器的权重总和
    pool->next_rebuild = 0LL;
    for (server_index = 0; server_index < nserver; server_index++) {
        struct server *server = array_get(&pool->server, server_index);

        if (pool->auto_eject_hosts) {
            if (server->next_retry <= now) {
                server->next_retry = 0LL;
                nlive_server++;
            } else if (pool->next_rebuild == 0LL ||
                       server->next_retry < pool->next_rebuild) {
                pool->next_rebuild = server->next_retry;
            }
        } else {
            nlive_server++;
        }

        ASSERT(server->weight > 0);

        /* count weight only for live servers */
        if (!pool->auto_eject_hosts || server->next_retry <= now) {
            total_weight += server->weight;
        }
    }

    pool->nlive_server = nlive_server;

    if (nlive_server == 0) {
        log_debug(LOG_DEBUG, "no live servers for pool %"PRIu32" '%.*s'",
                  pool->idx, pool->name.len, pool->name.data);

        return NC_OK;
    }
    log_debug(LOG_DEBUG, "%"PRIu32" of %"PRIu32" servers are live for pool "
              "%"PRIu32" '%.*s'", nlive_server, nserver, pool->idx,
              pool->name.len, pool->name.data);

    continuum_addition = KETAMA_CONTINUUM_ADDITION;
    points_per_server = KETAMA_POINTS_PER_SERVER;
    /*
     * Allocate the continuum for the pool, the first time, and every time we
     * add a new server to the pool
     */
    if (nlive_server > pool->nserver_continuum) {
        struct continuum *continuum;
        uint32_t nserver_continuum = nlive_server + continuum_addition; //实际在线服务器数 + 10
        //按ketama的算法，在这个环上创建服务器数*160个点，这些点把环分成了同等数量的段。
        uint32_t ncontinuum = nserver_continuum * points_per_server;//*160

        continuum = nc_realloc(pool->continuum, sizeof(*continuum) * ncontinuum); //实际上这里多分配了continuum_addition * 160个continuum结构，为什么呢?????
        if (continuum == NULL) {
            return NC_ENOMEM;
        }

        pool->continuum = continuum; //一共分配了多少个struct continuum结构，(实际的服务器数+10)*160
        pool->nserver_continuum = nserver_continuum;  //一致性hash后端有效服务器数+ KETAMA_CONTINUUM_ADDITION
        /* pool->ncontinuum is initialized later as it could be <= ncontinuum */
    }

    /*
     * Build a continuum with the servers that are live and points from
     * these servers that are proportial to their weight
     */
    continuum_index = 0;
    pointer_counter = 0;
    for (server_index = 0; server_index < nserver; server_index++) {
        struct server *server;
        float pct;

        server = array_get(&pool->server, server_index);

        if (pool->auto_eject_hosts && server->next_retry > now) {
            continue;
        }

        pct = (float)server->weight / (float)total_weight;
        //floorf 返回小于或者等于指定表达式的最大整数。 
        //根据权重计算某个服务器上应该拥有的环点数个数，权重越大，点数越多
        pointer_per_server = (uint32_t) ((floorf((float) (pct * KETAMA_POINTS_PER_SERVER / 4 * (float)nlive_server + 0.0000000001))) * 4);
        pointer_per_hash = 4;

/* 配置对应的打印
servers:
 - 192.168.1.111:7000:1 server1
 - 192.168.1.111:7001:1 server2
 - 192.168.1.111:7002:1 server3

[2016-06-06 15:02:05.946] nc_ketama.c:188 server1 weight 1 of 3 pct 0.33333 points per server 160
[2016-06-06 15:02:05.946] nc_ketama.c:188 server2 weight 1 of 3 pct 0.33333 points per server 160
[2016-06-06 15:02:05.946] nc_ketama.c:188 server3 weight 1 of 3 pct 0.33333 points per server 160
*/
        log_debug(LOG_VERB, "%.*s weight %"PRIu32" of %"PRIu32" "
                  "pct %0.5f points per server %"PRIu32"",
                  server->name.len, server->name.data, server->weight,
                  total_weight, pct, pointer_per_server);

        for (pointer_index = 1;
             pointer_index <= pointer_per_server / pointer_per_hash;
             pointer_index++) {

            char host[KETAMA_MAX_HOSTLEN]= "";
            size_t hostlen;
            uint32_t x;

            hostlen = snprintf(host, KETAMA_MAX_HOSTLEN, "%.*s-%u",
                               server->name.len, server->name.data,
                               pointer_index - 1);

            for (x = 0; x < pointer_per_hash; x++) { //平均一个服务器在环上会有160个点，但会根据权重比值有所不同，权重越大，在环上的点数越多
                value = ketama_hash(host, hostlen, x); 
                pool->continuum[continuum_index].index = server_index; //在还上的value值点锁对应的后端server
                pool->continuum[continuum_index++].value = value; //在环上的value
            }
        }
        pointer_counter += pointer_per_server; //计算所有后端服务器在环上面有多少个点
    }

    pool->ncontinuum = pointer_counter; //计算所有后端服务器在环上面有多少个点,该数值比实际分配的continuum要少10*160个
    qsort(pool->continuum, pool->ncontinuum, sizeof(*pool->continuum),
          ketama_item_cmp); //根据在还上的value点数，对所有的continuum进行排序

    for (pointer_index = 0;
         pointer_index < ((nlive_server * KETAMA_POINTS_PER_SERVER) - 1);
         pointer_index++) {
        if (pointer_index + 1 >= pointer_counter) {
            break;
        }
        ASSERT(pool->continuum[pointer_index].value <=
               pool->continuum[pointer_index + 1].value);
    }

    //updated pool 0 'beta' with 3 of 3 servers live in 13 slots and 480 active points in 3680 slots
    log_debug(LOG_VERB, "updated pool %"PRIu32" '%.*s' with %"PRIu32" of "
              "%"PRIu32" servers live in %"PRIu32" slots and %"PRIu32" "
              "active points in %"PRIu32" slots", pool->idx,
              pool->name.len, pool->name.data, nlive_server, nserver,
              pool->nserver_continuum, pool->ncontinuum,
              (pool->nserver_continuum + continuum_addition) * points_per_server);

    return NC_OK;
}

//找出给定hash值所在的连续区   使用二分法找到一个值在环中的对应区域。
uint32_t
ketama_dispatch(struct continuum *continuum, uint32_t ncontinuum, uint32_t hash)
{
    struct continuum *begin, *end, *left, *right, *middle;

    ASSERT(continuum != NULL);
    ASSERT(ncontinuum != 0);

    begin = left = continuum;
    end = right = continuum + ncontinuum;

    while (left < right) {
        middle = left + (right - left) / 2;
        if (middle->value < hash) {
          left = middle + 1;
        } else {
          right = middle;
        }
    }

    if (right == end) {
        right = begin;
    }

    return right->index;
}

