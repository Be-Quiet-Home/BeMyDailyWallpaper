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
````

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

## Load behavior

`AppSettings::Load()` returns a Haiku `status_t`.

Expected states:

* `B_OK`: settings were loaded
* `B_ENTRY_NOT_FOUND`: no settings file exists yet; defaults remain active
* other error: loading failed; defaults remain active unless individual fields
  were already changed before loading

`MainWindow` currently shows the load state as a diagnostic status.

## Save behavior

`AppSettings::Save()` writes all current settings fields to the settings file.

The settings file is created if missing and overwritten when saving.

## Current boundary

There is no Preferences window yet.

There is no UI for changing settings yet.

Persistence exists only as a small explicit seam so future preference handling
does not leak into unrelated components.
