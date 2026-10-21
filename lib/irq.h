/* SPDX-License-Identifier: Apache-2.0 */
#ifndef VV_IRQ_H
#define VV_IRQ_H

#include <stdint.h>

#define VV_IRQ_MAX_VECTORS 4
#define VV_IRQ_UNAVAILABLE 1

struct vv_irq {
    int container;
    int group;
    int device;
    int event[VV_IRQ_MAX_VECTORS];
    uint64_t count[VV_IRQ_MAX_VECTORS];
    unsigned vectors;
    int attached;
    int enabled;
    int bound;
    int override_set;
    int noiommu_changed;
    char slot[128];
    char driver[128];
    char override[128];
    char error[256];
};

/* Only guest init may change guest VFIO state. Never configure host VFIO. */
int vv_irq_open(struct vv_irq *irq, const char *slot, unsigned vectors);
int vv_irq_close(struct vv_irq *irq);
int vv_irq_read(struct vv_irq *irq, unsigned vector, uint64_t *count);

#endif
