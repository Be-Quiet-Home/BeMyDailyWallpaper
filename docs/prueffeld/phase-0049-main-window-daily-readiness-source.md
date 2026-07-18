# Phase 0049: MainWindow daily-readiness source

Date: 2026-07-18
Baseline commit: `290db03 Gate startup plan with saved setting`

## Scope

This phase extracts the real `MainWindow` daily-readiness calculation into one
private method that returns `DailyWallpaperReadiness`.

It changes no planning, execution, settings, provider, or Desktop behavior.

## New window method

```cpp
DailyWallpaperReadiness CurrentDailyReadiness() const;
```

The method uses the window-owned real state:

```text
AppSettings::LastUpdateDate()
current local YYYY-MM-DD
ProviderResult::HasImagePath()
```

It delegates all decisions to the existing `DailyWallpaperPolicy`.

## Calculation

```text
obtain current local date

date unavailable
    -> DAILY_WALLPAPER_UNAVAILABLE

date available
    -> compare LastUpdateDate with today

combine daily state with HasImagePath()
    -> unavailable
    -> applied today
    -> no candidate
    -> ready
```

The method does not inspect the startup-apply checkbox or call
`DailyWallpaperStartupPlan`.

## Presentation boundary

Before this phase, `UpdateDailyStatus()` both calculated readiness and selected
localized text.

After this phase:

```text
CurrentDailyReadiness()
    -> calculate and return enum

UpdateDailyStatus()
    -> call CurrentDailyReadiness()
    -> translate enum into existing visible text
```

All CatKeys and visible strings remain unchanged.

## Provider ordering

`ReloadProvider()` continues to:

```text
clear old ProviderResult
resolve and fetch provider
finalize candidate state
update provider controls
call UpdateDailyStatus()
```

The extracted method therefore observes the same final provider result as the
existing status path.

## Future seam

A later startup caller may use:

```cpp
DailyWallpaperStartupPlan::Plan(
    CurrentDailyReadiness(),
    fSettings.StartupApplyEnabled())
```

That call is intentionally not added here.

## Locale impact

No user-visible string changes.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-daily-policy
make smoke
```

No visual gate is required because the status text and startup behavior are
unchanged.

## Proves

- MainWindow has one returning source for real daily readiness
- date, history, and candidate logic are no longer embedded in presentation
- status presentation consumes the extracted value
- provider reload ordering remains unchanged
- a future startup caller can reuse the exact same real readiness
- application startup remains nonmutating

## Does not prove

- MainWindow invokes the startup plan
- real startup Desktop target resolution
- automatic wallpaper application
- startup result presentation
- retry, midnight refresh, or scheduling

Smoke tests prove life, not dignity.
