# Phase 0046: Startup apply setting

Date: 2026-07-18
Baseline commit: `4879cdf Prove startup action coordination`

## Scope

This phase adds one persisted user-decision field to `AppSettings`:

```text
startup_apply_enabled
```

The field defaults to `false`. No UI or startup execution path reads it yet.

## Native persistence

The existing Haiku-native storage contract remains unchanged:

```text
B_USER_SETTINGS_DIRECTORY
BMessage
BFile
Flatten / Unflatten
rename-based replacement
```

No parser, schema library, or external dependency is added.

## Public model terminal

```cpp
bool StartupApplyEnabled() const;
void SetStartupApplyEnabled(bool enabled);
```

The setting is stored as one `B_BOOL_TYPE` field named:

```text
startup_apply_enabled
```

## Safe default

```text
new AppSettings object
missing settings file
legacy settings file without the field
    -> startup apply disabled
```

Adding the setting therefore cannot silently enable a Desktop mutation.

## Migration rule

Older settings files predate the new key. Rejecting those files would discard
provider, folder, archive, and wallpaper history state. The reader therefore
treats only this new field as optional:

```text
field absent
    -> B_OK
    -> false

field present exactly once as bool
    -> load the stored value

field present with wrong type or duplicate values
    -> B_BAD_DATA
    -> current AppSettings values remain unchanged
```

Every current save writes the field, providing a silent forward migration.

The five previously established fields remain required. Unknown extra fields
remain ignored.

## Atomic load boundary

All required fields and the optional startup field are validated and read into
local variables before any setter is called. A malformed startup field cannot
partially update the object.

## Smoke extension

`make smoke-settings` additionally proves:

```text
default startup apply is false
legacy file without startup key loads as false
wrong startup field type is rejected atomically
duplicate startup values are rejected atomically
save failure preserves a previously stored true value
true survives a complete save/load round trip
```

The smoke continues to use only a temporary path under
`B_SYSTEM_TEMP_DIRECTORY`.

## Local authority note

Haiku's `BMessage::GetInfo(name, ...)` contract reports
`B_NAME_NOT_FOUND` when a field name is absent. The optional migration helper
uses that specific result; all other lookup failures remain errors.

## Locale impact

No user-visible string changes.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-settings
make smoke
```

No visual gate is required because application behavior is unchanged.

## Proves

- startup apply has a safe persisted default
- existing settings files remain readable
- malformed new fields fail atomically
- current saves migrate the file format
- no UI or startup mutation is introduced

## Does not prove

- a user can change the setting through the UI
- MainWindow reads the setting
- startup invokes DailyWallpaperStartupPlan
- the Desktop changes automatically
- scheduling, retry, or midnight behavior

Smoke tests prove life, not dignity.
