# SPDX-License-Identifier: Apache-2.0
"""
V0201 sidecar. Pauses then resumes the running VM.
"""
from host_helpers import pause_resume

SUPPORTED_BACKENDS = ("ch", "qemu", "qemu-mmio", "openvmm")
run = pause_resume


def vmm_config():
    return {"opts": {"mmio_device": "vsock"}}
