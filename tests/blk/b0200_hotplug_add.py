# SPDX-License-Identifier: Apache-2.0
"""
B0200 sidecar. Hot adds a virtio-blk function on the host side
while the guest waits for a new PCI device. CH only.
"""
import os

SUPPORTED_BACKENDS = ("ch",)


def run(ctx):
    if ctx.backend.name != "ch":
        ctx.skip("backend is not CH")
    if ctx.api_sock is None:
        ctx.skip("api socket disabled")

    if not ctx.wait_text("[vv]", timeout=20.0):
        raise RuntimeError("guest harness banner not seen")

    hp_disk = os.path.join(ctx.tmpdir, "hp.raw")
    with open(hp_disk, "wb") as f:
        f.truncate(16 * 1024 * 1024)
    ctx.log(f"created disk image {hp_disk}")

    cp = ctx.vm_api("add-disk",
                    [f"path={hp_disk},readonly=off,id=hp0,image_type=raw"])
    if cp.returncode != 0:
        ctx.log(f"add-disk failed rc={cp.returncode} "
                f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote add-disk failed")
    ctx.log("add-disk completed")
