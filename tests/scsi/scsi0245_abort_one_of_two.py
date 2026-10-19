# SPDX-License-Identifier: Apache-2.0
"""
SCSI0245 sidecar. Throttles the boot LUN so two guest reads stay
queued while one of them is aborted by tag. QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
