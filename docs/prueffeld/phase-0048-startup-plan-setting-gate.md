# Phase 0048: Startup-plan settings gate

Date: 2026-07-18
Baseline commit: `17aa319 Add startup apply checkbox`

## Scope

This phase makes the persisted startup-apply permission an explicit input to
`DailyWallpaperStartupPlan`.

It updates only the plan contract and isolated smokes. `MainWindow` still does
not invoke the plan during startup.

## Plan contract

Previous contract:

```cpp
Plan(DailyWallpaperReadiness readiness)
```

New contract:

```cpp
Plan(
    DailyWallpaperReadiness readiness,
    bool startupApplyEnabled)
```

The plan does not read `AppSettings` directly. Its caller supplies the already
loaded boolean.

## Decision table

```text
startup apply disabled + unavailable
    -> DO_NOTHING

startup apply disabled + applied today
    -> DO_NOTHING

startup apply disabled + no candidate
    -> DO_NOTHING

startup apply disabled + ready
    -> DO_NOTHING

startup apply disabled + unknown readiness
    -> DO_NOTHING

startup apply enabled + unavailable
    -> DO_NOTHING

startup apply enabled + applied today
    -> DO_NOTHING

startup apply enabled + no candidate
    -> DO_NOTHING

startup apply enabled + ready
    -> APPLY_ONCE

startup apply enabled + unknown readiness
    -> DO_NOTHING
```

Permission is therefore necessary but not sufficient. Readiness must also be
exactly READY.

## Execution contract

`DailyWallpaperStartupPlan::Execute()` is unchanged:

```text
DO_NOTHING
    -> B_OK
    -> no executor required
    -> executor not called

APPLY_ONCE
    -> one executor call
    -> returned status preserved
    -> no retry
```

## Plan smoke

`make smoke-startup-plan` now verifies both permission values across all
readiness states.

The execution tests remain unchanged.

## Coordination smoke

The cross-brick smoke now obtains permission from:

```cpp
coordination.settings->StartupApplyEnabled()
```

It proves:

```text
READY + stored false + valid candidate
    -> zero DailyWallpaperAction calls
    -> zero apply/date/save callbacks
    -> history unchanged

READY + stored true
    -> existing one-shot action cases remain active

non-ready + stored true
    -> no action
```

The smoke continues to use temporary in-memory state and injected callbacks.

## Product behavior

Unchanged:

```text
the checkbox persists opt-in and opt-out
MainWindow startup does not invoke the plan
no real startup target is resolved
no wallpaper is applied during startup
manual Apply wallpaper remains the only real Desktop mutation
```

## Locale impact

No user-visible string changes.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-startup-plan
make smoke-startup-action
make smoke
```

No visual gate is required because application behavior is unchanged.

## Proves

- stored permission is a hard planning gate
- disabled READY cannot execute the daily action
- enabled non-ready states remain nonmutating
- only enabled READY plans one action
- the plan remains independent of AppSettings persistence
- coordination reads the already-loaded model value
- startup remains nonmutating

## Does not prove

- MainWindow startup invokes the plan
- real startup target resolution
- automatic once-per-day application
- startup success or failure presentation
- retry, midnight refresh, or scheduling

Smoke tests prove life, not dignity.
