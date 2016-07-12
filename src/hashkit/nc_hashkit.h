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

#ifndef _NC_HASHKIT_H_
#define _NC_HASHKIT_H_

#include <nc_core.h>
#include <nc_server.h>
//HASH_ONE_AT_A_TIME对应one_at_a_time
#define HASH_CODEC(ACTION)                      \
    ACTION( HASH_ONE_AT_A_TIME, one_at_a_time ) \
    ACTION( HASH_MD5,           md5           ) \
    ACTION( HASH_CRC16,         crc16         ) \
    ACTION( HASH_CRC32,         crc32         ) \
    ACTION( HASH_CRC32A,        crc32a        ) \
    ACTION( HASH_FNV1_64,       fnv1_64       ) \
    ACTION( HASH_FNV1A_64,      fnv1a_64      ) \
    ACTION( HASH_FNV1_32,       fnv1_32       ) \
    ACTION( HASH_FNV1A_32,      fnv1a_32      ) \
    ACTION( HASH_HSIEH,         hsieh         ) \
    ACTION( HASH_MURMUR,        murmur        ) \
    ACTION( HASH_JENKINS,       jenkins       ) \

/*
distribution  存在ketama、modula和random3种可选的配置。其含义如下：  
ketama  ketama一致性hash算法，会根据服务器构造出一个hash ring，并为ring上的节点分配hash范围。ketama的
    优势在于单个节点添加、删除之后，会最大程度上保持整个群集中缓存的key值可以被重用。  
modula  modula非常简单，就是根据key值的hash值取模，根据取模的结果选择对应的服务器。  
random  random是无论key值的hash是什么，都随机的选择一个服务器作为key值操作的目标。
*/
#define DIST_CODEC(ACTION)                      \
    ACTION( DIST_KETAMA,        ketama        ) \
    ACTION( DIST_MODULA,        modula        ) \
    ACTION( DIST_RANDOM,        random        ) \

#define DEFINE_ACTION(_hash, _name) _hash,
typedef enum hash_type {
    HASH_CODEC( DEFINE_ACTION )
    HASH_SENTINEL
} hash_type_t;
#undef DEFINE_ACTION

#define DEFINE_ACTION(_dist, _name) _dist,
typedef enum dist_type {
    DIST_CODEC( DEFINE_ACTION )
    DIST_SENTINEL
} dist_type_t;
#undef DEFINE_ACTION

uint32_t hash_one_at_a_time(const char *key, size_t key_length);
void md5_signature(const unsigned char *key, unsigned int length, unsigned char *result);
uint32_t hash_md5(const char *key, size_t key_length);
uint32_t hash_crc16(const char *key, size_t key_length);
uint32_t hash_crc32(const char *key, size_t key_length);
uint32_t hash_crc32a(const char *key, size_t key_length);
uint32_t hash_fnv1_64(const char *key, size_t key_length);
uint32_t hash_fnv1a_64(const char *key, size_t key_length);
uint32_t hash_fnv1_32(const char *key, size_t key_length);
uint32_t hash_fnv1a_32(const char *key, size_t key_length);
uint32_t hash_hsieh(const char *key, size_t key_length);
uint32_t hash_jenkins(const char *key, size_t length);
uint32_t hash_murmur(const char *key, size_t length);

rstatus_t ketama_update(struct server_pool *pool);
uint32_t ketama_dispatch(struct continuum *continuum, uint32_t ncontinuum, uint32_t hash);
rstatus_t modula_update(struct server_pool *pool);
uint32_t modula_dispatch(struct continuum *continuum, uint32_t ncontinuum, uint32_t hash);
rstatus_t random_update(struct server_pool *pool);
uint32_t random_dispatch(struct continuum *continuum, uint32_t ncontinuum, uint32_t hash);

#endif
