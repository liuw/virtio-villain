# SPDX-License-Identifier: Apache-2.0
"""
C0200 sidecar. Pauses and resumes the VM around a console TX flush.
"""
import time

SUPPORTED_BACKENDS = ("ch",)


def run(ctx):
    if ctx.backend.name != "ch":
        ctx.skip("backend is not CH")
    if ctx.api_sock is None:
        ctx.skip("api socket disabled")
    if not ctx.wait_text("[vv]", timeout=20.0):
        raise RuntimeError("guest harness banner not seen")

    cp = ctx.vm_api("pause")
    if cp.returncode != 0:
        ctx.log(f"pause failed rc={cp.returncode} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote pause failed")
    ctx.log("paused")
    time.sleep(0.5)
    cp = ctx.vm_api("resume")
    if cp.returncode != 0:
        ctx.log(f"resume failed rc={cp.returncode} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote resume failed")
    ctx.log("resumed")
