# SPDX-License-Identifier: Apache-2.0
"""
V0200 sidecar. Hot adds a vsock function with cid 44 and id hp_vsock.
"""
import os

from host_helpers import wait_ready

SUPPORTED_BACKENDS = ("ch",)


def vmm_config():
    return {"opts": {"vsock_enabled": False}}


def run(ctx):
    wait_ready(ctx)

    socket = os.path.join(ctx.tmpdir, "vsock.sock")
    cp = ctx.vm_api("add-vsock", [f"cid=44,id=hp_vsock,socket={socket}"])
    if cp.returncode != 0:
        raise RuntimeError(
            f"add-vsock failed rc={cp.returncode} "
            f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
    ctx.log("add-vsock completed")
