# SPDX-License-Identifier: Apache-2.0
"""
B0205 sidecar. Grows guest RAM through resize --memory while the
guest keeps block reads in flight, so the block device serves I/O
across a guest memory map change.
"""
SUPPORTED_BACKENDS = ("ch",)


def run(ctx):
    if ctx.backend.name != "ch":
        ctx.skip("backend is not CH")
    if ctx.api_sock is None:
        ctx.skip("api socket disabled")
    if not ctx.wait_text("[vv]", timeout=20.0):
        raise RuntimeError("guest harness banner not seen")

    # Boot RAM is 512M with hotplug_size=256M, so 640M is in range.
    ctx.log("resizing guest memory to 640M")
    cp = ctx.vm_api("resize", ["--memory", "640M"], timeout=30.0)
    if cp.returncode != 0:
        ctx.log(f"resize --memory failed rc={cp.returncode} "
                f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote resize --memory failed")
    ctx.log("guest memory resized to 640M")
