# SPDX-License-Identifier: Apache-2.0
"""
V0200 sidecar. Hot adds a vsock function with cid 44 and id hp_vsock.
"""

SUPPORTED_BACKENDS = ("ch",)


def run(ctx):
    if ctx.backend.name != "ch":
        ctx.skip("backend is not CH")
    if ctx.api_sock is None:
        ctx.skip("api socket disabled")
    if not ctx.wait_text("[vv]", timeout=20.0):
        raise RuntimeError("guest harness banner not seen")

    cp = ctx.vm_api("add-vsock", ["cid=44,id=hp_vsock"])
    if cp.returncode != 0:
        raise RuntimeError(
            f"add-vsock failed rc={cp.returncode} "
            f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
    ctx.log("add-vsock completed")
