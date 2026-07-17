# Phase 0038: Daily application status

Date: 2026-07-17
Baseline commit: `0244dda Persist successful wallpaper state`

## Scope

This phase derives one visible daily state from the existing
`LastUpdateDate()` setting.

It adds no timer, scheduler, automatic wallpaper change, or apply restriction.

## Decision

The window obtains the current local date through the existing stable
`YYYY-MM-DD` helper.

```text
LastUpdateDate == current local date
    -> today's wallpaper is already applied

otherwise
    -> no wallpaper has been applied today
```

A clock-conversion failure produces an unavailable state.

## Visible states

```text
Daily status: today's wallpaper is already applied.
Daily status: no wallpaper has been applied today.
Daily status: unavailable.
```

## Manual authority

The status is informational.

`Apply wallpaper` remains enabled whenever the provider has a valid image path,
even when the stored update date equals today. The user remains the authority
for manual reapplication.

## Refresh points

The status is refreshed:

```text
during MainWindow construction
after successful wallpaper history persistence
```

A history-save failure does not advance the visible daily state.

## Locale impact

Run `make catkeys` and commit the regenerated English CatKeys file.

## Validation

```sh
make catkeys
make clean
make
make smoke
```

## Visual gate

After a successful apply and history save:

```text
Daily status: today's wallpaper is already applied.
```

After reopening the application on the same local calendar day, the same state
must be restored from settings.

## Proves

- the stored ISO date supports a deterministic same-day decision
- the decision survives application restart
- successful history persistence refreshes the visible state
- failed history persistence cannot create a false today state
- the manual apply action remains available

## Does not prove

- automatic daily execution
- midnight refresh while the window remains open
- next-image selection
- missed-day catch-up
- scheduling or Deskbar integration

Smoke tests prove life, not dignity.
