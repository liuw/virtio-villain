# SPDX-License-Identifier: Apache-2.0
"""L0201 sidecar. Saves VM state during balloon inflation."""

from host_helpers import snapshot, snapshot_config

SUPPORTED_BACKENDS = ("ch", "qemu", "openvmm")
run = snapshot


def vmm_config():
    return snapshot_config("balloon")
