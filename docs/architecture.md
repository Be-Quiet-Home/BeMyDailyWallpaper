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
- Deskbar icon preview
- provider status
- wallpaper setter status

The window is not the final product center. It is a development and diagnostic
surface while the small system parts are being wired together.

### DeskbarView

`DeskbarView` is the future Deskbar/Replicant view.

Current state:

- draws a small placeholder icon
- accepts `WallpaperInfo`
- exposes the wallpaper information through a tooltip
- is currently previewed inside `MainWindow`

The placeholder circle is not the final icon.

### WallpaperInfo

`WallpaperInfo` stores user-facing wallpaper metadata:

- title
- description
- source
- copyright / attribution
- date

It can build the tooltip text used by `DeskbarView`.

### ProviderResult

`ProviderResult` carries provider output:

- `WallpaperInfo`
- image path

The image path is currently optional and may be empty.

### DailyImageProvider

`DailyImageProvider` is the provider interface.

Providers supply daily image metadata and later image files.

### DemoProvider

`DemoProvider` is the current dry test provider.

It returns stable demo metadata and no image path.

### WallpaperSetter

`WallpaperSetter` is the wallpaper application interface.

Current state:

- accepts `ProviderResult`
- checks whether an image path exists
- returns Haiku `status_t`
- stores a human-readable last error message

The real backend is not implemented yet.

## Current data flow

```text
DailyImageProvider
  -> ProviderResult
      -> WallpaperInfo
      -> image path

ProviderResult
  -> DeskbarView tooltip

ProviderResult
  -> WallpaperSetter
      -> status_t / error message
      -> MainWindow status display
````

## Build system

BeMyDailyWall uses Haiku's Generic Makefile / makefile-engine.

This is the default build policy for the project because it is native enough for
a standalone Haiku application while keeping the project small and readable.

Jam is reserved for cases where code is integrated into an existing Jam-based
Haiku project or where matching Haiku source tree conventions is explicitly
required.

## Near-term boundaries

The project currently does not implement:

* network access
* real wallpaper download
* real wallpaper setting
* Deskbar installation
* archive/gallery browsing

Those will be added only after the internal seams are explicit and testable.
