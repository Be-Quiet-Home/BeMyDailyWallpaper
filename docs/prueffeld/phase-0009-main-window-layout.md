# Phase 0009: Main window layout

Date: 2026-07-14
Baseline commit: `da32f06 Use atomic settings replacement`

## Scope

This phase removes fixed `BRect` coordinates from the ordinary child views in
`MainWindow`.

The window remains the same diagnostic test bench. Status calculation, provider
flow, setter flow, and tooltip data do not change.

## Native layout

`MainWindow` now uses:

```text
BGroupView
BGroupLayout
BLayoutBuilder
B_USE_WINDOW_SPACING
B_AUTO_UPDATE_SIZE_LIMITS
```

The root panel keeps the standard Haiku panel background color.

The content order remains:

```text
application liveness
settings status
Deskbar preview + preview description
provider status
wallpaper setter status
```

`ResizeToPreferred()` derives the initial content size from the layout.

The constructor still supplies the initial screen position through the
`BWindow` frame. This phase removes fixed geometry from controls, not from
window placement.

## Deskbar preview

`DeskbarView` now provides two constructors:

```text
DeskbarView()
DeskbarView(BRect frame)
```

The no-frame constructor is used by the layout-managed preview.

Both constructors share `_Init()` and declare an explicit 32 x 32 layout size.
The existing frame constructor remains available for future Deskbar or
Replicant integration.

## Validation

Automated checks:

```sh
make clean
make
make smoke
```

Manual visual check:

```text
launch BeMyDailyWall
verify all five diagnostic areas are visible
verify labels do not overlap
verify the Deskbar preview remains square
hover the preview and verify the tooltip still appears
close the window and verify the application exits
```

## What this phase proves

This phase proves that:

- ordinary `MainWindow` child views no longer use fixed `BRect` coordinates,
- the diagnostic content is owned by native Haiku layouts,
- the preview row combines the icon and its label without manual positioning,
- `DeskbarView` can participate directly in a layout,
- window size limits follow the content layout,
- existing application liveness remains covered by `make smoke`.

## What this phase does not prove

This phase does not prove that:

- every font size and localization length has been visually tested,
- visible strings are localized,
- the current diagnostic window is final product UI,
- the Deskbar view can already be installed as a Replicant,
- the final application window size or placement is settled,
- accessibility review is complete,
- the application is release-ready.

Smoke tests prove life, not dignity.
