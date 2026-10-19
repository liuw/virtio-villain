# SPDX-License-Identifier: Apache-2.0
"""
SCSI0247 sidecar. Throttles the boot LUN so a guest read stays queued
long enough to be terminated by an I_T NEXUS RESET. QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
