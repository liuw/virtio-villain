# SPDX-License-Identifier: Apache-2.0
"""
SCSI0254 sidecar. Throttles the boot LUN so a guest eight block read
stays queued long enough to be aborted by tag. QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
