# SPDX-License-Identifier: Apache-2.0
"""
B0205 sidecar. Grows guest RAM through resize --memory while the
guest keeps block reads in flight, so the block device serves I/O
across a guest memory map change.
"""
from host_helpers import qmp, wait_ready

SUPPORTED_BACKENDS = ("ch", "qemu")


def run(ctx):
    wait_ready(ctx)

    if ctx.backend.name == "qemu":
        qmp(ctx, "object-add", {
            "qom-type": "memory-backend-ram", "id": "hpmem",
            "size": 128 * 1024 * 1024,
        })
        qmp(ctx, "device_add", {
            "driver": "pc-dimm", "id": "hp_dimm", "memdev": "hpmem",
        })
        ctx.log("added 128M guest memory")
        return

    # Boot RAM is 512M with hotplug_size=256M, so 640M is in range.
    ctx.log("resizing guest memory to 640M")
    cp = ctx.vm_api("resize", ["--memory", "640M"], timeout=30.0)
    if cp.returncode != 0:
        ctx.log(f"resize --memory failed rc={cp.returncode} "
                f"stdout={cp.stdout!r} stderr={cp.stderr!r}")
        raise RuntimeError("ch-remote resize --memory failed")
    ctx.log("guest memory resized to 640M")
