# Phase 0053: Startup target probe

Date: 2026-07-18
Baseline commit: `9dbaa85 Add startup execution sentinel`

## Scope

This phase replaces the startup sentinel with one real, non-mutating
`DesktopWallpaperTarget` probe.

It does not construct `WallpaperSetter` and does not apply a wallpaper.

## Executor callback

```cpp
status_t ResolveStartupTarget(void* context);
```

The context is one stack-owned `DesktopWallpaperTarget`.

Execution:

```text
reject null context
DesktopWallpaperTarget::Resolve()
stop on failure
verify DesktopWallpaperTarget::IsReady()
return B_OK or B_NO_INIT
```

The target object is destroyed when `ExecuteStartupAction()` returns.

## Real resources

A successful probe resolves:

```text
the Haiku Desktop directory as BNode
the running Tracker as BMessenger
```

This is intentionally the same target contract used by the manual apply path.

The probe does not:

```text
write or remove a Desktop attribute
send B_RESTORE_BACKGROUND_IMAGE
construct WallpaperSetter
call DailyWallpaperAction
save AppSettings
reload the provider
```

## Startup behavior

```text
DO_NOTHING
    -> target callback not invoked
    -> existing disabled or not-needed diagnosis remains

APPLY_ONCE + target ready
    -> target callback invoked once
    -> Startup target: ready.

APPLY_ONCE + target failure
    -> target callback invoked once
    -> Startup target failed: <Haiku status>
```

No retry occurs.

## Locale impact

The previous sentinel key disappears:

```text
Startup execution: not wired.
```

New CatKeys:

```text
Startup target: ready.
Startup target failed: %error%
```

Run `make catkeys` and commit the regenerated English CatKeys file.

## Validation

```sh
make catkeys
make clean
make
make smoke-desktop-wallpaper-target
make smoke-startup-plan
make smoke
```

## Real gate

With startup apply disabled or no daily action needed, no target probe is
executed and the existing plan diagnosis remains visible.

On a pending day with a valid candidate and saved startup opt-in:

```text
Startup target: ready.
```

The Desktop wallpaper and saved history must remain unchanged.

## Proves

- the constructor reaches the real Desktop/Tracker target seam
- target resolution occurs only for APPLY_ONCE
- the same target contract as manual apply is available at startup
- target readiness and failures are visible
- the probe performs no wallpaper, Tracker, or settings mutation
- no retry, timer, or scheduler is introduced

## Does not prove

- WallpaperSetter construction during startup
- automatic wallpaper application
- startup history persistence
- automatic operation-result presentation
- retry, midnight refresh, or scheduling

Smoke tests prove life, not dignity.
