/* SPDX-License-Identifier: Apache-2.0 */
#ifndef VV_VFIO_H
#define VV_VFIO_H

/* Linux VFIO userspace ABI subset, independent of libc kernel headers. */
#include <stdint.h>
#include <sys/ioctl.h>

#define VFIO_API_VERSION 0
#define VFIO_NOIOMMU_IOMMU 8
#define VFIO_PCI_MSIX_IRQ_INDEX 2
#define VFIO_GET_API_VERSION       _IO(';', 100)
#define VFIO_CHECK_EXTENSION       _IO(';', 101)
#define VFIO_SET_IOMMU             _IO(';', 102)
#define VFIO_GROUP_GET_STATUS      _IO(';', 103)
#define VFIO_GROUP_SET_CONTAINER   _IO(';', 104)
#define VFIO_GROUP_UNSET_CONTAINER _IO(';', 105)
#define VFIO_GROUP_GET_DEVICE_FD   _IO(';', 106)
#define VFIO_DEVICE_GET_IRQ_INFO   _IO(';', 109)
#define VFIO_DEVICE_SET_IRQS       _IO(';', 110)

#define VFIO_GROUP_FLAGS_VIABLE      (1u << 0)
#define VFIO_IRQ_INFO_EVENTFD        (1u << 0)
#define VFIO_IRQ_SET_DATA_NONE       (1u << 0)
#define VFIO_IRQ_SET_DATA_EVENTFD    (1u << 2)
#define VFIO_IRQ_SET_ACTION_TRIGGER  (1u << 5)

struct vfio_group_status {
    uint32_t argsz;
    uint32_t flags;
};

struct vfio_irq_info {
    uint32_t argsz;
    uint32_t flags;
    uint32_t index;
    uint32_t count;
};

struct vfio_irq_set {
    uint32_t argsz;
    uint32_t flags;
    uint32_t index;
    uint32_t start;
    uint32_t count;
    uint8_t data[];
};

#endif
