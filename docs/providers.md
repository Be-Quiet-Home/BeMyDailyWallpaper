# Providers

Providers supply daily image metadata and, later, image files.

The provider layer must stay small and explicit. Providers should not own UI,
Deskbar behavior, wallpaper setting, archive handling, or application lifecycle.

## Current provider components

### DailyImageProvider

`DailyImageProvider` is the abstract provider interface.

A provider must implement:

- `Name()`
- `Fetch(ProviderResult& result)`

`Fetch()` returns `true` when provider data was loaded successfully.

### ProviderResult

`ProviderResult` carries provider output.

Current fields:

- `WallpaperInfo`
- image path

The image path may be empty. In that case, `WallpaperSetter` must fail cleanly
with a visible error message.

### DemoProvider

`DemoProvider` is the current dry test provider.

It returns stable demo metadata:

- title
- description
- source name
- attribution text

It does not return an image path.

## Required metadata

A provider should return at least:

- title
- date, if available
- source name
- image URL or local image path, once image fetching exists
- copyright / attribution, if available
- short description, if available

Missing optional fields must not break the application.

## Planned provider ideas

### Microsoft daily image provider

Fetches the daily image metadata and image from Microsoft's public daily image
source.

The provider name must not imply affiliation with Microsoft or Bing.

Allowed style:

- `Microsoft daily image provider`
- `Daily Earth Image provider`

Avoid:

- product names that contain `Bing`
- use of Bing logos
- wording that implies official affiliation

### NASA APOD provider

Fetches NASA Astronomy Picture of the Day metadata.

The provider must handle non-image APOD entries cleanly, because APOD may also
publish videos or other media types.

Allowed style:

- `NASA APOD provider`

Avoid:

- NASA logo usage
- wording that implies official affiliation

### Local folder provider

Uses a local folder as a wallpaper source.

This is useful for archived images, user-selected folders, and offline testing.

## Branding and attribution rules

Provider names must avoid implying affiliation with third-party brands.

Third-party logos must not be used for the application icon, Deskbar item, or
default UI branding.

Attribution should be stored and displayed when available.

The BeMyDailyWall icon and Deskbar item belong to BeMyDailyWall, not to any
specific provider.
