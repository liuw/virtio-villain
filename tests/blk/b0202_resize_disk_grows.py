# SPDX-License-Identifier: Apache-2.0
"""
B0202 sidecar. Grows the boot disk on the host through resize-disk.
The guest watches device_cfg.capacity and config_generation.
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

    cur = os.path.getsize(ctx.disk)
    new = cur + 16 * 1024 * 1024
    ctx.log(f"resizing disk from {cur} to {new} bytes")
    cp = ctx.vm_api("resize-disk",
                    ["--disk", "_disk0", "--size", str(new)],
                    timeout=30.0)
    if cp.returncode != 0:
        ctx.log(f"resize-disk failed rc={cp.returncode} "
                f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote resize-disk failed")
    ctx.log("resize-disk completed")
