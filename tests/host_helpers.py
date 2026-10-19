# SPDX-License-Identifier: Apache-2.0
"""Shared checked host actions."""


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
