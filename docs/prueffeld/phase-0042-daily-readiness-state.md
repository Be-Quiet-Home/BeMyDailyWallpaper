# Phase 0042: Daily readiness state

Date: 2026-07-17
Baseline commit: `3d7538f Wire next-image preview through resolver`

## Scope

This phase combines the existing daily date state with the already loaded
provider candidate.

The result is visible and independently testable, but remains non-mutating.

No timer, scheduler, startup apply, or automatic Desktop operation is added.

## New readiness contract

New states:

```cpp
DAILY_WALLPAPER_READINESS_UNAVAILABLE
DAILY_WALLPAPER_READINESS_APPLIED_TODAY
DAILY_WALLPAPER_READINESS_NO_CANDIDATE
DAILY_WALLPAPER_READINESS_READY
```

New deterministic operation:

```cpp
static DailyWallpaperReadiness EvaluateReadiness(
    DailyWallpaperState state,
    bool hasCandidate);
```

## Decision table

```text
daily state unavailable, candidate false
    -> unavailable

daily state unavailable, candidate true
    -> unavailable

already applied today, candidate false
    -> already applied today

already applied today, candidate true
    -> already applied today

pending, candidate false
    -> no candidate

pending, candidate true
    -> ready
```

An unknown daily state fails closed as unavailable.

## Authority

The date state has authority over candidate availability:

- a candidate cannot make an unavailable date ready
- a prepared next candidate cannot erase an already-applied-today state
- candidate availability matters only while the daily state is pending

This readiness is informational. It does not control the manual apply button.

## MainWindow ordering

Construction changes from:

```text
update daily status
load provider
```

to:

```text
load provider
derive readiness from the final ProviderResult
```

`ReloadProvider()` now calls `UpdateDailyStatus()` after it has reset and fetched
`ProviderResult`, updated the preview, and decided apply-button availability.

Folder selection and successful post-apply next-image reload therefore use the
same ordering automatically.

## Visible states

```text
Daily status: unavailable.
Daily status: today's wallpaper is already applied.
Daily status: no wallpaper candidate is available.
Daily status: wallpaper is ready to apply.
```

The previous pending-only sentence is removed:

```text
Daily status: no wallpaper has been applied today.
```

## Manual authority

`Apply wallpaper` remains governed by provider success and
`ProviderResult::HasImagePath()`.

Even when readiness reports already applied today, a valid candidate keeps the
manual button available.

## Smoke extension

The daily policy smoke proves all decision-table rows plus the unknown-state
fallback.

It still touches no settings file and no Desktop state.

## Locale impact

Run:

```sh
make catkeys
```

and commit the regenerated English CatKeys file.

## Validation

```sh
make catkeys
make clean
make
make smoke-daily-policy
make smoke
```

## Visual gate

With a valid candidate and no successful apply recorded for today:

```text
Daily status: wallpaper is ready to apply.
```

After successful application and history persistence:

```text
Daily status: today's wallpaper is already applied.
```

The next candidate may already be prepared and the manual apply button may
remain enabled, but the Desktop must not change again without another click.

## Proves

- daily state and candidate availability form one explicit readiness contract
- readiness is derived only after provider loading completes
- a candidate cannot override unavailable or already-applied state
- a pending day without a candidate is distinguishable from date failure
- a pending day with a candidate is explicitly ready
- the manual action remains independent
- startup remains non-mutating

## Does not prove

- automatic once-per-day application
- midnight refresh
- missed-day catch-up
- timer or scheduler ownership
- Deskbar activation

Smoke tests prove life, not dignity.
