#ifndef LWIPOPTS_H
#define LWIPOPTS_H

// Include standard error definitions
#include <errno.h>

// Prevent timeval redefinition
#define LWIP_TIMEVAL_PRIVATE 0

// Set path to sys_arch.h
#define LWIP_ARCH_PATH "arch/"

// Required for CYW43 threadsafe background mode
#define NO_SYS 1

// Disable threading and synchronization features since NO_SYS=1
#define LWIP_NETCONN 0
#define LWIP_SOCKET 0
#define LWIP_COMPAT_SOCKETS 0
#define LWIP_NETIF_API 0

// Disable API message macros that are causing compilation errors
#define API_MSG_M_DEF(m)  m
#define API_MSG_M_DEF_C(t)  const t
#define API_MSG_M_DEF_SEM(s)  s

// Basic lwIP configuration
#define LWIP_IPV4 1
#define LWIP_IPV6 0
#define LWIP_TCP 1
#define LWIP_UDP 1
#define LWIP_DNS 1
#define LWIP_DHCP 1
#define LWIP_AUTOIP 1

// Memory configuration
#define MEMP_NUM_TCP_PCB 10
#define MEMP_NUM_TCP_SEG 20
#define TCP_WND 2048
#define TCP_SND_BUF 2048

// HTTP server configuration
#define LWIP_HTTPD 1
#define LWIP_HTTPD_MAX_TAG_NAME_LEN 8
#define LWIP_HTTPD_MAX_TAG_INSERT_LEN 192
#define LWIP_HTTPD_POST_MAX_PAYLOAD_LEN 1024

// Network interface configuration
#define LWIP_NETIF_HOSTNAME 1
#define LWIP_NETIF_TX_SINGLE_PBUF 1

// Debug configuration
#define LWIP_DEBUG 0
#define LWIP_STATS 0

// Timeouts
#define TCP_TMR_INTERVAL 250
#define TCP_MSL 60000

#endif // LWIPOPTS_H
