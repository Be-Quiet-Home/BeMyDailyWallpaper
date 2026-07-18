# Phase 0056: Real startup wallpaper action

Date: 2026-07-18
Baseline commit: `549fb5c Clarify applied and next wallpaper state`

## Scope

This phase replaces the startup target probe with the real reusable
apply-and-history operation.

It introduces the first production automatic wallpaper application.

## Authority gate

Automatic execution requires both:

```text
AppSettings::StartupApplyEnabled() == true
CurrentDailyReadiness() == READY
```

`DailyWallpaperStartupPlan::Plan()` maps only that combination to `APPLY_ONCE`.

Every other combination remains `DO_NOTHING`.

## Single execution path

```text
application constructor
    -> ReloadProvider()
    -> CurrentStartupAction()
    -> DailyWallpaperStartupPlan::Execute()
    -> ExecuteStartupWallpaper()
    -> ExecuteCurrentWallpaperAction()
```

The startup executor does not duplicate:

```text
DesktopWallpaperTarget resolution
WallpaperSetter construction
wallpaper apply
rollback handling
current local date lookup
LastImagePath update
LastUpdateDate update
AppSettings save
```

These remain owned by the same operation used by the manual button.

## Callback context

The startup callback stores:

```text
MainWindow pointer
target status
DailyWallpaperActionResult
```

It returns:

```text
target failure first
otherwise apply failure
otherwise history status
```

The detailed result remains available to `ExecuteStartupAction()` for visible
presentation.

## Success

Complete startup success means:

```text
target resolved
wallpaper applied
history saved
```

Then `ReloadProvider()` runs exactly once.

Visible state becomes:

```text
Last applied wallpaper: <consumed candidate>
Next wallpaper: <following candidate>
Daily status: today's wallpaper is already applied.
Startup wallpaper applied and history saved.
```

## Failure

Target failure:

```text
Startup target failed: <Haiku status>
```

Apply or rollback failure uses the existing wallpaper-failure presentation.

History failure uses:

```text
Wallpaper applied, but history save failed: <Haiku status>
```

No failure path reloads the provider or retries.

## Same-day idempotence

After complete success, `LastUpdateDate` equals the current local date.

A second application start on the same day evaluates:

```text
APPLIED_TODAY
    -> startup plan DO_NOTHING
    -> executor callback not invoked
```

The wallpaper therefore changes at most once per recorded local day through
startup execution.

## Checkbox behavior

Changing the checkbox in a running application only saves the preference and
updates the plan diagnosis.

It does not immediately execute the startup action. The saved permission is
consumed by the next application start.

## Locale impact

New CatKey:

```text
Startup wallpaper applied and history saved.
```

The previous success-only target-probe key disappears:

```text
Startup target: ready.
```

The target-failure key remains valid.

Run `make catkeys` and commit the regenerated English CatKeys file.

## Validation

```sh
make catkeys
make clean
make
make smoke-startup-plan
make smoke-startup-action
make smoke-daily-action
make smoke-wallpaper-setter
make smoke
```

## Real gate: disabled

With startup apply disabled, launch the application:

```text
Startup action: disabled.
```

The Desktop and history must remain unchanged.

## Real gate: enabled READY

Prepare at least two valid local-folder images and a pending day.

In one application run:

```text
enable startup apply
close the application
```

The Desktop must not change merely from enabling the checkbox.

On the next launch:

```text
the displayed Next wallpaper is applied once
history is saved
provider advances once
Last applied wallpaper names the applied file
Next wallpaper names the following file
Startup wallpaper applied and history saved.
```

## Real gate: same-day relaunch

Close and launch the application again on the same local day.

Expected:

```text
Daily status: today's wallpaper is already applied.
Startup action: not needed.
```

The Desktop must not change again.

## Proves

- saved opt-in and READY jointly authorize automatic startup application
- the constructor executes the real action exactly once
- manual and startup application share one production operation
- apply, rollback, date, and history contracts are not duplicated
- complete success refreshes Last applied / Next state exactly once
- same-day relaunch is nonmutating
- checkbox changes are not hidden immediate execution triggers
- no retry, timer, or scheduler is introduced

## Does not prove

- complete historical avoidance of previously used images
- midnight execution while the app remains open
- periodic provider refresh
- network providers
- retry or scheduling

Smoke tests prove life, not dignity.
