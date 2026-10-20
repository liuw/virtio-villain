/* SPDX-License-Identifier: Apache-2.0 */
#include "../lib/mmio.h"
#include "../lib/virtio_pci.h"

#define PROBE(width) \
uint##width##_t probe_read##width(const volatile void *addr) \
{ \
    return vv_mmio_read##width(addr); \
} \
void probe_write##width(volatile void *addr, uint##width##_t value) \
{ \
    vv_mmio_write##width(addr, value); \
}

PROBE(8)
PROBE(16)
PROBE(32)

uint64_t probe_read64(const volatile void *addr)
{
    return virtio_load64(addr);
}

void probe_write64(volatile void *addr, uint64_t value)
{
    virtio_store64(addr, value);
}

void probe_read_bytes(void *dst, const volatile void *src, size_t len)
{
    vv_mmio_read_bytes(dst, src, len);
}
