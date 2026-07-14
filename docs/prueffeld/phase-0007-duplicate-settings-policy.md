# Phase 0007: Duplicate settings policy

Date: 2026-07-14
Baseline commit: `48f394d Test settings field compatibility`

## Scope

This phase defines and proves the policy for duplicate required settings
values.

Each current field must exist exactly once with its expected type:

```text
provider_name      one string
archive_enabled    one bool
last_image_path    one string
last_update_date   one string
```

A message containing zero or more than one value for any required field is
invalid.

## Native validation

`AppSettings::LoadFrom()` uses `BMessage::GetInfo()` to inspect both:

```text
type_code
value count
```

before reading or applying any field.

Expected behavior:

```text
expected type and count == 1
    -> field passes structural validation

wrong type or count != 1
    -> B_BAD_DATA
    -> no AppSettings value changes
```

The policy does not depend on which duplicate value `FindString()` or
`FindBool()` would otherwise return.

## Smoke fixture

The smoke writes a complete flattened `BMessage` containing two string values
under:

```text
provider_name
```

The remaining required fields occur once with their expected types.

Before loading, the target object contains four explicit preserved values.

The smoke requires:

```text
LoadFrom() != B_OK
all four preserved values remain unchanged
```

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

- duplicate values for a required field are detected,
- duplicate-field rejection does not depend on value order,
- structural validation occurs before object state changes,
- duplicate-field failure preserves every current value,
- missing, corrupt, partial, wrong-type, extension, and round-trip cases remain
  active.

## What this phase does not prove

This phase does not prove that:

- every required field has an individual duplicate case,
- duplicate unknown fields are rejected,
- settings contain a schema version,
- future schema migrations work,
- saving is atomic,
- a crash during saving preserves the previous file,
- invalid production settings are quarantined or repaired automatically,
- final user-facing recovery text exists,
- the application is release-ready.

Smoke tests prove life, not dignity.
