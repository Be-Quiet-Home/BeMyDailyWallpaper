# Phase 0005: Partial settings policy

Date: 2026-07-14
Baseline commit: `52425d6 Test corrupt settings handling`

## Scope

This phase defines and proves the policy for incomplete flattened settings
messages.

A settings message is valid only when all current fields exist with their
expected types:

```text
provider_name      string
archive_enabled    bool
last_image_path    string
last_update_date   string
```

Unknown additional fields do not invalidate the message.

## Atomic load contract

`AppSettings::LoadFrom()` validates every required field before applying any
value to the object.

Expected behavior:

```text
complete valid message
    -> B_OK
    -> all four values are applied

missing or mistyped required field
    -> non-B_OK status
    -> no current value is changed
```

This prevents a damaged or incomplete file from creating a mixed state composed
of old values and selectively loaded new values.

## Smoke fixture

The settings smoke writes a valid flattened `BMessage` containing only:

```text
provider_name = Partial provider
```

The other three required fields are absent.

Before loading, the target object contains four explicit preserved values.

The smoke requires:

```text
LoadFrom() != B_OK
all four preserved values remain unchanged
```

The temporary file remains under `B_SYSTEM_TEMP_DIRECTORY` and is later
overwritten by the successful full round-trip case.

## Command

```sh
make smoke-settings
```

Expected output remains:

```text
BeMyDailyWall settings roundtrip smoke: ok
```

## What this phase proves

This phase proves that:

- a decodable but incomplete settings message is rejected,
- required fields are validated before object state is changed,
- failure preserves all current values,
- a missing field cannot cause selective settings application,
- the existing corrupt-file and complete round-trip cases still share the same
  isolated temporary-file lifecycle.

## What this phase does not prove

This phase does not prove that:

- every wrong field type has a dedicated smoke case,
- settings contain a schema version,
- older or newer schemas can be migrated,
- unknown additional fields are explicitly smoke-tested,
- saving is atomic,
- a crash during saving preserves the previous file,
- invalid production settings are quarantined or repaired automatically,
- final user-facing recovery text exists,
- the application is release-ready.

Smoke tests prove life, not dignity.
