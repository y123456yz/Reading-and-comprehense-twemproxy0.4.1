/*
配置:
[root@centlhw1 twemproxy]# cat conf/nutcracker.yml
alpha:
  listen: 192.168.1.111:22121
  hash: fnv1a_64
  distribution: ketama
  auto_eject_hosts: true
  redis: false
  server_retry_timeout: 2000
  server_failure_limit: 1
  servers:
   - 127.0.0.1:11210:1
   - 127.0.0.1:11211:1
   - 127.0.0.1:11212:1
   - 127.0.0.1:11213:1

beta:
  listen: 0.0.0.0:22122
  hash: fnv1a_64
  hash_tag: "{}"
  distribution: ketama
  auto_eject_hosts: false
  timeout: 400
  redis: true
  servers:
   - 192.168.1.111:7000:1 server1
   - 192.168.1.111:7001:1 server2
   - 192.168.1.111:7002:1 server3


启动日志:
[root@centlhw1 twemproxy]# ./src/nutcracker -c conf/nutcracker.yml -v 11
[2016-05-19 11:16:08.422] nc.c:187 nutcracker-0.4.1 built for Linux 2.6.32-431.el6.x86_64 x86_64 started on pid 32245
[2016-05-19 11:16:08.422] nc.c:192 run, rabbit run / dig that hole, forget the sun / and when at last the work is done / don't sit down / it's time to dig another one
[2016-05-19 11:16:08.422] nc_mbuf.c:276 mbuf hsize 48 chunk size 16384 offset 16336 length 16336
[2016-05-19 11:16:08.422] nc_message.c:426 msg size 440
[2016-05-19 11:16:08.422] nc_connection.c:323 conn size 352
[2016-05-19 11:16:08.422] nc_util.c:235 malloc(88) at 0x24ab0b0 @ nc_core.c:55
[2016-05-19 11:16:08.422] nc_util.c:235 malloc(760) at 0x24ab790 @ nc_conf.c:773
[2016-05-19 11:16:08.422] nc_util.c:235 malloc(48) at 0x24ab110 @ nc_array.c:59
[2016-05-19 11:16:08.422] nc_util.c:235 malloc(2496) at 0x24aba90 @ nc_array.c:59
[2016-05-19 11:16:08.422] nc_conf.c:806 opened conf 'conf/nutcracker.yml'
[2016-05-19 11:16:08.422] nc_conf.c:962 conf 'conf/nutcracker.yml' has valid tokens
[2016-05-19 11:16:08.422] nc_conf.c:1030 next event 1 depth 0 seq 0
[2016-05-19 11:16:08.422] nc_conf.c:1030 next event 3 depth 0 seq 0
[2016-05-19 11:16:08.422] nc_conf.c:1030 next event 9 depth 0 seq 0
[2016-05-19 11:16:08.422] nc_conf.c:1030 next event 6 depth 1 seq 0
[2016-05-19 11:16:08.422] nc_conf.c:1030 next event 9 depth 1 seq 0
[2016-05-19 11:16:08.422] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 7 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 8 depth 2 seq 1
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 10 depth 2 seq 1
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 1 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 9 depth 1 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 7 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 8 depth 2 seq 1
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 10 depth 2 seq 1
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 10 depth 1 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 4 depth 0 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:1030 next event 2 depth 0 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:565 next begin event 1
[2016-05-19 11:16:08.423] nc_conf.c:565 next begin event 3
[2016-05-19 11:16:08.423] nc_conf.c:565 next begin event 9
[2016-05-19 11:16:08.423] nc_conf.c:641 next event 6 depth 1 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:475 push 'alpha'
[2016-05-19 11:16:08.423] nc_conf.c:512 conf handler on 'alpha'
[2016-05-19 11:16:08.423] nc_util.c:235 malloc(1472) at 0x24bcff0 @ nc_array.c:59
[2016-05-19 11:16:08.423] nc_conf.c:230 init conf pool 0x24aba90, 'alpha'
[2016-05-19 11:16:08.423] nc_conf.c:641 next event 9 depth 1 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:475 push 'listen'
[2016-05-19 11:16:08.423] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.423] nc_conf.c:475 push '192.168.1.111:22121'
[2016-05-19 11:16:08.423] nc_conf.c:521 conf handler on listen: 192.168.1.111:22121
[2016-05-19 11:16:08.423] nc_conf.c:498 pop '192.168.1.111:22121'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bcf60) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:498 pop 'listen'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bcf40) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.424] nc_conf.c:475 push 'hash'
[2016-05-19 11:16:08.424] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.424] nc_conf.c:475 push 'fnv1a_64'
[2016-05-19 11:16:08.424] nc_conf.c:521 conf handler on hash: fnv1a_64
[2016-05-19 11:16:08.424] nc_conf.c:498 pop 'fnv1a_64'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bd690) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:498 pop 'hash'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bcfb0) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.424] nc_conf.c:475 push 'distribution'
[2016-05-19 11:16:08.424] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.424] nc_conf.c:475 push 'ketama'
[2016-05-19 11:16:08.424] nc_conf.c:521 conf handler on distribution: ketama
[2016-05-19 11:16:08.424] nc_conf.c:498 pop 'ketama'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bcee0) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:498 pop 'distribution'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bcf60) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.424] nc_conf.c:475 push 'auto_eject_hosts'
[2016-05-19 11:16:08.424] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.424] nc_conf.c:475 push 'true'
[2016-05-19 11:16:08.424] nc_conf.c:521 conf handler on auto_eject_hosts: true
[2016-05-19 11:16:08.424] nc_conf.c:498 pop 'true'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bd690) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:498 pop 'auto_eject_hosts'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bcfb0) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.424] nc_conf.c:475 push 'redis'
[2016-05-19 11:16:08.424] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.424] nc_conf.c:475 push 'false'
[2016-05-19 11:16:08.424] nc_conf.c:521 conf handler on redis: false
[2016-05-19 11:16:08.424] nc_conf.c:498 pop 'false'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bcee0) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:498 pop 'redis'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bcf90) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.424] nc_conf.c:475 push 'server_retry_timeout'
[2016-05-19 11:16:08.424] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.424] nc_conf.c:475 push '2000'
[2016-05-19 11:16:08.424] nc_conf.c:521 conf handler on server_retry_timeout: 2000
[2016-05-19 11:16:08.424] nc_conf.c:498 pop '2000'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bcf40) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:498 pop 'server_retry_timeout'
[2016-05-19 11:16:08.424] nc_util.c:281 free(0x24bd690) @ nc_string.c:54
[2016-05-19 11:16:08.424] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push 'server_failure_limit'
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push '1'
[2016-05-19 11:16:08.425] nc_conf.c:521 conf handler on server_failure_limit: 1
[2016-05-19 11:16:08.425] nc_conf.c:498 pop '1'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bd6c0) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:498 pop 'server_failure_limit'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bcf90) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push 'servers'
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 7 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.425] nc_conf.c:475 push '127.0.0.1:11210:1'
[2016-05-19 11:16:08.425] nc_conf.c:521 conf handler on servers: 127.0.0.1:11210:1
[2016-05-19 11:16:08.425] nc_conf.c:133 init conf server 0x24bcff0
[2016-05-19 11:16:08.425] nc_conf.c:498 pop '127.0.0.1:11210:1'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bcf40) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.425] nc_conf.c:475 push '127.0.0.1:11211:1'
[2016-05-19 11:16:08.425] nc_conf.c:521 conf handler on servers: 127.0.0.1:11211:1
[2016-05-19 11:16:08.425] nc_conf.c:133 init conf server 0x24bd0a8
[2016-05-19 11:16:08.425] nc_conf.c:498 pop '127.0.0.1:11211:1'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bd720) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.425] nc_conf.c:475 push '127.0.0.1:11212:1'
[2016-05-19 11:16:08.425] nc_conf.c:521 conf handler on servers: 127.0.0.1:11212:1
[2016-05-19 11:16:08.425] nc_conf.c:133 init conf server 0x24bd160
[2016-05-19 11:16:08.425] nc_conf.c:498 pop '127.0.0.1:11212:1'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bd780) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.425] nc_conf.c:475 push '127.0.0.1:11213:1'
[2016-05-19 11:16:08.425] nc_conf.c:521 conf handler on servers: 127.0.0.1:11213:1
[2016-05-19 11:16:08.425] nc_conf.c:133 init conf server 0x24bd218
[2016-05-19 11:16:08.425] nc_conf.c:498 pop '127.0.0.1:11213:1'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bd7e0) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 8 depth 2 seq 1
[2016-05-19 11:16:08.425] nc_conf.c:498 pop 'servers'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bcee0) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 10 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:498 pop 'alpha'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bcec0) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 1 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push 'beta'
[2016-05-19 11:16:08.425] nc_conf.c:512 conf handler on 'beta'
[2016-05-19 11:16:08.425] nc_util.c:235 malloc(1472) at 0x24bd800 @ nc_array.c:59
[2016-05-19 11:16:08.425] nc_conf.c:230 init conf pool 0x24abbc8, 'beta'
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 9 depth 1 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push 'listen'
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push '0.0.0.0:22122'
[2016-05-19 11:16:08.425] nc_conf.c:521 conf handler on listen: 0.0.0.0:22122
[2016-05-19 11:16:08.425] nc_conf.c:498 pop '0.0.0.0:22122'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bd620) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:498 pop 'listen'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bd5e0) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push 'hash'
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push 'fnv1a_64'
[2016-05-19 11:16:08.425] nc_conf.c:521 conf handler on hash: fnv1a_64
[2016-05-19 11:16:08.425] nc_conf.c:498 pop 'fnv1a_64'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bde40) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:498 pop 'hash'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bde20) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push 'hash_tag'
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push '{}'
[2016-05-19 11:16:08.425] nc_conf.c:521 conf handler on hash_tag: {}
[2016-05-19 11:16:08.425] nc_conf.c:498 pop '{}'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bcee0) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:498 pop 'hash_tag'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bd620) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push 'distribution'
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push 'ketama'
[2016-05-19 11:16:08.425] nc_conf.c:521 conf handler on distribution: ketama
[2016-05-19 11:16:08.425] nc_conf.c:498 pop 'ketama'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bde60) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:498 pop 'distribution'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bd5e0) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push 'auto_eject_hosts'
[2016-05-19 11:16:08.425] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.425] nc_conf.c:475 push 'false'
[2016-05-19 11:16:08.425] nc_conf.c:521 conf handler on auto_eject_hosts: false
[2016-05-19 11:16:08.425] nc_conf.c:498 pop 'false'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bcee0) @ nc_string.c:54
[2016-05-19 11:16:08.425] nc_conf.c:498 pop 'auto_eject_hosts'
[2016-05-19 11:16:08.425] nc_util.c:281 free(0x24bd620) @ nc_string.c:54
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.426] nc_conf.c:475 push 'timeout'
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.426] nc_conf.c:475 push '400'
[2016-05-19 11:16:08.426] nc_conf.c:521 conf handler on timeout: 400
[2016-05-19 11:16:08.426] nc_conf.c:498 pop '400'
[2016-05-19 11:16:08.426] nc_util.c:281 free(0x24bde60) @ nc_string.c:54
[2016-05-19 11:16:08.426] nc_conf.c:498 pop 'timeout'
[2016-05-19 11:16:08.426] nc_util.c:281 free(0x24bd5e0) @ nc_string.c:54
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.426] nc_conf.c:475 push 'redis'
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.426] nc_conf.c:475 push 'true'
[2016-05-19 11:16:08.426] nc_conf.c:521 conf handler on redis: true
[2016-05-19 11:16:08.426] nc_conf.c:498 pop 'true'
[2016-05-19 11:16:08.426] nc_util.c:281 free(0x24bde40) @ nc_string.c:54
[2016-05-19 11:16:08.426] nc_conf.c:498 pop 'redis'
[2016-05-19 11:16:08.426] nc_util.c:281 free(0x24bcee0) @ nc_string.c:54
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 6 depth 2 seq 0
[2016-05-19 11:16:08.426] nc_conf.c:475 push 'servers'
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 7 depth 2 seq 0
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.426] nc_conf.c:475 push '192.168.1.111:7000:1 server1'
[2016-05-19 11:16:08.426] nc_conf.c:521 conf handler on servers: 192.168.1.111:7000:1 server1
[2016-05-19 11:16:08.426] nc_conf.c:133 init conf server 0x24bd800
[2016-05-19 11:16:08.426] nc_conf.c:498 pop '192.168.1.111:7000:1 server1'
[2016-05-19 11:16:08.426] nc_util.c:281 free(0x24bcf60) @ nc_string.c:54
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.426] nc_conf.c:475 push '192.168.1.111:7001:1 server2'
[2016-05-19 11:16:08.426] nc_conf.c:521 conf handler on servers: 192.168.1.111:7001:1 server2
[2016-05-19 11:16:08.426] nc_conf.c:133 init conf server 0x24bd8b8
[2016-05-19 11:16:08.426] nc_conf.c:498 pop '192.168.1.111:7001:1 server2'
[2016-05-19 11:16:08.426] nc_util.c:281 free(0x24bcf60) @ nc_string.c:54
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 6 depth 2 seq 1
[2016-05-19 11:16:08.426] nc_conf.c:475 push '192.168.1.111:7002:1 server3'
[2016-05-19 11:16:08.426] nc_conf.c:521 conf handler on servers: 192.168.1.111:7002:1 server3
[2016-05-19 11:16:08.426] nc_conf.c:133 init conf server 0x24bd970
[2016-05-19 11:16:08.426] nc_conf.c:498 pop '192.168.1.111:7002:1 server3'
[2016-05-19 11:16:08.426] nc_util.c:281 free(0x24bcf60) @ nc_string.c:54
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 8 depth 2 seq 1
[2016-05-19 11:16:08.426] nc_conf.c:498 pop 'servers'
[2016-05-19 11:16:08.426] nc_util.c:281 free(0x24bde60) @ nc_string.c:54
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 10 depth 2 seq 0
[2016-05-19 11:16:08.426] nc_conf.c:498 pop 'beta'
[2016-05-19 11:16:08.426] nc_util.c:281 free(0x24bcec0) @ nc_string.c:54
[2016-05-19 11:16:08.426] nc_conf.c:641 next event 10 depth 1 seq 0
[2016-05-19 11:16:08.426] nc_conf.c:605 next end event 4
[2016-05-19 11:16:08.426] nc_conf.c:605 next end event 2
[2016-05-19 11:16:08.426] nc_conf.c:335 2 pools in configuration file 'conf/nutcracker.yml'
[2016-05-19 11:16:08.427] nc_conf.c:340 beta
[2016-05-19 11:16:08.427] nc_conf.c:342   listen: 0.0.0.0:22122
[2016-05-19 11:16:08.427] nc_conf.c:343   timeout: 400
[2016-05-19 11:16:08.427] nc_conf.c:344   backlog: 512
[2016-05-19 11:16:08.427] nc_conf.c:345   hash: 6
[2016-05-19 11:16:08.427] nc_conf.c:347   hash_tag: "{}"
[2016-05-19 11:16:08.427] nc_conf.c:348   distribution: 0
[2016-05-19 11:16:08.427] nc_conf.c:350   client_connections: 0
[2016-05-19 11:16:08.427] nc_conf.c:351   redis: 1
[2016-05-19 11:16:08.427] nc_conf.c:352   preconnect: 0
[2016-05-19 11:16:08.427] nc_conf.c:353   auto_eject_hosts: 0
[2016-05-19 11:16:08.427] nc_conf.c:355   server_connections: 1
[2016-05-19 11:16:08.427] nc_conf.c:357   server_retry_timeout: 30000
[2016-05-19 11:16:08.427] nc_conf.c:359   server_failure_limit: 2
[2016-05-19 11:16:08.427] nc_conf.c:362   servers: 3
[2016-05-19 11:16:08.427] nc_conf.c:366     192.168.1.111:7000:1
[2016-05-19 11:16:08.427] nc_conf.c:366     192.168.1.111:7001:1
[2016-05-19 11:16:08.427] nc_conf.c:366     192.168.1.111:7002:1
[2016-05-19 11:16:08.427] nc_conf.c:340 alpha
[2016-05-19 11:16:08.427] nc_conf.c:342   listen: 192.168.1.111:22121
[2016-05-19 11:16:08.427] nc_conf.c:343   timeout: -1
[2016-05-19 11:16:08.427] nc_conf.c:344   backlog: 512
[2016-05-19 11:16:08.427] nc_conf.c:345   hash: 6
[2016-05-19 11:16:08.427] nc_conf.c:347   hash_tag: ""
[2016-05-19 11:16:08.427] nc_conf.c:348   distribution: 0
[2016-05-19 11:16:08.427] nc_conf.c:350   client_connections: 0
[2016-05-19 11:16:08.427] nc_conf.c:351   redis: 0
[2016-05-19 11:16:08.427] nc_conf.c:352   preconnect: 0
[2016-05-19 11:16:08.427] nc_conf.c:353   auto_eject_hosts: 1
[2016-05-19 11:16:08.427] nc_conf.c:355   server_connections: 1
[2016-05-19 11:16:08.427] nc_conf.c:357   server_retry_timeout: 2000
[2016-05-19 11:16:08.427] nc_conf.c:359   server_failure_limit: 1
[2016-05-19 11:16:08.427] nc_conf.c:362   servers: 4
[2016-05-19 11:16:08.427] nc_conf.c:366     127.0.0.1:11211:1
[2016-05-19 11:16:08.427] nc_conf.c:366     127.0.0.1:11210:1
[2016-05-19 11:16:08.427] nc_conf.c:366     127.0.0.1:11212:1
[2016-05-19 11:16:08.427] nc_conf.c:366     127.0.0.1:11213:1
[2016-05-19 11:16:08.427] nc_util.c:235 malloc(800) at 0x24ac460 @ nc_array.c:59
[2016-05-19 11:16:08.427] nc_util.c:235 malloc(792) at 0x24ac790 @ nc_array.c:59
[2016-05-19 11:16:08.427] nc_conf.c:177 transform to server 0 '192.168.1.111:7000:1'
[2016-05-19 11:16:08.427] nc_conf.c:177 transform to server 1 '192.168.1.111:7001:1'
[2016-05-19 11:16:08.427] nc_conf.c:177 transform to server 2 '192.168.1.111:7002:1'
[2016-05-19 11:16:08.427] nc_server.c:173 init 3 servers in pool 0 'beta'
[2016-05-19 11:16:08.427] nc_conf.c:317 transform to pool 0 'beta'
[2016-05-19 11:16:08.427] nc_util.c:235 malloc(1056) at 0x24acab0 @ nc_array.c:59
[2016-05-19 11:16:08.427] nc_conf.c:177 transform to server 0 '127.0.0.1:11211:1'
[2016-05-19 11:16:08.427] nc_conf.c:177 transform to server 1 '127.0.0.1:11210:1'
[2016-05-19 11:16:08.427] nc_conf.c:177 transform to server 2 '127.0.0.1:11212:1'
[2016-05-19 11:16:08.427] nc_conf.c:177 transform to server 3 '127.0.0.1:11213:1'
[2016-05-19 11:16:08.427] nc_server.c:173 init 4 servers in pool 1 'alpha'
[2016-05-19 11:16:08.427] nc_conf.c:317 transform to pool 1 'alpha'
[2016-05-19 11:16:08.427] nc_ketama.c:141 3 of 3 servers are live for pool 0 'beta'
[2016-05-19 11:16:08.427] nc_util.c:271 realloc(16640) at 0x24acee0 @ nc_ketama.c:155
[2016-05-19 11:16:08.427] nc_ketama.c:188 server1 weight 1 of 3 pct 0.33333 points per server 160
[2016-05-19 11:16:08.427] nc_ketama.c:188 server2 weight 1 of 3 pct 0.33333 points per server 160
[2016-05-19 11:16:08.427] nc_ketama.c:188 server3 weight 1 of 3 pct 0.33333 points per server 160
[2016-05-19 11:16:08.427] nc_ketama.c:230 updated pool 0 'beta' with 3 of 3 servers live in 13 slots and 480 active points in 3680 slots
[2016-05-19 11:16:08.427] nc_ketama.c:141 4 of 4 servers are live for pool 1 'alpha'
[2016-05-19 11:16:08.427] nc_util.c:271 realloc(17920) at 0x24b0ff0 @ nc_ketama.c:155
[2016-05-19 11:16:08.427] nc_ketama.c:188 127.0.0.1 weight 1 of 4 pct 0.25000 points per server 160
[2016-05-19 11:16:08.427] nc_ketama.c:188 127.0.0.1:11210 weight 1 of 4 pct 0.25000 points per server 160
[2016-05-19 11:16:08.428] nc_ketama.c:188 127.0.0.1:11212 weight 1 of 4 pct 0.25000 points per server 160
[2016-05-19 11:16:08.428] nc_ketama.c:188 127.0.0.1:11213 weight 1 of 4 pct 0.25000 points per server 160
[2016-05-19 11:16:08.429] nc_ketama.c:230 updated pool 1 'alpha' with 4 of 4 servers live in 14 slots and 640 active points in 3840 slots
[2016-05-19 11:16:08.429] nc_server.c:919 init 2 pools
[2016-05-19 11:16:08.429] nc_core.c:43 max fds 1024 max client conns 983 max server conns 9
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(336) at 0x24ab550 @ nc_stats.c:930
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(160) at 0x24ab6b0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(192) at 0x24b5600 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(144) at 0x24ab150 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b56d0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server 'server1' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b5880 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server 'server2' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b5a30 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server 'server3' with 13 metric
[2016-05-19 11:16:08.429] nc_stats.c:235 map 3 stats servers
[2016-05-19 11:16:08.429] nc_stats.c:283 init stats pool 'beta' with 6 metric and 6 server
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(192) at 0x24b5be0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(192) at 0x24b5cb0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b5d80 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b5f30 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1:11210' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b60e0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1:11212' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b6290 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1:11213' with 13 metric
[2016-05-19 11:16:08.429] nc_stats.c:235 map 4 stats servers
[2016-05-19 11:16:08.429] nc_stats.c:283 init stats pool 'alpha' with 6 metric and 6 server
[2016-05-19 11:16:08.429] nc_stats.c:333 map 2 stats pools
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(160) at 0x24b6440 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(192) at 0x24b64f0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(144) at 0x24b65c0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b6660 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server 'server1' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b6810 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server 'server2' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b69c0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server 'server3' with 13 metric
[2016-05-19 11:16:08.429] nc_stats.c:235 map 3 stats servers
[2016-05-19 11:16:08.429] nc_stats.c:283 init stats pool 'beta' with 6 metric and 6 server
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(192) at 0x24b6b70 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(192) at 0x24b6c40 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b6d10 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b6ec0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1:11210' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b7070 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1:11212' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b7220 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1:11213' with 13 metric
[2016-05-19 11:16:08.429] nc_stats.c:235 map 4 stats servers
[2016-05-19 11:16:08.429] nc_stats.c:283 init stats pool 'alpha' with 6 metric and 6 server
[2016-05-19 11:16:08.429] nc_stats.c:333 map 2 stats pools
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(160) at 0x24b73d0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(192) at 0x24b7480 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(144) at 0x24b7550 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b75f0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server 'server1' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b77a0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server 'server2' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b7950 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server 'server3' with 13 metric
[2016-05-19 11:16:08.429] nc_stats.c:235 map 3 stats servers
[2016-05-19 11:16:08.429] nc_stats.c:283 init stats pool 'beta' with 6 metric and 6 server
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(192) at 0x24b7b00 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(192) at 0x24b7bd0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b7ca0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b7e50 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1:11210' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b8000 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1:11212' with 13 metric
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(416) at 0x24b81b0 @ nc_array.c:59
[2016-05-19 11:16:08.429] nc_stats.c:203 init stats server '127.0.0.1:11213' with 13 metric
[2016-05-19 11:16:08.429] nc_stats.c:235 map 4 stats servers
[2016-05-19 11:16:08.429] nc_stats.c:283 init stats pool 'alpha' with 6 metric and 6 server
[2016-05-19 11:16:08.429] nc_stats.c:333 map 2 stats pools
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(4536) at 0x24b8360 @ nc_stats.c:439
[2016-05-19 11:16:08.429] nc_stats.c:447 stats buffer size 4536
[2016-05-19 11:16:08.429] nc_stats.c:884 m 3 listening on '0.0.0.0:22222'
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(12288) at 0x24b9640 @ nc_epoll.c:39
[2016-05-19 11:16:08.429] nc_util.c:235 malloc(32) at 0x24bcf60 @ nc_epoll.c:48
[2016-05-19 11:16:08.430] nc_epoll.c:63 e 4 with nevent 1024
[2016-05-19 11:16:08.430] nc_util.c:235 malloc(352) at 0x24bc650 @ nc_connection.c:123
[2016-05-19 11:16:08.430] nc_proxy.c:46 ref conn 0x24bc650 owner 0x24ac460 into pool 0
[2016-05-19 11:16:08.430] nc_connection.c:291 get conn 0x24bc650 proxy 1
[2016-05-19 11:16:08.430] nc_proxy.c:221 p 5 listening on '0.0.0.0:22122' in redis pool 0 'beta' with 3 servers
[2016-05-19 11:16:08.430] nc_util.c:235 malloc(352) at 0x24bc7c0 @ nc_connection.c:123
[2016-05-19 11:16:08.430] nc_proxy.c:46 ref conn 0x24bc7c0 owner 0x24ac5f0 into pool 1
[2016-05-19 11:16:08.430] nc_connection.c:291 get conn 0x24bc7c0 proxy 1
[2016-05-19 11:16:08.430] nc_proxy.c:221 p 6 listening on '192.168.1.111:22121' in memcache pool 1 'alpha' with 4 servers
[2016-05-19 11:16:08.430] nc_proxy.c:240 init proxy with 2 pools
[2016-05-19 11:16:08.430] nc_core.c:144 created ctx 0x24ab0b0 id 1










一次请求完整日志:
[2016-05-19 11:16:12.256] nc_epoll.c:255 epoll 0001 triggered on conn 0x24bc7c0
[2016-05-19 11:16:12.256] nc_core.c:331 event 00FF on p 6
[2016-05-19 11:16:12.256] nc_util.c:235 malloc(352) at 0x24bc930 @ nc_connection.c:123
[2016-05-19 11:16:12.256] nc_client.c:50 ref conn 0x24bc930 owner 0x24ac5f0 into pool 'alpha'
[2016-05-19 11:16:12.256] nc_connection.c:250 get conn 0x24bc930 client 1
[2016-05-19 11:16:12.256] nc_stats.c:1071 metric 'client_connections' in pool 1
[2016-05-19 11:16:12.256] nc_stats.c:1088 incr field 'client_connections' to 1
[2016-05-19 11:16:12.256] nc_proxy.c:391 accepted c 8 on p 6 from '192.168.1.111:50487'
[2016-05-19 11:16:12.256] nc_proxy.c:297 accept on p 6 not ready - eagain
[2016-05-19 11:16:12.256] nc_stats.c:1038 swap stats current 0x24ab6b0 shadow 0x24b6440
[2016-05-19 11:16:12.256] nc_epoll.c:255 epoll 0004 triggered on conn 0x24bc930
[2016-05-19 11:16:12.256] nc_core.c:331 event FF00 on c 8
[2016-05-19 11:16:12.256] nc_stats.c:1027 skip swap of current 0x24b6440 shadow 0x24ab6b0 as aggregator is busy
[2016-05-19 11:16:16.540] nc_epoll.c:255 epoll 0001 triggered on conn 0x24bc930
[2016-05-19 11:16:16.540] nc_core.c:331 event 00FF on c 8
[2016-05-19 11:16:16.540] nc_util.c:235 malloc(440) at 0x24bcaa0 @ nc_message.c:210
[2016-05-19 11:16:16.541] nc_util.c:235 malloc(32) at 0x24ab760 @ nc_array.c:29
[2016-05-19 11:16:16.541] nc_util.c:235 malloc(16) at 0x24bcfd0 @ nc_array.c:34
[2016-05-19 11:16:16.541] nc_message.c:324 get msg 0x24bcaa0 id 1 request 1 owner sd 8
[2016-05-19 11:16:16.541] nc_util.c:235 malloc(16384) at 0x24bdf40 @ nc_mbuf.c:47
[2016-05-19 11:16:16.541] nc_mbuf.c:102 get mbuf 0x24c1f10
[2016-05-19 11:16:16.541] nc_mbuf.c:185 insert mbuf 0x24c1f10 len 0
[2016-05-19 11:16:16.541] nc_connection.c:354 recv on sd 8 18 of 16336
[2016-05-19 11:16:16.541] nc_memcache.c:717 parsed req 1 res 3 type 5 state 14 rpos 18 of 18
00000000  73 65 74 20 79 61 6e 67  5f 31 20 30 20 30 20 32   |set yang_1 0 0 2|
00000010  0d 0a                                              |..|
[2016-05-19 11:16:16.541] nc_stats.c:1027 skip swap of current 0x24b6440 shadow 0x24ab6b0 as aggregator is busy
[2016-05-19 11:16:17.108] nc_epoll.c:255 epoll 0001 triggered on conn 0x24bc930
[2016-05-19 11:16:17.108] nc_core.c:331 event 00FF on c 8
[2016-05-19 11:16:17.108] nc_connection.c:354 recv on sd 8 4 of 16318
[2016-05-19 11:16:17.108] nc_memcache.c:729 parsed req 1 res 0 type 5 state 0 rpos 22 of 22
00000000  73 65 74 20 79 61 6e 67  5f 31 20 30 20 30 20 32   |set yang_1 0 0 2|
00000010  0d 0a 62 62 0d 0a                                  |..bb..|
[2016-05-19 11:16:17.109] nc_server.c:718 key 'yang_1' on dist 0 maps to server '127.0.0.1:11210:1'
[2016-05-19 11:16:17.109] nc_util.c:235 malloc(352) at 0x24bcc60 @ nc_connection.c:123
[2016-05-19 11:16:17.109] nc_server.c:62 ref conn 0x24bcc60 owner 0x24acbb8 into '127.0.0.1:11210:1
[2016-05-19 11:16:17.109] nc_connection.c:250 get conn 0x24bcc60 client 0
[2016-05-19 11:16:17.109] nc_server.c:493 connect to server '127.0.0.1:11210:1'
[2016-05-19 11:16:17.109] nc_server.c:535 connecting on s 9 to server '127.0.0.1:11210:1'
[2016-05-19 11:16:17.109] nc_stats.c:1173 metric 'in_queue' in pool 1 server 1
[2016-05-19 11:16:17.109] nc_stats.c:1191 incr field 'in_queue' to 1
[2016-05-19 11:16:17.109] nc_stats.c:1173 metric 'in_queue_bytes' in pool 1 server 1
[2016-05-19 11:16:17.109] nc_stats.c:1222 incr by field 'in_queue_bytes' to 22
[2016-05-19 11:16:17.109] nc_stats.c:1173 metric 'requests' in pool 1 server 1
[2016-05-19 11:16:17.109] nc_stats.c:1191 incr field 'requests' to 1
[2016-05-19 11:16:17.109] nc_stats.c:1173 metric 'request_bytes' in pool 1 server 1
[2016-05-19 11:16:17.109] nc_stats.c:1222 incr by field 'request_bytes' to 22
[2016-05-19 11:16:17.109] nc_request.c:619 forward from c 8 to s 9 req 1 len 22 type 5 with key 'yang_1'
[2016-05-19 11:16:17.109] nc_stats.c:1027 skip swap of current 0x24b6440 shadow 0x24ab6b0 as aggregator is busy
[2016-05-19 11:16:17.109] nc_epoll.c:255 epoll 0004 triggered on conn 0x24bcc60
[2016-05-19 11:16:17.109] nc_core.c:331 event FF00 on s 9
[2016-05-19 11:16:17.109] nc_stats.c:1173 metric 'server_connections' in pool 1 server 1
[2016-05-19 11:16:17.109] nc_stats.c:1191 incr field 'server_connections' to 1
[2016-05-19 11:16:17.109] nc_server.c:574 connected on s 9 to server '127.0.0.1:11210:1'
[2016-05-19 11:16:17.109] nc_request.c:745 send next req 1 len 22 type 5 on s 9
[2016-05-19 11:16:17.109] nc_connection.c:406 sendv on sd 9 22 of 22 in 1 buffers
[2016-05-19 11:16:17.109] nc_request.c:760 send done req 1 len 22 type 5 on s 9
[2016-05-19 11:16:17.109] nc_stats.c:1173 metric 'in_queue' in pool 1 server 1
[2016-05-19 11:16:17.109] nc_stats.c:1206 decr field 'in_queue' to 0
[2016-05-19 11:16:17.109] nc_stats.c:1173 metric 'in_queue_bytes' in pool 1 server 1
[2016-05-19 11:16:17.109] nc_stats.c:1237 decr by field 'in_queue_bytes' to 0
[2016-05-19 11:16:17.109] nc_stats.c:1173 metric 'out_queue' in pool 1 server 1
[2016-05-19 11:16:17.109] nc_stats.c:1191 incr field 'out_queue' to 1
[2016-05-19 11:16:17.109] nc_stats.c:1173 metric 'out_queue_bytes' in pool 1 server 1
[2016-05-19 11:16:17.109] nc_stats.c:1222 incr by field 'out_queue_bytes' to 22
[2016-05-19 11:16:17.109] nc_stats.c:1027 skip swap of current 0x24b6440 shadow 0x24ab6b0 as aggregator is busy
[2016-05-19 11:16:17.109] nc_epoll.c:255 epoll 0001 triggered on conn 0x24bcc60
[2016-05-19 11:16:17.109] nc_core.c:331 event 00FF on s 9
[2016-05-19 11:16:17.110] nc_util.c:235 malloc(440) at 0x24c1f50 @ nc_message.c:210
[2016-05-19 11:16:17.111] nc_util.c:235 malloc(32) at 0x24ab210 @ nc_array.c:29
[2016-05-19 11:16:17.111] nc_util.c:235 malloc(16) at 0x24bde60 @ nc_array.c:34
[2016-05-19 11:16:17.111] nc_message.c:324 get msg 0x24c1f50 id 2 request 0 owner sd 9
[2016-05-19 11:16:17.111] nc_util.c:235 malloc(16384) at 0x24c2110 @ nc_mbuf.c:47
[2016-05-19 11:16:17.111] nc_mbuf.c:102 get mbuf 0x24c60e0
[2016-05-19 11:16:17.111] nc_mbuf.c:185 insert mbuf 0x24c60e0 len 0
[2016-05-19 11:16:17.111] nc_connection.c:354 recv on sd 9 8 of 16336
[2016-05-19 11:16:17.111] nc_memcache.c:1190 parsed rsp 2 res 0 type 15 state 0 rpos 8 of 8
00000000  53 54 4f 52 45 44 0d 0a                            |STORED..|
[2016-05-19 11:16:17.111] nc_stats.c:1173 metric 'out_queue' in pool 1 server 1
[2016-05-19 11:16:17.111] nc_stats.c:1206 decr field 'out_queue' to 0
[2016-05-19 11:16:17.111] nc_stats.c:1173 metric 'out_queue_bytes' in pool 1 server 1
[2016-05-19 11:16:17.111] nc_stats.c:1237 decr by field 'out_queue_bytes' to 0
[2016-05-19 11:16:17.111] nc_stats.c:1173 metric 'responses' in pool 1 server 1
[2016-05-19 11:16:17.111] nc_stats.c:1191 incr field 'responses' to 1
[2016-05-19 11:16:17.111] nc_stats.c:1173 metric 'response_bytes' in pool 1 server 1
[2016-05-19 11:16:17.111] nc_stats.c:1222 incr by field 'response_bytes' to 8
[2016-05-19 11:16:17.111] nc_stats.c:1027 skip swap of current 0x24b6440 shadow 0x24ab6b0 as aggregator is busy
[2016-05-19 11:16:17.111] nc_epoll.c:255 epoll 0004 triggered on conn 0x24bc930
[2016-05-19 11:16:17.111] nc_core.c:331 event FF00 on c 8
[2016-05-19 11:16:17.111] nc_response.c:347 send next rsp 2 on c 8
[2016-05-19 11:16:17.111] nc_connection.c:406 sendv on sd 8 8 of 8 in 1 buffers
[2016-05-19 11:16:17.111] nc_response.c:360 send done rsp 2 on c 8
[2016-05-19 11:16:17.111] nc_request.c:98 req 1 done on c 8 req_time 570.704 msec type REQ_MC_SET narg 2 req_len 22 rsp_len 8 key0 'yang_1' peer '192.168.1.111:50487' done 1 error 0
[2016-05-19 11:16:17.112] nc_message.c:375 put msg 0x24c1f50 id 2
[2016-05-19 11:16:17.112] nc_mbuf.c:194 remove mbuf 0x24c60e0 len 0
[2016-05-19 11:16:17.112] nc_mbuf.c:124 put mbuf 0x24c60e0 len 0
[2016-05-19 11:16:17.112] nc_util.c:281 free(0x24bde60) @ nc_array.c:77
[2016-05-19 11:16:17.112] nc_util.c:281 free(0x24ab210) @ nc_array.c:51
[2016-05-19 11:16:17.112] nc_message.c:375 put msg 0x24bcaa0 id 1
[2016-05-19 11:16:17.112] nc_mbuf.c:194 remove mbuf 0x24c1f10 len 0
[2016-05-19 11:16:17.112] nc_mbuf.c:124 put mbuf 0x24c1f10 len 0
[2016-05-19 11:16:17.112] nc_util.c:281 free(0x24bcfd0) @ nc_array.c:77
[2016-05-19 11:16:17.112] nc_util.c:281 free(0x24ab760) @ nc_array.c:51
[2016-05-19 11:16:17.112] nc_stats.c:1027 skip swap of current 0x24b6440 shadow 0x24ab6b0 as aggregator is busy
^C[.......................] signal 2 (SIGINT) received, exiting

*/
