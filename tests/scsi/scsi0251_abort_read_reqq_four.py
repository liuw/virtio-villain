# SPDX-License-Identifier: Apache-2.0
"""
SCSI0251 sidecar. Throttles the boot LUN so a guest read on the third
request queue stays queued long enough to be aborted. QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
