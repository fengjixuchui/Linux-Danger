/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SCHED_NUMA_BALANCING_H
#define _LINUX_SCHED_NUMA_BALANCING_H

/*
 * This is the interface between the scheduler and the MM that
 * implements memory access pattern based NUMA-balancing:
 */

#include <linux/sched.h>

#define TNF_MIGRATED	0x01
#define TNF_NO_GROUP	0x02
#define TNF_SHARED	0x04
#define TNF_FAULT_LOCAL	0x08
#define TNF_MIGRATE_FAIL 0x10

#ifdef CONFIG_NUMA_BALANCING
extern void set_numabalancing_state(bool enabled);
#else
static inline void set_numabalancing_state(bool enabled)
{
}
#endif

#endif /* _LINUX_SCHED_NUMA_BALANCING_H */
