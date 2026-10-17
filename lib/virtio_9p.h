/* SPDX-License-Identifier: Apache-2.0 */
/*
 * virtio_9p.h - Shared virtio-9p and 9P2000.L wire definitions.
 *
 * The virtio feature and configuration layout follow the Linux virtio-9p
 * UAPI. Message types and structures follow the 9P2000.L protocol used by
 * the Linux virtio transport and the supported VMM device models.
 */
#ifndef VV_VIRTIO_9P_H
#define VV_VIRTIO_9P_H

#include <stdint.h>

#define VIRTIO_9P_F_MOUNT_TAG 0

#define P9_RLERROR   7
#define P9_TGETATTR  24
#define P9_RGETATTR  25
#define P9_TVERSION 100
#define P9_RVERSION 101
#define P9_TATTACH  104
#define P9_RATTACH  105
#define P9_TWALK    110
#define P9_RWALK    111
#define P9_NOTAG    0xffff
#define P9_NOFID    0xffffffffu
#define P9_MAXWELEM 16

#define P9_VERSION "9P2000.L"
#define P9_VERSION_LEN 8
#define P9_QID_TYPE_DIRECTORY 0x80

struct p9_header {
    uint32_t size;
    uint8_t type;
    uint16_t tag;
} __attribute__((packed));

struct p9_version {
    struct p9_header header;
    uint32_t msize;
    uint16_t version_len;
    uint8_t version[P9_VERSION_LEN];
} __attribute__((packed));

struct p9_qid {
    uint8_t type;
    uint32_t version;
    uint64_t path;
} __attribute__((packed));

struct p9_getattr {
    struct p9_header header;
    uint32_t fid;
    uint64_t request_mask;
} __attribute__((packed));

#endif /* VV_VIRTIO_9P_H */
