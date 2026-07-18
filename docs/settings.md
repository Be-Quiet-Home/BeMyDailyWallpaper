# Settings

BeMyDailyWall keeps settings ownership inside `AppSettings`.

Settings logic must not spread into:

- `MainWindow`
- providers
- `WallpaperSetter`
- `DeskbarView`

Those components may read already-loaded values or receive settings-derived
state, but they must not own persistence rules.

## Current implementation

`AppSettings` currently supports:

- provider name
- local folder path
- archive enabled flag
- startup apply enabled flag
- last image path
- last update date

## Defaults

Current default values:

```text
provider_name          = Demo provider
local_folder_path      =
archive_enabled       = false
startup_apply_enabled = false
last_image_path        =
last_update_date       =
```

## Persistence

Settings are stored under Haiku's user settings directory.

The settings file name is:

```text
BeMyDailyWall_settings
```

The implementation uses Haiku-native APIs:

```text
B_USER_SETTINGS_DIRECTORY
BPath
BFile
BMessage::Flatten()
BMessage::Unflatten()
```

The file format is a flattened `BMessage`.

No JSON, INI, XML, or custom parser is used for settings.

## Storage paths

Normal application code uses:

```text
Load()
Save()
```

Those methods resolve `B_USER_SETTINGS_DIRECTORY` and use the normal
`BeMyDailyWall_settings` file.

The explicit-path methods:

```text
LoadFrom(const BPath&)
SaveTo(const BPath&)
```

provide the same persistence contract for a caller-supplied path. They are an
internal storage seam used by the settings smoke so tests never overwrite or
delete the user's real settings file.

They do not create a user-facing alternate settings location.

## Load behavior

`AppSettings::Load()` and `AppSettings::LoadFrom()` return a Haiku `status_t`.

Expected states:

- `B_OK`: settings were loaded
- `B_ENTRY_NOT_FOUND`: no settings file exists yet; defaults remain active
- other error: loading failed; values remain unchanged when the flattened
  `BMessage` cannot be decoded or a known field violates its contract

Loading is atomic at the `AppSettings` object boundary. All required fields and
the optional startup field are validated before any field is applied.

The five established fields remain required. A partial settings message is
rejected and must not selectively overwrite current values.

`startup_apply_enabled` is an optional migration field. A settings file written
before this field existed loads with the safe default `false`. When present, it
must appear exactly once as `B_BOOL_TYPE`; a wrong type or duplicate is rejected
without changing current values. Every new save writes the field.

Every required field must also appear exactly once with the expected type. The
reader never relies on implicit first-value or last-value selection.

Unknown additional fields are ignored because they do not invalidate the known
settings contract. This permits forward-compatible readers to preserve the
known settings subset while newer writers add unrelated fields.

`MainWindow` shows the default-path load state as a diagnostic status and
exposes `startup_apply_enabled` through one native checkbox.

## Save behavior

`AppSettings::Save()` and `AppSettings::SaveTo()` write all current settings
fields, including `startup_apply_enabled`, to the selected settings file.

The settings file is created if missing and replaced when saving.

`SaveTo()` writes a sibling file whose path ends in `.tmp`. The new message is
flattened and synchronized there before `BEntry::Rename()` replaces the target.

The existing target is not erased before the temporary file is complete. A
failure before the rename leaves the previous settings file unchanged.

Message field insertion, temporary-file opening, `BMessage::Flatten()`,
`BNode::Sync()`, and rename failures are returned through `status_t`.

A failed temporary write is cleaned up when possible. A successful rename leaves
no `.tmp` sibling behind.

This is a rename-based replacement contract. It does not claim parent-directory
sync or complete crash durability across every filesystem and power-loss point.

## Smoke behavior

`make smoke-settings` uses a temporary settings file under
`B_SYSTEM_TEMP_DIRECTORY`. It verifies:

- missing-file behavior and unchanged defaults
- corrupt flattened-message behavior and unchanged current values
- partial-message rejection and unchanged current values
- wrong-field-type rejection and unchanged current values
- successful legacy loading when `startup_apply_enabled` is absent
- successful loading when unknown additional fields are present
- wrong-type and duplicate startup-field rejection with unchanged current values
- duplicate required-field rejection and unchanged current values
- preservation of an existing file when the temporary path cannot be opened
- successful rename-based replacement with no temporary sibling left behind
- a complete save/load round trip for all six fields
- cleanup of the temporary file

The smoke does not touch:

```text
B_USER_SETTINGS_DIRECTORY/BeMyDailyWall_settings
```

## Current boundary

There is no separate Preferences window yet.

`MainWindow` exposes one native checkbox for `startup_apply_enabled`. It reads
the already-loaded value and saves every user change immediately through
`AppSettings::Save()`.

A save failure restores both the prior in-memory value and the visible checkbox
state. The checkbox does not own the flattened-message schema or file
replacement rules.

No startup code reads the flag and no Desktop mutation is enabled by the
checkbox in this phase.

Persistence remains a small explicit seam so preference handling does not leak
into unrelated components.
