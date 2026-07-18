# Phase 0052: Startup-execution sentinel

Date: 2026-07-18
Baseline commit: `2c86ea3 Show planned startup action`

## Scope

This phase connects the real `MainWindow` constructor to
`DailyWallpaperStartupPlan::Execute()` without providing a real wallpaper
executor.

The startup path remains non-mutating.

## New window entry

```cpp
status_t ExecuteStartupAction();
```

Constructor order:

```text
load settings
build controls
reload provider
calculate and display readiness and startup plan
execute startup plan through the sentinel
resize window
```

The plan is therefore consumed only after the final provider result exists.

## Sentinel executor

```cpp
static status_t StartupExecutionNotWired(void*)
{
    return B_NOT_SUPPORTED;
}
```

It owns no context and performs no operation.

## Execution behavior

```text
CurrentStartupAction() == DO_NOTHING
    -> DailyWallpaperStartupPlan::Execute()
    -> B_OK
    -> sentinel is not called
    -> existing disabled or not-needed diagnosis remains visible

CurrentStartupAction() == APPLY_ONCE
    -> DailyWallpaperStartupPlan::Execute()
    -> sentinel called exactly once
    -> B_NOT_SUPPORTED
    -> Startup execution: not wired.
```

The callback-count contract is already covered by the focused startup-plan
smoke.

## Safety boundary

The constructor still does not call:

```text
DesktopWallpaperTarget::Resolve()
WallpaperSetter constructor
DailyWallpaperAction::Execute()
AppSettings::Save()
ReloadProvider() a second time
```

No image, Desktop attribute, Tracker message, or history value can change.

## Setting changes after startup

Changing the checkbox still updates only the saved permission and planned-action
diagnosis. It does not call `ExecuteStartupAction()` again.

The sentinel is therefore a construction-time proof, not a scheduler.

## Locale impact

New CatKey:

```text
Startup execution: not wired.
```

Run `make catkeys` and commit the regenerated English CatKeys file.

## Validation

```sh
make catkeys
make clean
make
make smoke-startup-plan
make smoke
```

## Visual gate

With startup apply disabled or no daily action needed, the existing diagnosis
remains visible and startup leaves the Desktop unchanged.

On a pending day with a valid candidate and saved startup opt-in:

```text
Startup execution: not wired.
```

The Desktop must still remain unchanged.

## Proves

- the real constructor consumes the real startup action
- consumption occurs only after provider loading
- DO_NOTHING remains callback-free
- APPLY_ONCE reaches one explicit executor seam
- the current executor is visibly and deliberately non-mutating
- no real target, setter, action, history, retry, or scheduler is introduced

## Does not prove

- real automatic wallpaper application
- real startup target resolution
- startup history persistence
- automatic success or operation-failure presentation
- retry, midnight refresh, or scheduling

Smoke tests prove life, not dignity.
