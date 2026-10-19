# SPDX-License-Identifier: Apache-2.0
"""
RNG0200 sidecar. Pauses, snapshots, resumes the VM through ch-remote.
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

    snap_dir = os.path.join(ctx.tmpdir, "snap")
    os.makedirs(snap_dir, exist_ok=True)
    cp = ctx.vm_api("pause")
    if cp.returncode != 0:
        ctx.log(f"pause failed rc={cp.returncode} "
                f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote pause failed")
    try:
        cp = ctx.vm_api("snapshot", [f"file://{snap_dir}"])
        if cp.returncode != 0:
            ctx.log(f"snapshot failed rc={cp.returncode} "
                    f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
            raise RuntimeError("ch-remote snapshot failed")
    finally:
        cp = ctx.vm_api("resume")
        if cp.returncode != 0:
            ctx.log(f"resume failed rc={cp.returncode} "
                    f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
            raise RuntimeError("ch-remote resume failed")
    ctx.log("snapshot completed")
