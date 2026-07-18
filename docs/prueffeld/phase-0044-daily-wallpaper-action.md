# Phase 0044: Daily wallpaper action

Date: 2026-07-17
Baseline commit: `9636109 Add daily wallpaper startup plan`

## Scope

This phase extracts the proven wallpaper-apply and history-persistence sequence
from `MainWindow` into one reusable action brick.

The existing button remains the only real caller. The startup plan is not wired
to the action in this phase.

## New brick

New files:

```text
src/DailyWallpaperAction.h
src/DailyWallpaperAction.cpp
```

The action receives:

```text
AppSettings&
ProviderResult
apply callback
current-date callback
settings-save callback
one opaque callback context
```

No heap-owned command object, scheduler, or generic task framework is added.

## Result contract

```cpp
struct DailyWallpaperActionResult {
    status_t applyStatus;
    status_t historyStatus;
    status_t rollbackStatus;
};
```

The three statuses remain independent.

A successful Desktop operation is not rewritten as a failure merely because
date production or history persistence fails afterward.

## Execution order

```text
validate image path and callback table
apply candidate exactly once
stop on apply failure
obtain current date
stop on date failure or empty date
capture previous history
set candidate image path and date
invoke save callback exactly once
restore previous in-memory history on save failure
return all statuses
```

There is no retry.

## Callback contract

Apply:

```cpp
status_t Apply(
    void* context,
    const ProviderResult& result,
    status_t& rollbackStatus);
```

Date:

```cpp
status_t CurrentDate(
    void* context,
    BString& date);
```

Save:

```cpp
status_t Save(
    void* context,
    const AppSettings& settings);
```

The callback table is validated before any wallpaper operation. A missing
callback therefore cannot permit an unrecordable Desktop change.

## MainWindow responsibility

`MainWindow` still owns:

```text
button enable/disable state
DesktopWallpaperTarget resolution
WallpaperSetter lifetime
localized success and failure text
provider reload after complete success
```

Its adapters forward to:

```text
WallpaperSetter::Apply()
DailyWallpaperPolicy::CurrentLocalDate()
AppSettings::Save()
```

The button's visible behavior and existing CatKeys remain unchanged.

## Smoke

New target:

```sh
make smoke-daily-action
```

The smoke uses only in-memory callbacks and verifies:

```text
missing candidate -> no callbacks
missing callback -> no apply
apply failure -> no date or save
rollback status preserved
date failure -> apply remains successful, no save
empty date -> rejected, no save
save failure -> new values observed by saver, old values restored afterward
complete success -> new path and date retained
every used callback is called exactly once
```

No Desktop target is resolved and `AppSettings::Save()` is not called by the
smoke.

## Locale impact

No user-facing string changes.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-daily-action
make smoke
```

## Real gate

Use the existing configured Local-folder provider and click
`Apply wallpaper`.

Expected behavior remains:

```text
wallpaper changes once
history saves
success text appears
next manual candidate loads
no second Desktop change occurs automatically
```

## Proves

- one reusable action owns apply and history ordering
- callback configuration is validated before mutation
- the apply callback runs at most once
- post-apply history failure remains separately visible
- save failure restores previous in-memory history
- MainWindow no longer duplicates persistence mechanics
- the manual real path uses the extracted action
- startup remains non-mutating

## Does not prove

- DailyWallpaperStartupPlan invokes this action
- startup auto-apply
- retry or missed-day catch-up
- midnight refresh
- Deskbar scheduling

Smoke tests prove life, not dignity.
