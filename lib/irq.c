/* SPDX-License-Identifier: Apache-2.0 */
#include "irq.h"
#include "vfio.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <unistd.h>

static const char *noiommu_parameter =
    "/sys/module/vfio/parameters/enable_unsafe_noiommu_mode";

static int irq_error(struct vv_irq *irq, const char *operation)
{
    snprintf(irq->error, sizeof(irq->error), "%s: %s", operation,
             strerror(errno));
    return -1;
}

static int write_file(const char *path, const char *value)
{
    int fd = open(path, O_WRONLY | O_CLOEXEC);
    if (fd < 0)
        return -1;
    size_t length = strlen(value);
    ssize_t written = write(fd, value, length);
    int saved = errno;
    if (written >= 0 && (size_t)written != length)
        saved = EIO;
    int closed = close(fd);
    if (written < 0 || (size_t)written != length) {
        errno = saved;
        return -1;
    }
    return closed;
}

static int set_override(struct vv_irq *irq, const char *value)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/driver_override", irq->slot);
    return write_file(path, value);
}

static int enable_noiommu(struct vv_irq *irq)
{
    int fd = open(noiommu_parameter, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        if (errno == ENOENT) {
            snprintf(irq->error, sizeof(irq->error),
                     "guest kernel lacks builtin VFIO no-IOMMU support");
            return VV_IRQ_UNAVAILABLE;
        }
        return irq_error(irq, "read guest VFIO no-IOMMU mode");
    }
    char value[4];
    ssize_t length = read(fd, value, sizeof(value));
    int saved = errno;
    int closed = close(fd);
    if (length < 0 || closed < 0) {
        if (length < 0)
            errno = saved;
        return irq_error(irq, "read guest VFIO no-IOMMU mode");
    }
    if (length != 2 || (value[0] != 'N' && value[0] != 'Y') ||
        value[1] != '\n') {
        errno = EPROTO;
        return irq_error(irq, "invalid guest VFIO no-IOMMU mode");
    }
    if (value[0] == 'N') {
        if (write_file(noiommu_parameter, "Y\n") < 0)
            return irq_error(irq, "enable guest VFIO no-IOMMU mode");
        irq->noiommu_changed = 1;
    }
    return 0;
}

static const char *device_name(struct vv_irq *irq)
{
    return strrchr(irq->slot, '/') + 1;
}

static int unbind(struct vv_irq *irq)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/driver/unbind", irq->slot);
    return write_file(path, device_name(irq));
}

int vv_irq_close(struct vv_irq *irq)
{
    int failed = 0;
    if (irq->enabled) {
        struct vfio_irq_set disable = {
            .argsz = sizeof(disable),
            .flags = VFIO_IRQ_SET_DATA_NONE | VFIO_IRQ_SET_ACTION_TRIGGER,
            .index = VFIO_PCI_MSIX_IRQ_INDEX,
        };
        if (ioctl(irq->device, VFIO_DEVICE_SET_IRQS, &disable) < 0) {
            irq_error(irq, "disable VFIO MSI-X");
            failed = -1;
        }
        irq->enabled = 0;
    }
    int *fds[] = { &irq->device, &irq->event[0], &irq->event[1],
                   &irq->event[2], &irq->event[3] };
    for (unsigned i = 0; i < sizeof(fds) / sizeof(fds[0]); i++) {
        if (*fds[i] >= 0 && close(*fds[i]) < 0) {
            irq_error(irq, "close VFIO IRQ descriptor");
            failed = -1;
        }
        *fds[i] = -1;
    }
    if (irq->attached &&
        ioctl(irq->group, VFIO_GROUP_UNSET_CONTAINER) < 0) {
        irq_error(irq, "detach VFIO group");
        failed = -1;
    }
    irq->attached = 0;
    if (irq->group >= 0 && close(irq->group) < 0) {
        irq_error(irq, "close VFIO group");
        failed = -1;
    }
    irq->group = -1;
    if (irq->container >= 0 && close(irq->container) < 0) {
        irq_error(irq, "close VFIO container");
        failed = -1;
    }
    irq->container = -1;
    if (irq->bound && unbind(irq) < 0) {
        irq_error(irq, "unbind VFIO driver");
        failed = -1;
    }
    irq->bound = 0;
    if (irq->override_set && set_override(irq, irq->override) < 0) {
        irq_error(irq, "restore PCI driver override");
        failed = -1;
    }
    irq->override_set = 0;
    if (irq->driver[0]) {
        char path[256];
        snprintf(path, sizeof(path), "/sys/bus/pci/drivers/%s/bind", irq->driver);
        if (write_file(path, device_name(irq)) < 0) {
            irq_error(irq, "restore PCI driver binding");
            failed = -1;
        }
        irq->driver[0] = '\0';
    }
    if (irq->noiommu_changed && write_file(noiommu_parameter, "N\n") < 0) {
        irq_error(irq, "restore guest VFIO no-IOMMU mode");
        failed = -1;
    }
    irq->noiommu_changed = 0;
    if (failed)
        printf("vv-irq-close %s\n", irq->error);
    return failed;
}

