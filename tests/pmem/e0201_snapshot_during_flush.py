# SPDX-License-Identifier: Apache-2.0
"""E0201 sidecar. Saves VM state during a pmem flush."""

from host_helpers import snapshot, snapshot_config

SUPPORTED_BACKENDS = ("ch", "qemu", "openvmm")
run = snapshot
vmm_config = snapshot_config
