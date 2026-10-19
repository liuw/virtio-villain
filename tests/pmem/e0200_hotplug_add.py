# SPDX-License-Identifier: Apache-2.0
"""
E0200 sidecar. Hot adds a virtio-pmem region through ch-remote.
Backs it with a fresh sparse file in ctx.tmpdir.
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

    pmem_file = os.path.join(ctx.tmpdir, "hp_pmem.raw")
    size = 16 * 1024 * 1024
    with open(pmem_file, "wb") as f:
        f.truncate(size)
    cp = ctx.vm_api("add-pmem",
                    [f"file={pmem_file},size={size},id=hp_pmem"])
    if cp.returncode != 0:
        ctx.log(f"add-pmem failed rc={cp.returncode} "
                f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote add-pmem failed")
    ctx.log("add-pmem completed")
