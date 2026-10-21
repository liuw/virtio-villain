/* SPDX-License-Identifier: Apache-2.0 */
#include "lib/irq.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>

int main(void)
{
    assert(getpid() != 1);
    struct vv_irq irq;
    assert(vv_irq_open(&irq, "/sys/bus/pci/devices/0000:00:01.0", 2) == -1);
    assert(errno == EPERM);
    assert(strstr(irq.error, "guest init"));
    assert(irq.container == -1 && irq.group == -1 && irq.device == -1);
    assert(vv_irq_close(&irq) == 0);

    irq.vectors = 2;
    for (unsigned i = 0; i < 2; i++) {
        irq.event[i] = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        assert(irq.event[i] >= 0);
    }
    uint64_t count = 99;
    assert(vv_irq_read(&irq, 0, &count) == 0 && count == 0);
    uint64_t value = 3;
    assert(write(irq.event[0], &value, sizeof(value)) == sizeof(value));
    assert(vv_irq_read(&irq, 0, &count) == 0 && count == 3);
    assert(vv_irq_read(&irq, 0, &count) == 0 && count == 3);
    assert(vv_irq_read(&irq, 1, &count) == 0 && count == 0);
    value = 5;
    assert(write(irq.event[0], &value, sizeof(value)) == sizeof(value));
    value = 2;
    assert(write(irq.event[1], &value, sizeof(value)) == sizeof(value));
    assert(vv_irq_read(&irq, 0, &count) == 0 && count == 8);
    assert(vv_irq_read(&irq, 1, &count) == 0 && count == 2);
    irq.count[0] = UINT64_MAX - 1;
    assert(write(irq.event[0], &value, sizeof(value)) == sizeof(value));
    assert(vv_irq_read(&irq, 0, &count) == -1 && errno == EOVERFLOW);
    assert(strstr(irq.error, "overflow"));
    assert(vv_irq_read(&irq, 2, &count) == -1 && errno == EINVAL);
    assert(irq.count[1] == 2);
    int first = irq.event[0], second = irq.event[1];
    assert(vv_irq_close(&irq) == 0);
    assert(fcntl(first, F_GETFD) == -1 && errno == EBADF);
    assert(fcntl(second, F_GETFD) == -1 && errno == EBADF);
    assert(vv_irq_close(&irq) == 0);
    assert(vv_irq_read(&irq, 0, &count) == -1 && errno == EINVAL);
    puts("IRQ observer host checks passed");
    return 0;
}
