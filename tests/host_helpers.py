# SPDX-License-Identifier: Apache-2.0
"""Shared checked host actions."""

import os
import time


def wait_ready(ctx):
    if ctx.backend.name not in ("ch", "qemu", "qemu-mmio", "openvmm"):
        ctx.skip("backend is not CH, QEMU, QEMU MMIO or OpenVMM")
    if ctx.api_sock is None:
        ctx.skip("api socket disabled")
    if not ctx.wait_text("[vv]", timeout=20.0):
        raise RuntimeError("guest harness banner not seen")


def qmp(ctx, command, arguments=None):
    response = ctx.vm_api(command, arguments)
    ctx.log(f"{command}: {response}")
    if (not isinstance(response, dict) or "error" in response
            or "return" not in response):
        raise RuntimeError(f"QMP {command} failed: {response!r}")
    return response


def ch_command(ctx, command, arguments=None):
    response = ctx.vm_api(command, arguments)
    if response.returncode != 0:
        ctx.log(f"{command} failed rc={response.returncode} "
                f"stdout={response.stdout!r} stderr={response.stderr!r}")
        client = "OpenVMM" if ctx.backend.name == "openvmm" else "ch-remote"
        raise RuntimeError(f"{client} {command} failed")
    return response


def add_disk(ctx):
    hp_disk = os.path.join(ctx.tmpdir, "hp.raw")
    with open(hp_disk, "wb") as image:
        image.truncate(16 * 1024 * 1024)
    if ctx.backend.name == "ch":
        ch_command(ctx, "add-disk",
                   [f"path={hp_disk},readonly=off,id=hp0,image_type=raw"])
    else:
        qmp(ctx, "blockdev-add", {
            "driver": "raw", "node-name": "hpnode",
            "file": {"driver": "file", "filename": hp_disk},
        })
        readd_disk(ctx)
    ctx.log("add-disk completed")


def readd_disk(ctx):
    qmp(ctx, "device_add", {
        "driver": "virtio-blk-pci-non-transitional",
        "drive": "hpnode", "id": "hp0",
    })


def remove_disk(ctx):
    if ctx.backend.name == "ch":
        ch_command(ctx, "remove-device", ["hp0"])
    else:
        qmp(ctx, "device_del", {"id": "hp0"})
        deadline = time.monotonic() + 5
        while True:
            reply = qmp(ctx, "qom-list", {"path": "/machine/peripheral"})
            devices = reply["return"]
            if not isinstance(devices, list) or any(
                    not isinstance(device, dict)
                    or not isinstance(device.get("name"), str)
                    for device in devices):
                raise RuntimeError(f"QMP qom-list failed: {reply!r}")
            if not any(device["name"] == "hp0" for device in devices):
                break
            if time.monotonic() >= deadline:
                raise TimeoutError("QEMU did not complete device_del for hp0")
            time.sleep(0.02)
    ctx.log("remove-device completed")
