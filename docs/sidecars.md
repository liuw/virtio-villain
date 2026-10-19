# Host side sidecars

Some tests need a host action while the guest is running, such as adding or removing a device, changing its configuration, pausing the VM or saving its state. A sidecar coordinates that action with the guest test.

The guest checks device behavior before and after the action. A passing result requires both the host action and the guest checks to complete. A failed or unfinished host action cannot count as a pass.

A test is skipped when the required device or host control is unavailable. A skip means the scenario was not exercised. It does not prove that the VMM cannot support it.

## How these tests differ from VMM integration tests

VMM integration tests usually check that a guest operating system continues to work after a host action. Virtio Villain accesses the device directly instead of relying on the normal guest driver. It checks whether the device follows the virtio specification, including behavior that a driver might tolerate or work around.

This lets a test check that a newly added device starts in a clean state, that removing a device does not disturb another device or that saving and restoring state preserves the expected device behavior.
