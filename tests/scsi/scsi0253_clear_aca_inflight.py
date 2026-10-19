# SPDX-License-Identifier: Apache-2.0
"""
SCSI0253 sidecar. Throttles the boot LUN so a guest read stays queued
while a CLEAR ACA is issued. QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
