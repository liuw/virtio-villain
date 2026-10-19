# SPDX-License-Identifier: Apache-2.0
"""
N0200 sidecar. Hot adds a virtio-net function through ch-remote.
Requires permission to create a tap interface on the host.
"""

SUPPORTED_BACKENDS = ("ch",)


def run(ctx):
    if ctx.backend.name != "ch":
        ctx.skip("backend is not CH")
    if ctx.api_sock is None:
        ctx.skip("api socket disabled")
    if not ctx.wait_text("[vv]", timeout=20.0):
        raise RuntimeError("guest harness banner not seen")

    cp = ctx.vm_api("add-net", ["tap=,id=hp_net,mac="])
    if cp.returncode != 0:
        ctx.log(f"add-net failed rc={cp.returncode} "
                f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote add-net failed")
    ctx.log("add-net completed")
