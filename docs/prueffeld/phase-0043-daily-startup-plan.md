# Phase 0043: Daily startup plan

Date: 2026-07-17
Baseline commit: `94998f0 Add daily wallpaper readiness state`

## Scope

This phase converts the already proven readiness state into a narrow startup
action plan and proves one-shot execution through an injected callback.

It does not wire the plan to `MainWindow` or any real wallpaper target.

## New brick

New files:

```text
src/DailyWallpaperStartupPlan.h
src/DailyWallpaperStartupPlan.cpp
```

New action:

```cpp
DAILY_WALLPAPER_STARTUP_DO_NOTHING
DAILY_WALLPAPER_STARTUP_APPLY_ONCE
```

New executor seam:

```cpp
typedef status_t (*DailyWallpaperStartupExecutor)(void* context);
```

No `std::function`, heap-owned command object, scheduler, or generic task
framework is introduced.

## Planning rule

```text
readiness ready
    -> APPLY_ONCE

unavailable
already applied today
no candidate
unknown readiness
    -> DO_NOTHING
```

Unknown input therefore fails closed.

## Execution rule

```text
DO_NOTHING
    -> B_OK
    -> executor is not required
    -> executor is never called

APPLY_ONCE with no executor
    -> B_BAD_VALUE

APPLY_ONCE with executor
    -> call exactly once
    -> return executor status unchanged

unknown action
    -> B_BAD_VALUE
    -> executor is not called
```

The plan owns no retry behavior.

## Product wiring

The new source is compiled into the application, but `MainWindow` does not
include or call it in this phase.

Application startup therefore remains non-mutating.

## Smoke

New target:

```sh
make smoke-startup-plan
```

The smoke proves:

```text
all non-ready readiness states -> DO_NOTHING
ready -> APPLY_ONCE
unknown readiness -> DO_NOTHING
DO_NOTHING invokes zero callbacks
APPLY_ONCE invokes one callback
success status is preserved
failure status is preserved
failure is not retried
missing executor is rejected
unknown action is rejected without execution
```

The callback only increments an in-memory counter.

## Locale impact

No user-facing string changes.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-startup-plan
make smoke
```

No visual gate is required because application behavior is unchanged.

## Proves

- readiness has one explicit startup action mapping
- all unsafe or incomplete states fail closed
- an apply action requires an injected executor
- one plan execution performs at most one callback
- callback failure is not retried or rewritten
- the brick has no knowledge of Desktop, Tracker, settings, or providers
- aggregate startup remains non-mutating

## Does not prove

- `MainWindow` invokes the startup plan
- the real wallpaper setter can be used as the executor
- successful startup apply persists history
- automatic once-per-day behavior
- midnight refresh or scheduling
- Deskbar activation

Smoke tests prove life, not dignity.
