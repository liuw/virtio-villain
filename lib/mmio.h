/* SPDX-License-Identifier: Apache-2.0 */
#ifndef VV_MMIO_H
#define VV_MMIO_H

#include <stddef.h>
#include <stdint.h>

/*
 * Raw native endian accesses with compiler barriers, not hardware barriers.
 * The caller supplies an address aligned for the requested width.
 * ARM64 uses plain loads and stores without address writeback so KVM gets
 * valid syndrome information for MMIO emulation.
 */
static inline uint8_t vv_mmio_read8(const volatile void *addr)
{
#if defined(__aarch64__)
    uint32_t value;
    __asm__ volatile("ldrb %w0, [%1]"
                     : "=r"(value) : "r"(addr) : "memory");
    return (uint8_t)value;
#else
    __asm__ volatile("" ::: "memory");
    uint8_t value = *(const volatile uint8_t *)addr;
    __asm__ volatile("" ::: "memory");
    return value;
#endif
}

static inline uint16_t vv_mmio_read16(const volatile void *addr)
{
#if defined(__aarch64__)
    uint32_t value;
    __asm__ volatile("ldrh %w0, [%1]"
                     : "=r"(value) : "r"(addr) : "memory");
    return (uint16_t)value;
#else
    __asm__ volatile("" ::: "memory");
    uint16_t value = *(const volatile uint16_t *)addr;
    __asm__ volatile("" ::: "memory");
    return value;
#endif
}

static inline uint32_t vv_mmio_read32(const volatile void *addr)
{
#if defined(__aarch64__)
    uint32_t value;
    __asm__ volatile("ldr %w0, [%1]"
                     : "=r"(value) : "r"(addr) : "memory");
    return value;
#else
    __asm__ volatile("" ::: "memory");
    uint32_t value = *(const volatile uint32_t *)addr;
    __asm__ volatile("" ::: "memory");
    return value;
#endif
}

static inline void vv_mmio_write8(volatile void *addr, uint8_t value)
{
#if defined(__aarch64__)
    __asm__ volatile("strb %w0, [%1]"
                     : : "r"((uint32_t)value), "r"(addr) : "memory");
#else
    __asm__ volatile("" ::: "memory");
    *(volatile uint8_t *)addr = value;
    __asm__ volatile("" ::: "memory");
#endif
}

static inline void vv_mmio_write16(volatile void *addr, uint16_t value)
{
#if defined(__aarch64__)
    __asm__ volatile("strh %w0, [%1]"
                     : : "r"((uint32_t)value), "r"(addr) : "memory");
#else
    __asm__ volatile("" ::: "memory");
    *(volatile uint16_t *)addr = value;
    __asm__ volatile("" ::: "memory");
#endif
}

static inline void vv_mmio_write32(volatile void *addr, uint32_t value)
{
#if defined(__aarch64__)
    __asm__ volatile("str %w0, [%1]"
                     : : "r"(value), "r"(addr) : "memory");
#else
    __asm__ volatile("" ::: "memory");
    *(volatile uint32_t *)addr = value;
    __asm__ volatile("" ::: "memory");
#endif
}

/* Read exactly len bytes without widening device accesses. */
static inline void vv_mmio_read_bytes(void *dst, const volatile void *src,
                                      size_t len)
{
    uint8_t *out = dst;
    const volatile uint8_t *in = src;
    for (size_t i = 0; i < len; i++)
        out[i] = vv_mmio_read8(in + i);
}

#endif /* VV_MMIO_H */
