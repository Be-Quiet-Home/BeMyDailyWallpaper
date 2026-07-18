# Phase 0050: MainWindow startup-action source

Date: 2026-07-18
Baseline commit: `474f525 Extract MainWindow daily readiness source`

## Scope

This phase adds one private `MainWindow` method that returns the startup action
derived from the real readiness and persisted startup-apply permission.

It does not execute the returned action.

## New window method

```cpp
DailyWallpaperStartupAction CurrentStartupAction() const;
```

Implementation:

```cpp
return DailyWallpaperStartupPlan::Plan(
    CurrentDailyReadiness(),
    fSettings.StartupApplyEnabled());
```

## Authority chain

```text
loaded settings history
current local date
loaded provider candidate
    -> CurrentDailyReadiness()

CurrentDailyReadiness()
persisted startup-apply permission
    -> DailyWallpaperStartupPlan::Plan()

plan result
    -> DO_NOTHING or APPLY_ONCE
```

The method does not inspect the checkbox control. It reads the already-loaded
and persisted model value from `AppSettings`.

## No execution

The following remains absent:

```text
DailyWallpaperStartupPlan::Execute()
DesktopWallpaperTarget::Resolve() during startup
DailyWallpaperAction::Execute() during startup
automatic provider reload after startup
```

The method is intentionally not called by the constructor in this phase.

## Presentation

`UpdateDailyStatus()` continues to call only `CurrentDailyReadiness()`.

Visible status text, checkbox behavior, and manual wallpaper application remain
unchanged.

## Locale impact

No user-visible string changes.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-startup-plan
make smoke
```

No visual gate is required because the application behavior is unchanged.

## Proves

- MainWindow has one returning source for the real startup action
- the action uses the same readiness as the visible daily status
- the action uses the persisted model permission, not control state
- planning remains separate from execution
- the constructor remains nonmutating
- no target or setter is resolved during startup

## Does not prove

- the constructor consumes the returned action
- `DailyWallpaperStartupPlan::Execute()` is called
- a real automatic wallpaper operation
- automatic success or failure presentation
- retry, midnight refresh, or scheduling

Smoke tests prove life, not dignity.
