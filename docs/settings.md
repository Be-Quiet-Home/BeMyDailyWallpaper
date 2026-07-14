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
- archive enabled flag
- last image path
- last update date

## Defaults

Current default values:

```text
provider_name      = Demo provider
archive_enabled   = false
last_image_path    =
last_update_date   =
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
- other error: loading failed; defaults remain active unless individual fields
  were already changed before loading

`MainWindow` currently shows the default-path load state as a diagnostic status.

## Save behavior

`AppSettings::Save()` and `AppSettings::SaveTo()` write all current settings
fields to the selected settings file.

The settings file is created if missing and overwritten when saving.

Message field insertion, file opening, and `BMessage::Flatten()` failures are
returned through `status_t`.

## Smoke behavior

`make smoke-settings` writes a temporary settings file under
`B_SYSTEM_TEMP_DIRECTORY`, loads it into a fresh `AppSettings` object, verifies
all four fields, and removes the temporary file.

The smoke does not touch:

```text
B_USER_SETTINGS_DIRECTORY/BeMyDailyWall_settings
```

## Current boundary

There is no Preferences window yet.

There is no UI for changing settings yet.

Persistence exists only as a small explicit seam so future preference handling
does not leak into unrelated components.
