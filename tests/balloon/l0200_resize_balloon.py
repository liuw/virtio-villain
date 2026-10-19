# SPDX-License-Identifier: Apache-2.0
"""
L0200 sidecar. Resizes the balloon target on the host. The guest
watches num_pages and config_generation for the change.
"""

SUPPORTED_BACKENDS = ("ch",)


def run(ctx):
    if ctx.backend.name != "ch":
        ctx.skip("backend is not CH")
    if ctx.api_sock is None:
        ctx.skip("api socket disabled")
    if not ctx.wait_text("[vv]", timeout=20.0):
        raise RuntimeError("guest harness banner not seen")

    cp = ctx.vm_api("resize", ["--balloon", "64M"])
    if cp.returncode != 0:
        ctx.log(f"resize --balloon failed rc={cp.returncode} "
                f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote resize --balloon failed")
    ctx.log("balloon resized to 64M")
