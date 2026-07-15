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

After loading settings, the window asks `ProviderResolver` for the selected
provider, calls `Fetch()`, copies the provider name, and releases the provider
instance immediately. Deskbar preview data and wallpaper setter work continue
only when both resolution and fetch return `B_OK`.

### AppSettings

`AppSettings` owns application defaults and persisted settings state.

Current state:

- default provider name
- local folder path
- archive enabled flag
- last image path
- last update date
- Haiku-native flattened `BMessage` persistence
- default storage under `B_USER_SETTINGS_DIRECTORY`
- explicit-path storage seam for isolated smoke tests

Normal application code uses the default settings path. Tests use a temporary
path and do not touch the user's settings file. The local folder path defaults
to empty and is stored as one required `B_STRING_TYPE` field named
`local_folder_path`.

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
stored result. A subsequent `SetInfo()` call replaces all five stored fields.
The provider contract smoke verifies these copy and replacement boundaries and
the neutral state before both its successful and failing provider probes.

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

### LocalFolderProvider

`LocalFolderProvider` is the first filesystem-backed provider.

Current state:

- receives one directory path at construction
- returns `B_BAD_VALUE` for an empty path
- returns `B_ENTRY_NOT_FOUND` for a missing path
- returns `B_NOT_A_DIRECTORY` when the path names a regular file
- resolves the path through `BEntry` before directory enumeration
- enumerates entries with Haiku's Storage Kit
- considers regular `.jpg`, `.jpeg`, and `.png` files
- matches those suffixes without case sensitivity
- asks Haiku's Translation Kit whether each candidate can become a bitmap
- skips files that no installed translator recognizes as an image
- chooses the bytewise lexicographically smallest recognized filename
- returns the filename as title, `Local folder` as source, and the absolute path
- returns `B_ENTRY_NOT_FOUND` when no recognized image is present

Selection is intentionally independent of directory enumeration order.
Translation Kit identification validates image structure without fully decoding
the selected image into application-owned bitmap memory. Recursive traversal,
daily rotation, and application wiring are not part of the current provider.

### ProviderResolver

`ProviderResolver` maps persisted provider identity to one concrete provider
instance.

Current mappings:

```text
Demo provider -> DemoProvider
Local folder  -> LocalFolderProvider(LocalFolderPath())
```

The caller passes an empty output pointer and owns the returned provider.
An occupied output pointer returns `B_BAD_VALUE`. An unknown provider name
returns `B_NAME_NOT_FOUND` without creating an object.

The resolver does not fetch provider data. `MainWindow` owns each resolved
instance only for one synchronous `Fetch()` call and then deletes it.

### HaikuWallpaperContract

`HaikuWallpaperContract` captures the public Tracker background contract
without changing the desktop.

Current state:

- resolves `B_DESKTOP_DIRECTORY` and verifies that it is a directory
- uses the public `<be_apps/Tracker/Background.h>` constants
- identifies `B_BACKGROUND_INFO` as the desktop-node attribute
- builds one value-aligned `BMessage` for an image path
- requests scaled placement at origin `(0, 0)`
- enables icon label outline
- targets all workspaces
- exposes the public `B_RESTORE_BACKGROUND_IMAGE` message code
- writes a prepared message as `B_MESSAGE_TYPE` to a caller-supplied `BNode`
- reads and unflattens that attribute from a caller-supplied `BNode`
- captures an existing attribute as raw type, size, and bytes
- represents a previously missing attribute as a neutral backup
- restores either the exact raw attribute or its previous absence
- rejects missing attributes, wrong types, and incomplete I/O explicitly

The contract does not connect its Desktop target lookup to the write seam and
does not send messages. The attribute roundtrip smoke uses an isolated temporary
file, never the Desktop node. Tracker's application signature remains outside
this public contract because Haiku currently defines it in a private Tracker
header.

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
MainWindow
  -> AppSettings
      <-> flattened BMessage settings file
      -> selected/default provider settings
      -> local folder source path
  -> ProviderResolver
      -> DemoProvider
      -> LocalFolderProvider
  -> one synchronous provider Fetch()
  -> release provider instance

AppSettings
  -> archive preference
  -> last image path and update date

DailyImageProvider
  -> DemoProvider
  -> LocalFolderProvider
      -> Haiku Storage Kit directory enumeration
      -> Haiku Translation Kit image identification
  -> status_t
  -> ProviderResult on B_OK
      -> WallpaperInfo
      -> image path

ProviderResult
  -> DeskbarView tooltip

ProviderResult
  -> HaikuWallpaperContract
      -> public Tracker background BMessage schema
      -> isolated caller-supplied BNode attribute roundtrip
      -> raw attribute backup and restore
      -> Desktop target remains outside the write seam
      -> no Desktop mutation yet
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
- full image decode before local-folder selection
- writing the Tracker background attribute
- notifying Tracker to restore the background
- real wallpaper setting
- Deskbar installation
- archive/gallery browsing

Those will be added only after the internal seams are explicit and testable.
