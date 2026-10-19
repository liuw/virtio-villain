# SPDX-License-Identifier: Apache-2.0
"""
SCSI0250 sidecar. Throttles the boot LUN so a guest read on the second
request queue stays queued long enough to be aborted. QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
