# SPDX-License-Identifier: Apache-2.0
"""
SCSI0248 sidecar. Throttles the boot LUN so a guest read stays queued
while a QUERY TASK asks whether the task is present. QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
