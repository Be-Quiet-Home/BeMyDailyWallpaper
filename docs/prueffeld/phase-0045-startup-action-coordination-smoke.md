# Phase 0045: Startup-action coordination smoke

Date: 2026-07-18
Baseline commit: `db3949d Extract daily wallpaper action`

## Scope

This phase proves that `DailyWallpaperStartupPlan` and
`DailyWallpaperAction` compose through their existing callback contracts.

It adds no production coordinator class and changes no application source.

## Why smoke-only

The two bricks already provide the required terminals:

```text
DailyWallpaperStartupPlan
    readiness -> DO_NOTHING / APPLY_ONCE
    one injected startup executor

DailyWallpaperAction
    settings + candidate + callbacks
    -> apply / history / rollback statuses
```

A new production coordinator would add ownership before a real startup caller
exists. The smoke therefore performs the composition locally and keeps the
product architecture unchanged.

## Composition

The smoke executor:

```text
receives one coordination context
increments one action counter
calls DailyWallpaperAction::Execute()
stores the complete action result
returns apply failure first
otherwise returns history status
```

The full `DailyWallpaperActionResult` remains available, so the combined
`status_t` does not erase the distinction between wallpaper and history
failure.

## Proven cases

```text
unavailable readiness
already applied today
no candidate readiness
unknown readiness
    -> DO_NOTHING
    -> zero daily-action calls
    -> zero apply/date/save callbacks
    -> history unchanged

READY with empty ProviderResult
    -> startup executor called once
    -> action rejects the missing image
    -> zero apply/date/save callbacks
    -> history unchanged

READY with apply failure
    -> daily action called once
    -> apply callback called once
    -> date/save not called
    -> rollback status preserved
    -> history unchanged

READY with save failure
    -> daily action called once
    -> apply/date/save each called once
    -> apply remains successful
    -> history failure returned
    -> prior in-memory history restored

READY with complete success
    -> daily action called once
    -> apply/date/save each called once
    -> new image path and date retained
```

No case retries.

## New target

```sh
make smoke-startup-action
```

## Product behavior

Unchanged:

```text
MainWindow startup does not invoke DailyWallpaperStartupPlan.
No real Desktop target is resolved.
No wallpaper is applied during startup.
The manual button remains the only real caller of DailyWallpaperAction.
```

## Locale impact

No user-visible strings change.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-startup-action
make smoke
```

No visual gate is required because application behavior is unchanged.

## Proves

- the plan callback seam can carry the reusable daily action
- all non-ready states remain nonmutating
- READY invokes the action at most once
- action and callback counts remain deterministic
- apply, history, and rollback truth remain separately observable
- no production coordinator is needed yet
- application startup remains nonmutating

## Does not prove

- MainWindow startup invokes the plan
- real Desktop target resolution during startup
- automatic once-per-day application
- UI behavior after startup success or failure
- midnight refresh, retry, or scheduling

Smoke tests prove life, not dignity.
