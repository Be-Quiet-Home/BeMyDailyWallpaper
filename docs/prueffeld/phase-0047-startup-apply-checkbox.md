# Phase 0047: Startup-apply checkbox

Date: 2026-07-18
Baseline commit: `9fecf85 Add startup apply setting`

## Scope

This phase exposes the persisted startup-apply preference through one native
`BCheckBox` in `MainWindow`.

It does not execute the startup plan and does not change the Desktop during
application startup.

## Native control

New control:

```text
Apply today's wallpaper automatically at startup
```

The control is a Haiku Interface Kit `BCheckBox` and participates in the
existing `BGroupLayout`.

Its initial value is:

```cpp
fSettings.StartupApplyEnabled()
    ? B_CONTROL_ON
    : B_CONTROL_OFF
```

The checkbox remains available even when no provider candidate exists. It
represents a future preference, while readiness remains responsible for deciding
whether a later startup operation would be safe.

## Message contract

New window message:

```cpp
kStartupApplyChanged = 'StAp'
```

A user invocation calls:

```cpp
MainWindow::StartupApplyChanged()
```

Programmatic initialization through `SetValue()` does not execute the startup
plan.

## Immediate persistence

On a real value change:

```text
capture previous setting
disable checkbox during synchronous save
set new in-memory value
AppSettings::Save()
```

Success:

```text
retain new value
show "Startup apply setting saved."
re-enable checkbox
```

Failure:

```text
restore previous AppSettings value
restore previous visible checkbox value
show the existing settings-save error with Haiku status text
re-enable checkbox
```

No provider reload is needed because the setting does not affect candidate
selection.

## Startup authority

The following remains absent from `MainWindow` startup:

```text
DailyWallpaperStartupPlan::Plan()
DailyWallpaperStartupPlan::Execute()
DesktopWallpaperTarget::Resolve() for startup
DailyWallpaperAction::Execute() for startup
```

The manual `Apply wallpaper` button remains the only real Desktop mutation path.

## Locale impact

New CatKeys:

```text
Apply today's wallpaper automatically at startup
Startup apply setting saved.
```

The existing key is reused for failures:

```text
Settings save failed: %error%
```

Run `make catkeys` and commit the regenerated English CatKeys file.

## Validation

```sh
make catkeys
make clean
make
make smoke
```

## Visual gate

Starting with the migrated default:

```text
checkbox is off
startup does not change the Desktop
```

Then:

```text
turn checkbox on
success status appears
close and reopen the application
checkbox remains on
startup still does not change the Desktop
```

Finally:

```text
turn checkbox off
close and reopen the application
checkbox remains off
startup still does not change the Desktop
```

## Proves

- the user can explicitly opt in and opt out
- the safe default is visible
- the loaded value initializes a native Haiku control
- every user change is persisted immediately
- save failure restores model and view state
- preference editing is independent of provider readiness
- startup remains nonmutating

## Does not prove

- MainWindow startup reads the flag for execution
- a real automatic wallpaper operation
- UI behavior after automatic success or failure
- retry, midnight refresh, or scheduling
- a separate Preferences window

Smoke tests prove life, not dignity.
