# SPDX-License-Identifier: Apache-2.0
"""
SCSI0246 sidecar. Throttles the boot LUN so a guest read stays queued
long enough to be terminated by a LOGICAL UNIT RESET. QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
