
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 * Copyright (C) Winshining
 */


#ifndef _NGX_RTMP_UPSTREAM_ROUND_ROBIN_H_INCLUDED_
#define _NGX_RTMP_UPSTREAM_ROUND_ROBIN_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"


typedef struct ngx_rtmp_upstream_rr_peer_s   ngx_rtmp_upstream_rr_peer_t;

struct ngx_rtmp_upstream_rr_peer_s {
    struct sockaddr                *sockaddr;
    socklen_t                       socklen;
    ngx_str_t                       name;
    ngx_str_t                       server;

    ngx_int_t                       current_weight;
    ngx_int_t                       effective_weight;
    ngx_int_t                       weight;

    ngx_uint_t                      conns;
    ngx_uint_t                      max_conns;

    ngx_uint_t                      fails;
    time_t                          accessed;
    time_t                          checked;

    ngx_uint_t                      max_fails;
    time_t                          fail_timeout;
    ngx_msec_t                      slow_start;
    ngx_msec_t                      start_time;

    ngx_uint_t                      down;

#if (NGX_RTMP_UPSTREAM_ZONE)
    ngx_atomic_t                    lock;
#endif

    ngx_rtmp_upstream_rr_peer_t    *next;

#if (nginx_version >= 1011004)
    NGX_COMPAT_BEGIN(32)
    NGX_COMPAT_END
#endif
};


typedef struct ngx_rtmp_upstream_rr_peers_s  ngx_rtmp_upstream_rr_peers_t;

struct ngx_rtmp_upstream_rr_peers_s {
    ngx_uint_t                      number;

#if (NGX_RTMP_UPSTREAM_ZONE)
    ngx_slab_pool_t                *shpool;
    ngx_atomic_t                    rwlock;
    ngx_rtmp_upstream_rr_peers_t   *zone_next;
#endif

    ngx_uint_t                      total_weight;

    unsigned                        single:1;
    unsigned                        weighted:1;

    ngx_str_t                      *name;

    ngx_rtmp_upstream_rr_peers_t   *next;

    ngx_rtmp_upstream_rr_peer_t    *peer;
};


#if (NGX_RTMP_UPSTREAM_ZONE)

#define ngx_rtmp_upstream_rr_peers_rlock(peers)                               \
                                                                              \
    if (peers->shpool) {                                                      \
        ngx_rwlock_rlock(&peers->rwlock);                                     \
    }

#define ngx_rtmp_upstream_rr_peers_wlock(peers)                               \
                                                                              \
    if (peers->shpool) {                                                      \
        ngx_rwlock_wlock(&peers->rwlock);                                     \
    }

#define ngx_rtmp_upstream_rr_peers_unlock(peers)                              \
                                                                              \
    if (peers->shpool) {                                                      \
        ngx_rwlock_unlock(&peers->rwlock);                                    \
    }


#define ngx_rtmp_upstream_rr_peer_lock(peers, peer)                           \
                                                                              \
    if (peers->shpool) {                                                      \
        ngx_rwlock_wlock(&peer->lock);                                        \
    }

#define ngx_rtmp_upstream_rr_peer_unlock(peers, peer)                         \
                                                                              \
    if (peers->shpool) {                                                      \
        ngx_rwlock_unlock(&peer->lock);                                       \
    }

#else

#define ngx_rtmp_upstream_rr_peers_rlock(peers)
#define ngx_rtmp_upstream_rr_peers_wlock(peers)
#define ngx_rtmp_upstream_rr_peers_unlock(peers)
#define ngx_rtmp_upstream_rr_peer_lock(peers, peer)
#define ngx_rtmp_upstream_rr_peer_unlock(peers, peer)

#endif


typedef struct {
    ngx_uint_t                      config;
    ngx_rtmp_upstream_rr_peers_t   *peers;
    ngx_rtmp_upstream_rr_peer_t    *current;
    uintptr_t                      *tried;
    uintptr_t                       data;
} ngx_rtmp_upstream_rr_peer_data_t;


ngx_int_t ngx_rtmp_upstream_init_round_robin(ngx_conf_t *cf,
    ngx_rtmp_upstream_srv_conf_t *us);
ngx_int_t ngx_rtmp_upstream_init_round_robin_peer(ngx_rtmp_session_t *s,
    ngx_rtmp_upstream_srv_conf_t *us);
ngx_int_t ngx_rtmp_upstream_create_round_robin_peer(ngx_rtmp_session_t *s,
    ngx_rtmp_upstream_resolved_t *ur);
ngx_int_t ngx_rtmp_upstream_get_round_robin_peer(ngx_peer_connection_t *pc,
    void *data);
void ngx_rtmp_upstream_free_round_robin_peer(ngx_peer_connection_t *pc,
    void *data, ngx_uint_t state);


#endif /* _NGX_RTMP_UPSTREAM_ROUND_ROBIN_H_INCLUDED_ */

