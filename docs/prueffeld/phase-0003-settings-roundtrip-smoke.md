# Phase 0003: Settings round-trip smoke

Date: 2026-07-14
Baseline commit: `36d3be0 Add provider contract smoke`

## Scope

This phase proves the current `AppSettings` persistence contract without
touching the user's real settings file.

The production default remains:

```text
B_USER_SETTINGS_DIRECTORY/BeMyDailyWall_settings
```

The smoke uses an explicit temporary path under `B_SYSTEM_TEMP_DIRECTORY`.

## Commands

Run only the settings smoke:

```sh
make smoke-settings
```

Run every current smoke:

```sh
make smoke
```

Expected output includes:

```text
BeMyDailyWall settings roundtrip smoke: ok
```

## Storage seam

`AppSettings` now provides two storage layers.

Default-path operations:

```text
Load()
Save()
```

Explicit-path operations:

```text
LoadFrom(const BPath&)
SaveTo(const BPath&)
```

The default methods resolve the normal Haiku user settings path and delegate to
the explicit-path methods.

The explicit-path methods keep serialization behavior testable without adding a
test mode or alternate settings preference to the application.

## Smoke cases

### Missing file

The smoke starts with a nonexistent temporary file.

Expected result:

```text
LoadFrom() returns B_ENTRY_NOT_FOUND.
All default values remain unchanged.
```

### Full round trip

The smoke writes and reloads:

```text
provider_name      = Roundtrip provider
archive_enabled    = true
last_image_path    = /boot/home/test-wallpaper.jpg
last_update_date   = 2026-07-14
```

Every value must survive the flattened `BMessage` round trip.

### Cleanup

The temporary file name contains the smoke process ID.

The smoke removes a stale same-name file before use and removes the temporary
file again during shutdown.

## What this phase proves

This phase proves that:

- a missing settings file reports `B_ENTRY_NOT_FOUND`,
- missing-file loading leaves current defaults unchanged,
- all four settings fields can be flattened and unflattened,
- `SaveTo()` creates and writes the selected file,
- `LoadFrom()` reads the selected file,
- message field insertion errors are no longer ignored by `SaveTo()`,
- the smoke uses Haiku's system temporary directory,
- the smoke does not use the normal user settings path,
- `make smoke` includes the settings round-trip check.

## What this phase does not prove

This phase does not prove that:

- saving is atomic,
- a crash during saving preserves the previous settings file,
- corrupt or schema-incompatible settings recover cleanly,
- partial settings messages should be accepted or rejected,
- future settings migrations work,
- permission and disk-full failures show final user-facing text,
- a Preferences window exists,
- the application is release-ready.

Smoke tests prove life, not dignity.
