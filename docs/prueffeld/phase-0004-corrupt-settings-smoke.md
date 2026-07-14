# Phase 0004: Corrupt settings smoke

Date: 2026-07-14
Baseline commit: `1687be1 Add settings roundtrip smoke`

## Scope

This phase extends the existing settings smoke with a controlled corrupt-file
case.

The test remains isolated under `B_SYSTEM_TEMP_DIRECTORY` and does not touch the
user's real settings file.

## Command

```sh
make smoke-settings
```

Expected output remains:

```text
BeMyDailyWall settings roundtrip smoke: ok
```

The existing target now proves missing-file, corrupt-file, and full round-trip
behavior in one temporary-file lifecycle.

## Corrupt fixture

The smoke writes plain bytes that are not a flattened `BMessage`:

```text
not a flattened BMessage
```

The exact error code returned by `BMessage::Unflatten()` is intentionally not
fixed by the test.

The required contract is:

```text
LoadFrom() returns a value other than B_OK.
The AppSettings object keeps all values it had before LoadFrom().
```

## Preserved values

Before loading the corrupt file, the smoke assigns:

```text
provider_name      = Preserved provider
archive_enabled    = true
last_image_path    = /boot/home/preserved-wallpaper.jpg
last_update_date   = 2026-07-13
```

Every value must remain unchanged after the failed load.

The same temporary file is then overwritten through `SaveTo()` and used for the
normal successful round trip.

## What this phase proves

This phase proves that:

- an existing non-`BMessage` settings file does not report `B_OK`,
- an unflatten failure occurs before any settings field is applied,
- current values remain unchanged after corrupt-file loading fails,
- `SaveTo()` can overwrite the corrupt fixture with a valid settings message,
- the existing successful round trip still works afterward,
- the corrupt fixture stays outside the user settings directory.

## What this phase does not prove

This phase does not prove that:

- every possible malformed flattened `BMessage` is rejected,
- partially valid settings messages are accepted or rejected consistently,
- settings fields have a schema version,
- unknown future fields are handled through a migration policy,
- saving is atomic,
- a crash during saving preserves the previous settings file,
- corrupt production settings are quarantined or repaired automatically,
- final user-facing recovery text exists,
- the application is release-ready.

Smoke tests prove life, not dignity.
