# Phase 0054: Reusable MainWindow wallpaper action

Date: 2026-07-18
Baseline commit: `e69c100 Refresh Deskbar tooltip metadata`

## Scope

This phase extracts the real target/setter/action operation from the manual
button handler into one private reusable `MainWindow` method.

The manual button remains the only caller.

## New operation seam

```cpp
DailyWallpaperActionResult ExecuteCurrentWallpaperAction(
    status_t& targetStatus);
```

The returned `DailyWallpaperActionResult` keeps:

```text
applyStatus
historyStatus
rollbackStatus
```

`targetStatus` separately reports whether the Desktop/Tracker target could be
resolved.

## Operational ownership

`ExecuteCurrentWallpaperAction()` owns:

```text
rejecting a missing image before target resolution
DesktopWallpaperTarget construction and Resolve()
WallpaperSetter construction
real apply callback
real current-date callback
real settings-save callback
one DailyWallpaperAction::Execute() call
```

It does not own:

```text
button enable or disable state
localized result text
provider reload after success
startup planning
retry or scheduling
```

## Manual button ownership

`ApplyWallpaper()` continues to own:

```text
early visible no-image message
disabling controls during the synchronous operation
target failure presentation
apply/history/rollback presentation
provider reload after complete success
restoring controls
```

Visible behavior is unchanged.

## Missing-image contract

The manual button still performs its visible early guard.

The reusable operation also fails closed when called without an image:

```text
targetStatus = B_OK
applyStatus = B_BAD_VALUE
historyStatus = B_NO_INIT
rollbackStatus = B_NO_INIT
```

This prevents a future non-button caller from resolving the Desktop without a
candidate.

## Startup boundary

`ExecuteStartupAction()` continues to use only the non-mutating target probe.

It does not call `ExecuteCurrentWallpaperAction()` in this phase.

## Locale impact

No user-visible strings are added or changed.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-daily-action
make smoke-wallpaper-setter
make smoke
```

## Real gate

Use a local folder with at least two valid images.

```text
click Apply wallpaper
    -> current candidate is applied
    -> history is saved
    -> provider advances once
    -> success text is unchanged
    -> Deskbar tooltip shows the next candidate
```

Target, apply, history, and rollback failures must retain their existing
messages.

## Proves

- the real operation has one reusable MainWindow seam
- UI presentation is separate from target/setter/action execution
- the manual button still uses exactly one DailyWallpaperAction
- target setup status remains distinct from action statuses
- provider reload remains a caller responsibility
- startup remains target-probe-only
- no second implementation or scheduler is introduced

## Does not prove

- startup calls the reusable action seam
- automatic wallpaper application
- startup success or failure presentation
- retry, midnight refresh, or scheduling

Smoke tests prove life, not dignity.
