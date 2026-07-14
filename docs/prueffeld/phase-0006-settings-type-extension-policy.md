# Phase 0006: Settings type and extension policy

Date: 2026-07-14
Baseline commit: `8661b18 Reject partial settings messages`

## Scope

This phase proves the remaining current settings-message validation rules:

```text
required field with wrong type
    -> reject the message
    -> preserve every current value

complete required fields plus unknown fields
    -> accept the message
    -> load the four known values
```

No production code changes are required. The atomic `LoadFrom()` implementation
already provides these behaviors.

## Wrong-type fixture

The smoke writes every required field, but stores:

```text
archive_enabled = "true"
```

as a string instead of a boolean.

The other required fields use their expected types.

Before loading, the target object contains four explicit preserved values.

Expected result:

```text
LoadFrom() != B_OK
all four preserved values remain unchanged
```

The test does not require one specific Haiku error value. It requires only a
non-`B_OK` result and unchanged object state.

## Unknown-field fixture

The smoke writes all four required fields correctly and adds:

```text
future_note     string
future_revision int32
```

Expected result:

```text
LoadFrom() == B_OK
all four known values are loaded
unknown fields are ignored
```

This is a small forward-compatibility rule. Readers validate the fields they own
without rejecting unrelated additions.

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

- a required boolean stored as a string is rejected,
- wrong-type rejection preserves every current value,
- validation remains atomic when the message is otherwise complete,
- unknown string and `int32` fields do not invalidate a complete message,
- known fields load correctly when unknown additions are present,
- the existing missing, corrupt, partial, and round-trip cases remain active.

## What this phase does not prove

This phase does not prove that:

- every required field has an individual wrong-type case,
- duplicate fields have an explicit policy,
- settings contain a schema version,
- future schema migrations work,
- unknown fields survive a subsequent `SaveTo()` rewrite,
- saving is atomic,
- a crash during saving preserves the previous file,
- invalid production settings are quarantined or repaired automatically,
- final user-facing recovery text exists,
- the application is release-ready.

Smoke tests prove life, not dignity.
