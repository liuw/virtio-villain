# SPDX-License-Identifier: Apache-2.0
"""
SCSI0252 sidecar. Throttles the boot LUN so a guest read on LUN 0
stays queued while an ABORT TASK SET targets a different LUN.
QEMU only.
"""

from scsi.scsi_helpers import throttle as run

SUPPORTED_BACKENDS = ("qemu",)
