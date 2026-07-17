# Phase 0035: Explicit wallpaper apply action

Date: 2026-07-17
Baseline commit: `dd3838d Add Desktop wallpaper target resolution`

## Scope

This phase connects the previously verified real target and injected setter
boundaries to one explicit `MainWindow` button.

Application startup remains non-mutating.

## User action

New button:

```text
Apply wallpaper
```

The button posts one window message:

```cpp
'ApWp'
```

Only that message enters the real wallpaper operation.

## Availability

The button is enabled only when:

```text
ProviderResolver::Create() == B_OK
provider Fetch() == B_OK
ProviderResult::HasImagePath() == true
```

The default `DemoProvider` intentionally returns no image path, so default
settings display a disabled button and:

```text
Wallpaper: no image path from the provider.
```

A configured `LocalFolderProvider` with a recognized image can enable the
button.

## Window ownership

`MainWindow` now retains the fetched `ProviderResult` as a member.

It continues to delete the short-lived provider immediately after the
synchronous fetch.

Window-owned members:

```cpp
ProviderResult fProviderResult;
BButton* fApplyButton;
BStringView* fSetterStatusLabel;
```

The child views remain owned by the Haiku view hierarchy.

## Apply flow

On the button message:

```text
disable the button
resolve DesktopWallpaperTarget
construct WallpaperSetter(target.Node(), target.Messenger())
apply the retained ProviderResult
show success or failure
re-enable the button
```

Successful application displays:

```text
Wallpaper applied.
```

Target and setter failures include Haiku's status description.

If rollback was attempted and failed, both the primary failure and rollback
failure are visible in the status label.

## Safety

No wallpaper operation occurs in:

```text
MainWindow constructor
application startup
aggregate smoke launch
button-disabled state
```

The existing default `WallpaperSetter` constructor remains a safe stub and is
no longer invoked by `MainWindow`.

The real operation requires a delivered user-action message.

## Layout

The button is placed in a trailing horizontal action row:

```text
[glue] [Apply wallpaper]
```

The existing `BGroupLayout` and `BLayoutBuilder` ownership model is retained.
No fixed child coordinates are introduced.

## Locale impact

New `MainWindow` catalog keys are introduced for:

```text
button label
ready / unavailable / no-path states
success
target failure
operation failure
operation plus rollback failure
```

Run:

```sh
make catkeys
```

and commit the regenerated English CatKeys file.

## Automated validation

```sh
make clean
make
make catkeys
make smoke
```

The aggregate smoke launches and closes the application without posting the
apply message. It must not change the Desktop.

## Visual gate

With default settings:

```text
Apply wallpaper button is visible
button is disabled
status says no image path is available
Desktop wallpaper does not change
```

With an already configured local-folder provider and valid image result:

```text
button is enabled
clicking it is the only real mutation trigger
success changes the wallpaper and reports Wallpaper applied.
failure reports the system error
```

The configured local-folder gate may be completed in the following settings UI
phase when no usable settings file exists yet.

## Proves

- the real target and setter are connected only behind an explicit UI message
- application startup remains non-mutating
- provider results survive provider destruction
- missing image paths cannot trigger the backend
- button re-entry is blocked during the synchronous operation
- rollback failure remains visible
- the action follows native Haiku message and layout patterns

## Does not prove

- local-folder provider configuration is reachable from the UI
- daily scheduling
- settings update after success
- Deskbar installation
- release readiness

Smoke tests prove life, not dignity.
