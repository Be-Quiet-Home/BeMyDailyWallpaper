# Architecture

BeMyDailyWall is a small Haiku-native daily wallpaper companion.

It is built in pedantic mode: small explicit components, clear ownership,
visible status paths, and no premature feature jumps.

## Current components

### App

`BeMyDailyWallApp` owns the Haiku application lifecycle.

It creates the main window during startup.

### MainWindow

`MainWindow` is the current visible test bench.

It shows:

- application liveness
- settings status
- Deskbar icon preview
- provider status
- wallpaper setter status

The window is not the final product center. It is a development and diagnostic
surface while the small system parts are being wired together.

Its ordinary child views are arranged through Haiku's `BGroupLayout` and
`BLayoutBuilder`. Fixed `BRect` coordinates are not used for labels or the
Deskbar preview.

The window-owned diagnostic sentences use Haiku's Locale Kit with the
`MainWindow` translation context. Provider metadata and setter-owned error text
remain owned by their source components.

### AppSettings

`AppSettings` owns application defaults and persisted settings state.

Current state:

- default provider name
- archive enabled flag
- last image path
- last update date
- Haiku-native flattened `BMessage` persistence
- default storage under `B_USER_SETTINGS_DIRECTORY`
- explicit-path storage seam for isolated smoke tests

Normal application code uses the default settings path. Tests use a temporary
path and do not touch the user's settings file.

### DeskbarView

`DeskbarView` is the future Deskbar/Replicant view.

Current state:

- draws a small placeholder icon
- accepts `WallpaperInfo`
- exposes the wallpaper information through a tooltip
- is currently previewed inside `MainWindow`
- provides a layout-aware 32 x 32 constructor for that preview

The placeholder circle is not the final icon.

### WallpaperInfo

`WallpaperInfo` stores user-facing wallpaper metadata:

- title
- description
- source
- copyright / attribution
- date

Its default constructor leaves every metadata field empty. Demo or provider
content must enter through an explicit provider result and is never implied by
container construction.

It builds the tooltip text used by `DeskbarView`.

The tooltip header remains the product name. Complete title, source, and date
lines are localized by `WallpaperInfo`; provider metadata is inserted after
catalog lookup.

### ProviderResult

`ProviderResult` carries provider output:

- `WallpaperInfo`
- image path

A default `ProviderResult` therefore contains neutral empty metadata and an
empty image path. `SetInfo()` copies all five metadata fields into result-owned
storage; later replacement of the source `WallpaperInfo` does not change the
stored result. The provider contract smoke verifies this copy boundary and the
neutral state before both its successful and failing provider probes.

The image path is optional and may remain empty after a successful
metadata-only provider fetch. `SetImagePath()` replaces the stored string;
`HasImagePath()` reflects whether that string is non-empty. The provider
contract smoke verifies setting and clearing this state.

Only a result returned with `B_OK` is consumed; this contract does not require
providers to preserve the input object after a failed fetch.

### DailyImageProvider

`DailyImageProvider` is the provider interface.

Providers supply daily image metadata and later image files.

### DemoProvider

`DemoProvider` is the current dry test provider.

It returns localized user-facing demo metadata, an explicitly empty date, and
no image path. The provider contract smoke covers all five `WallpaperInfo`
fields and verifies image-path emptiness through both `ImagePath()` and
`HasImagePath()` after a successful fetch.

Its `Name()` value remains the stable provider identifier used by settings and
source metadata.

### WallpaperSetter

`WallpaperSetter` is the wallpaper application interface.

Current state:

- accepts `ProviderResult`
- checks whether an image path exists
- returns Haiku `status_t`
- stores a human-readable last error message
- owns translation of its component-specific errors

The real backend is not implemented yet.

## Current data flow

```text
AppSettings
  <-> flattened BMessage settings file
  -> selected/default provider settings
  -> archive preference
  -> last image path and update date

DailyImageProvider
  -> status_t
  -> ProviderResult on B_OK
      -> WallpaperInfo
      -> image path

ProviderResult
  -> DeskbarView tooltip

ProviderResult
  -> WallpaperSetter
      -> status_t / error message
      -> MainWindow status display
```

## Build system

BeMyDailyWall uses Haiku's Generic Makefile / makefile-engine.

This is the default build policy for the project because it is native enough for
a standalone Haiku application while keeping the project small and readable.

Jam is reserved for cases where code is integrated into an existing Jam-based
Haiku project or where matching Haiku source tree conventions is explicitly
required.

## Near-term boundaries

The project currently does not implement:

- network access
- real wallpaper download
- real wallpaper setting
- Deskbar installation
- archive/gallery browsing

Those will be added only after the internal seams are explicit and testable.
