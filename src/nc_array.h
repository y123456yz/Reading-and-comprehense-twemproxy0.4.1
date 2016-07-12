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

#ifndef _NC_ARRAY_H_
#define _NC_ARRAY_H_

#include <nc_core.h>

typedef int (*array_compare_t)(const void *, const void *);
typedef rstatus_t (*array_each_t)(void *, void *);

struct array { //参考array_init
    uint32_t nelem;  /* # element */ //数组中已使用的element数目
    void     *elem;  /* element */ //真实的数组空间
    size_t   size;   /* element size */ //每个elem空间大小
    uint32_t nalloc; /* # allocated element */ //总的开辟的elem空间
};

#define null_array { 0, NULL, 0, 0 }

static inline void
array_null(struct array *a)
{
    a->nelem = 0;
    a->elem = NULL;
    a->size = 0;
    a->nalloc = 0;
}

static inline void
array_set(struct array *a, void *elem, size_t size, uint32_t nalloc)
{
    a->nelem = 0;
    a->elem = elem;
    a->size = size;
    a->nalloc = nalloc;
}

static inline uint32_t
array_n(const struct array *a)
{
    return a->nelem;
}

struct array *array_create(uint32_t n, size_t size);
void array_destroy(struct array *a);
rstatus_t array_init(struct array *a, uint32_t n, size_t size);
void array_deinit(struct array *a);

uint32_t array_idx(struct array *a, void *elem);
void *array_push(struct array *a);
void *array_pop(struct array *a);
void *array_get(struct array *a, uint32_t idx);
void *array_top(struct array *a);
void array_swap(struct array *a, struct array *b);
void array_sort(struct array *a, array_compare_t compare);
rstatus_t array_each(struct array *a, array_each_t func, void *data);

#endif
