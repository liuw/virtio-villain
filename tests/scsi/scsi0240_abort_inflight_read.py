# SPDX-License-Identifier: Apache-2.0
"""
SCSI0240 sidecar. Throttles the boot LUN backing drive to one
operation per second so a guest read stays queued long enough to be
aborted by tag. QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