int vv_irq_open(struct vv_irq *irq, const char *slot, unsigned vectors)
{
    memset(irq, 0, sizeof(*irq));
    irq->container = irq->group = irq->device = -1;
    for (unsigned i = 0; i < VV_IRQ_MAX_VECTORS; i++)
        irq->event[i] = -1;
    if (getpid() != 1) {
        errno = EPERM;
        return irq_error(irq, "VFIO observation requires guest init");
    }
    const char *prefix = "/sys/bus/pci/devices/";
    if (!vectors || vectors > VV_IRQ_MAX_VECTORS ||
        strncmp(slot, prefix, strlen(prefix)) ||
        strlen(slot) >= sizeof(irq->slot) ||
        strchr(slot + strlen(prefix), '/')) {
        errno = EINVAL;
        return irq_error(irq, "invalid VFIO IRQ request");
    }
    strcpy(irq->slot, slot);
    irq->vectors = vectors;
    int enabled = enable_noiommu(irq);
    if (enabled != 0)
        return enabled;
    irq->container = open("/dev/vfio/vfio", O_RDWR | O_CLOEXEC);
    if (irq->container < 0) {
        irq_error(irq, "open guest VFIO container");
        goto fail;
    }
    int version = ioctl(irq->container, VFIO_GET_API_VERSION);
    if (version != VFIO_API_VERSION) {
        if (version >= 0)
            errno = EPROTO;
        irq_error(irq, "VFIO API version");
        goto fail;
    }
    int extension = ioctl(irq->container, VFIO_CHECK_EXTENSION,
                          VFIO_NOIOMMU_IOMMU);
    if (extension <= 0) {
        if (extension >= 0)
            errno = ENOTSUP;
        irq_error(irq, "VFIO no-IOMMU extension");
        goto fail;
    }
    char path[256], link[256];
    snprintf(path, sizeof(path), "%s/driver_override", slot);
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        irq_error(irq, "read PCI driver override");
        goto fail;
    }
    ssize_t length = read(fd, irq->override, sizeof(irq->override) - 1);
    int saved = errno;
    int closed = close(fd);
    if (length <= 0 || length == sizeof(irq->override) - 1 || closed < 0) {
        errno = length < 0 ? saved : EIO;
        irq_error(irq, "read PCI driver override");
        goto fail;
    }
    irq->override[length] = '\0';
    if (!strcmp(irq->override, "(null)\n"))
        strcpy(irq->override, "\n");
    snprintf(path, sizeof(path), "%s/driver", slot);
    length = readlink(path, link, sizeof(link) - 1);
    if (length >= 0) {
        link[length] = '\0';
        const char *driver = strrchr(link, '/');
        if (!driver || strlen(driver + 1) >= sizeof(irq->driver)) {
            errno = EOVERFLOW;
            irq_error(irq, "PCI driver symlink");
            goto fail;
        }
        if (unbind(irq) < 0) {
            irq_error(irq, "unbind PCI driver");
            goto fail;
        }
        strcpy(irq->driver, driver + 1);
    } else if (errno != ENOENT) {
        irq_error(irq, "read PCI driver binding");
        goto fail;
    }
    if (set_override(irq, "vfio-pci\n") < 0) {
        irq_error(irq, "set VFIO driver override");
        goto fail;
    }
    irq->override_set = 1;
    if (write_file("/sys/bus/pci/drivers_probe", device_name(irq)) < 0) {
        irq_error(irq, "bind guest VFIO PCI driver");
        goto fail;
    }
    irq->bound = 1;
    snprintf(path, sizeof(path), "%s/iommu_group", slot);
    length = readlink(path, link, sizeof(link) - 1);
    if (length < 0 || length == sizeof(link) - 1) {
        if (length >= 0)
            errno = EOVERFLOW;
        irq_error(irq, "read VFIO group");
        goto fail;
    }
    link[length] = '\0';
    char *group = strrchr(link, '/');
    char *end;
    errno = 0;
    unsigned long number = group ? strtoul(group + 1, &end, 10) : 0;
    if (!group || !group[1] || errno || *end) {
        errno = EPROTO;
        irq_error(irq, "invalid VFIO group symlink");
        goto fail;
    }
    snprintf(path, sizeof(path), "/dev/vfio/noiommu-%lu", number);
    irq->group = open(path, O_RDWR | O_CLOEXEC);
    if (irq->group < 0) {
        irq_error(irq, "open VFIO no-IOMMU group");
        goto fail;
    }
    struct vfio_group_status status = { .argsz = sizeof(status) };
    if (ioctl(irq->group, VFIO_GROUP_GET_STATUS, &status) < 0) {
        irq_error(irq, "query VFIO group status");
        goto fail;
    }
    if (!(status.flags & VFIO_GROUP_FLAGS_VIABLE)) {
        errno = EBUSY;
        irq_error(irq, "VFIO group is not viable");
        goto fail;
    }
    if (ioctl(irq->group, VFIO_GROUP_SET_CONTAINER, &irq->container) < 0) {
        irq_error(irq, "attach VFIO group");
        goto fail;
    }
    irq->attached = 1;
    if (ioctl(irq->container, VFIO_SET_IOMMU, VFIO_NOIOMMU_IOMMU) < 0) {
        irq_error(irq, "select guest no-IOMMU mode");
        goto fail;
    }
    irq->device = ioctl(irq->group, VFIO_GROUP_GET_DEVICE_FD, device_name(irq));
    if (irq->device < 0) {
        irq_error(irq, "open VFIO PCI device");
        goto fail;
    }
    struct vfio_irq_info info = {
        .argsz = sizeof(info), .index = VFIO_PCI_MSIX_IRQ_INDEX,
    };
    if (ioctl(irq->device, VFIO_DEVICE_GET_IRQ_INFO, &info) < 0) {
        irq_error(irq, "query VFIO MSI-X vectors");
        goto fail;
    }
    if (!(info.flags & VFIO_IRQ_INFO_EVENTFD) || info.count < vectors) {
        errno = ENOSPC;
        irq_error(irq, "VFIO MSI-X eventfds unavailable");
        goto fail;
    }
    size_t bytes = sizeof(struct vfio_irq_set) + vectors * sizeof(int32_t);
    struct vfio_irq_set *set = calloc(1, bytes);
    if (!set) {
        irq_error(irq, "allocate VFIO IRQ request");
        goto fail;
    }
    set->argsz = bytes;
    set->flags = VFIO_IRQ_SET_DATA_EVENTFD | VFIO_IRQ_SET_ACTION_TRIGGER;
    set->index = VFIO_PCI_MSIX_IRQ_INDEX;
    set->count = vectors;
    for (unsigned i = 0; i < vectors; i++) {
        irq->event[i] = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if (irq->event[i] < 0) {
            irq_error(irq, "allocate MSI-X eventfd");
            free(set);
            goto fail;
        }
        int32_t event = irq->event[i];
        memcpy(set->data + i * sizeof(event), &event, sizeof(event));
    }
    int configured = ioctl(irq->device, VFIO_DEVICE_SET_IRQS, set);
    saved = errno;
    free(set);
    if (configured < 0) {
        errno = saved;
        irq_error(irq, "install VFIO MSI-X handlers");
        goto fail;
    }
    irq->enabled = 1;
    return 0;
fail:
    printf("vv-irq-open %s\n", irq->error);
    vv_irq_close(irq);
    return -1;
}

int vv_irq_read(struct vv_irq *irq, unsigned vector, uint64_t *count)
{
    if (vector >= irq->vectors || irq->event[vector] < 0) {
        errno = EINVAL;
        return irq_error(irq, "invalid MSI-X eventfd");
    }
    uint64_t value;
    ssize_t bytes;
    do {
        bytes = read(irq->event[vector], &value, sizeof(value));
    } while (bytes < 0 && errno == EINTR);
    if (bytes == sizeof(value)) {
        if (UINT64_MAX - irq->count[vector] < value) {
            errno = EOVERFLOW;
            return irq_error(irq, "MSI-X interrupt count overflow");
        }
        irq->count[vector] += value;
    } else if (bytes < 0 && errno == EAGAIN) {
        /* An empty nonblocking eventfd means no interrupt was delivered. */
    } else {
        if (bytes >= 0)
            errno = EIO;
        return irq_error(irq, "read MSI-X eventfd");
    }
    *count = irq->count[vector];
    return 0;
}
