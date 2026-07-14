# Phase 0024: Local folder path settings contract

Date: 2026-07-14
Baseline commit: `f8aaf87 Define local folder path statuses`

## Scope

This phase adds the local image source path to `AppSettings`.

It stores provider configuration but does not yet construct or select
`LocalFolderProvider` in `MainWindow`.

## Settings value

New value:

```text
local folder path
```

Public access:

```cpp
const BString& LocalFolderPath() const;
void SetLocalFolderPath(const char* folderPath);
```

The default value is an empty string.

An empty value means that no local source folder has been configured.

## BMessage field

Persisted field:

```text
name: local_folder_path
type: B_STRING_TYPE
count: exactly one
```

The field joins the existing strict settings contract:

```text
provider_name
local_folder_path
archive_enabled
last_image_path
last_update_date
```

All five known fields are required and must have the expected type and exactly
one value.

Unknown additional fields remain ignored.

## Atomic load boundary

`LoadFrom()` validates all five fields and extracts all five temporary values
before changing the current `AppSettings` object.

Corrupt, partial, mistyped, or duplicate settings therefore preserve the
existing local folder path along with every other in-memory value.

## Atomic save boundary

`SaveTo()` adds `local_folder_path` to the same flattened `BMessage` that is
written to the sibling temporary file, synchronized, closed, and renamed over
the target.

A failed replacement preserves the previous local folder path in the protected
settings file.

A successful replacement leaves no temporary file behind.

## Compatibility boundary

Settings files created before this phase contain only four required fields.

They are treated as incomplete and rejected atomically. This is intentional
during the pre-product development phase.

No silent migration or implicit filesystem default is introduced without a
versioned settings policy.

## Smoke extension

The existing settings smoke now verifies the local folder path in:

```text
missing-file defaults
corrupt-file preservation
partial-file preservation
wrong-type preservation
unknown-field compatibility
duplicate-field preservation
failed atomic replacement
successful round trip
```

Fixtures whose intended failure is unrelated to the new field include a valid
`local_folder_path`, so their original failure reason remains meaningful.

## Build impact

Changed production files:

```text
src/AppSettings.h
src/AppSettings.cpp
```

Changed smoke:

```text
tests/settings_roundtrip_smoke.cpp
```

No Makefile target, linked library, catalog key, or visible UI is changed.

## Validation

```sh
make clean
make
make smoke-settings
make smoke
```

No visual gate or catalog regeneration is required.

## What this phase proves

This phase proves that:

- the local source folder has an explicit settings owner,
- its neutral default is empty,
- it survives a settings round trip,
- invalid loads cannot partially replace it,
- failed atomic saves preserve it,
- unknown future fields remain compatible.

## What this phase does not prove

This phase does not prove that:

- the stored path exists,
- the stored path names a directory,
- the UI can choose a folder,
- `MainWindow` constructs `LocalFolderProvider`,
- local provider selection is persisted,
- the application changes the wallpaper,
- the application is release-ready.

Smoke tests prove life, not dignity.
