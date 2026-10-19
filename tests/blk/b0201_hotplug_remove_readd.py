# SPDX-License-Identifier: Apache-2.0
"""
B0201 sidecar. Hot adds a virtio-blk function with id hp0, gives
the guest time to attach, then removes it and adds it back. The
guest test exercises the freshly re added function. All host
actions must complete even if the guest finishes after the first add.
"""
import os
import time


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
    ctx.log("first add-disk completed")

    time.sleep(0.5)
    cp = ctx.vm_api("remove-device", ["hp0"])
    if cp.returncode != 0:
        ctx.log(f"remove-device failed rc={cp.returncode} "
                f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote remove-device failed")
    ctx.log("remove-device completed")

    time.sleep(0.5)
    cp = ctx.vm_api("add-disk",
                    [f"path={hp_disk},readonly=off,id=hp0,image_type=raw"])
    if cp.returncode != 0:
        ctx.log(f"second add-disk failed rc={cp.returncode} "
                f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote second add-disk failed")
    ctx.log("second add-disk completed")
