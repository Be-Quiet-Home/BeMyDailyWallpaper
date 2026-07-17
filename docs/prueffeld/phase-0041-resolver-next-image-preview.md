# Phase 0041: Resolver next-image preview

Date: 2026-07-17
Baseline commit: `81a0a37 Add deterministic next-image selection`

## Scope

This phase connects the persisted `LastImagePath()` to the already proven
Local-folder next-image constructor.

After a confirmed wallpaper success and history save, `MainWindow` reloads the
provider so the next manual candidate is immediately visible.

No automatic wallpaper operation, timer, or scheduler is added.

## Resolver mapping

The Local-folder mapping changes from:

```cpp
LocalFolderProvider(settings.LocalFolderPath().String())
```

to:

```cpp
LocalFolderProvider(
    settings.LocalFolderPath().String(),
    settings.LastImagePath().String())
```

The provider still owns copies of both values.

## Resolver smoke

The resolver smoke now creates a temporary directory containing two valid PNG
fixtures:

```text
Alpha.JPG
middle.jpeg
```

Settings inject:

```text
provider = Local folder
local folder path = temporary directory
last image path = Alpha.JPG
```

The provider created by `ProviderResolver` must fetch:

```text
middle.jpeg
```

This proves forwarding through real provider behavior rather than through a
test-only accessor.

## Successful apply flow

After:

```text
WallpaperSetter::Apply() == B_OK
history date was produced
AppSettings::Save() == B_OK
```

`MainWindow` now performs:

```text
update daily status
reload provider
show the existing success message
restore the Choose folder action
return
```

`ReloadProvider()` refreshes:

```text
ProviderResult
Deskbar preview
provider status
wallpaper action status
Apply wallpaper enabled state
```

The success branch returns before the old unconditional apply-button
re-enablement. A failed next-image reload therefore leaves the apply action
disabled, as decided by `ReloadProvider()`.

## Manual behavior

On application startup, a persisted Local-folder configuration now previews the
image after `LastImagePath()`.

After a successful manual apply, the next image is prepared immediately.

The user must still click `Apply wallpaper` for every Desktop change, including
another change on the same day.

## Locale impact

No user-visible string is added or removed.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-provider-resolver
make smoke
```

## Functional gate

Use a configured folder containing at least two recognized images.

Expected:

```text
current candidate is applied
success and history are saved
preview/result advances to the next candidate
Apply wallpaper remains available when that candidate loaded successfully
the Desktop does not change again until another explicit click
```

## Proves

- persisted last-image state reaches LocalFolderProvider through the resolver
- resolver forwarding is verified using real recognized image fixtures
- a successful apply advances the prepared manual candidate
- provider reload remains the authority for button availability
- a failed reload cannot be overridden by unconditional re-enablement
- every Desktop mutation still requires a user message

## Does not prove

- automatic once-per-day application
- midnight refresh
- missed-day catch-up
- archive or gallery behavior
- Deskbar scheduling

Smoke tests prove life, not dignity.
