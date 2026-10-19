# SPDX-License-Identifier: Apache-2.0
"""
SCSI0235 sidecar. Hot adds a scsi-hd logical unit on the running
virtio-scsi controller so the device delivers a transport reset
event to the guest event queue. QEMU only.
"""

from scsi.scsi_helpers import hot_add, wait_armed

SUPPORTED_BACKENDS = ("qemu",)


def run(ctx):
    wait_armed(ctx)
    hot_add(ctx)
    ctx.log("hot add completed")
