# Phase 0051: Startup-action diagnostic

Date: 2026-07-18
Baseline commit: `b05a385 Add MainWindow startup action source`

## Scope

This phase exposes the current startup plan as a visible, non-mutating
diagnostic in `MainWindow`.

It does not execute the plan.

## New diagnostic

New `BStringView`:

```text
startupActionStatusLabel
```

New updater:

```cpp
void UpdateStartupActionStatus();
```

Visible states:

```text
Startup action: disabled.
Startup action: not needed.
Startup action: apply once.
```

## Meaning

```text
setting false
    -> disabled

setting true + DO_NOTHING
    -> not needed

setting true + APPLY_ONCE
    -> apply once
```

The text describes a plan, not a completed operation.

## Refresh ownership

The diagnosis refreshes after provider reload and after a startup-setting save
or rollback. It therefore cannot lag behind candidate or permission state.

## No execution

Absent in this phase:

```text
DailyWallpaperStartupPlan::Execute()
startup DesktopWallpaperTarget::Resolve()
startup DailyWallpaperAction::Execute()
```

The manual button remains the only real wallpaper mutation path.

## Locale impact

New CatKeys:

```text
Startup action: disabled.
Startup action: not needed.
Startup action: apply once.
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

With startup apply disabled:

```text
Startup action: disabled.
```

Enable the checkbox on a pending day with a valid candidate:

```text
Startup action: apply once.
```

When today's wallpaper is already recorded as applied:

```text
Startup action: not needed.
```

In every state, application startup must leave the Desktop unchanged.

## Proves

- the real MainWindow startup plan is visible
- disabled state is distinct from enabled DO_NOTHING
- provider and setting changes refresh the diagnosis
- diagnosis uses model and plan state
- planning remains separate from execution
- startup remains nonmutating

## Does not prove

- constructor execution of APPLY_ONCE
- real automatic wallpaper application
- automatic result presentation
- retry, midnight refresh, or scheduling

Smoke tests prove life, not dignity.
