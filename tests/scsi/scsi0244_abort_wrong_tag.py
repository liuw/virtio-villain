# SPDX-License-Identifier: Apache-2.0
"""
SCSI0244 sidecar. Throttles the boot LUN so a guest read stays queued
while a mismatched ABORT TASK is issued. QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
