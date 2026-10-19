# SPDX-License-Identifier: Apache-2.0
"""
SCSI0256 sidecar. Throttles the boot LUN so a guest read stays queued
long enough to be aborted before a fresh read is issued. QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
