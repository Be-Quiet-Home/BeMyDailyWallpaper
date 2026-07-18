# Phase 0053a: Deskbar tooltip refresh

Date: 2026-07-18
Baseline commit: `e69af7f Probe startup wallpaper target`

## Observation

After a successful manual wallpaper apply, `MainWindow` reloads the local-folder
provider and advances to the next candidate. The preview receives the new
`WallpaperInfo`, but the hover tooltip can continue showing the previous image
metadata.

## Cause

`DeskbarView::SetInfo()` already replaces both its owned `WallpaperInfo` and its
stored tooltip string.

Haiku's public `BView::SetToolTip(const char*)` updates an existing text tooltip
in place. It does not first close a tooltip that is already being managed for
the view.

The result is a presentation-cache mismatch rather than stale provider data.

## Fix

`DeskbarView::SetInfo()` now:

```text
build new tooltip text
compare it with the stored tooltip text

unchanged
    -> keep the current tooltip

changed
    -> HideToolTip()

store new WallpaperInfo
store new tooltip text
SetToolTip(new text)
invalidate the icon view
```

Only the public Interface Kit methods `HideToolTip()` and `SetToolTip()` are
used. No private Haiku header is introduced.

## Why compare first

Provider reload may return identical metadata. Hiding the hover in that case
would add visible churn without changing information.

## Product behavior

After a successful manual apply:

```text
history is saved
provider advances to the next candidate
Deskbar preview receives the next candidate metadata
any old visible tooltip is closed
the next hover displays the next candidate metadata
```

No wallpaper, provider-selection, startup, or history contract changes.

## Locale impact

No user-visible strings are added or changed.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-wallpaper-info
make smoke
```

## Real gate

Use a local folder with at least two images whose filenames differ visibly.

```text
hover preview
    -> first candidate filename is visible

click Apply wallpaper
    -> wallpaper changes
    -> provider advances

hover preview again
    -> second candidate filename is visible
    -> first filename is no longer shown
```

Also verify that leaving the pointer over the preview while metadata changes
does not leave the old tooltip balloon visible.

## Proves

- provider metadata was not the stale authority
- changed tooltip metadata closes the currently managed hover
- the replacement uses only public Haiku Interface Kit API
- unchanged metadata does not cause needless hover churn
- manual apply still advances to the next candidate
- startup behavior remains unchanged

## Does not prove

- final Deskbar Replicant installation
- a final icon design
- automatic startup application
- timer, midnight refresh, or scheduling

Smoke tests prove life, not dignity.
