# SPDX-License-Identifier: Apache-2.0
"""PCI0201 sidecar. Saves VM state while MSI-X vectors are masked."""

from host_helpers import snapshot, snapshot_config

SUPPORTED_BACKENDS = ("ch", "qemu", "openvmm")
run = snapshot
vmm_config = snapshot_config
