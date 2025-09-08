#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

// Minimal implementation for Raspberry Pi Pico W
#include "pico/stdlib.h"

// Define sys_sem_t, sys_mutex_t, sys_mbox_t and sys_thread_t
typedef void* sys_sem_t;
typedef void* sys_mutex_t;
typedef void* sys_mbox_t;
typedef void* sys_thread_t;

// Define the sys_prot_t type
typedef int sys_prot_t;

#endif /* LWIP_ARCH_SYS_ARCH_H */