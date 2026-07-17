# Phase 0039: Daily wallpaper policy

Date: 2026-07-17
Baseline commit: `092b7bd Add daily wallpaper status`

## Scope

This phase extracts local-date production and same-day evaluation from
`MainWindow` into one deterministic policy brick.

It adds no timer, automatic apply behavior, midnight refresh, or scheduler.

## New boundary

New files:

```text
src/DailyWallpaperPolicy.h
src/DailyWallpaperPolicy.cpp
```

New state:

```cpp
DAILY_WALLPAPER_UNAVAILABLE
DAILY_WALLPAPER_PENDING
DAILY_WALLPAPER_APPLIED_TODAY
```

New operations:

```cpp
static status_t CurrentLocalDate(BString& date);

static DailyWallpaperState Evaluate(
    const char* lastUpdateDate,
    const char* currentDate);
```

## Date production

`CurrentLocalDate()` preserves the established contract:

```text
local calendar date
YYYY-MM-DD
```

It uses the Haiku system clock through standard POSIX interfaces already
available on the platform:

```text
time
localtime_r
strftime
```

## Deterministic evaluation

`Evaluate()` does not read the clock.

```text
missing or empty current date
    -> unavailable

last update date exactly equals current date
    -> applied today

all other history values
    -> pending
```

Older, future, empty, and missing history therefore remain pending rather than
being interpreted as completed.

## MainWindow responsibility

`MainWindow` now:

```text
asks the policy for today's date
passes LastUpdateDate and today's date into Evaluate
translates the returned state into the existing localized sentence
```

The existing strings and visual layout do not change.

The same policy date producer is used when persisting successful wallpaper
history.

## Smoke

New target:

```sh
make smoke-daily-policy
```

It verifies:

```text
null current date -> unavailable
empty current date -> unavailable
null history -> pending
empty history -> pending
older history -> pending
future history -> pending
matching dates -> applied today
live current date uses exactly YYYY-MM-DD
```

The smoke touches no settings file and no Desktop state.

## Locale impact

No new or removed user-facing string is introduced.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-daily-policy
make smoke
```

No visual gate is required because the visible sentences and layout remain
unchanged.

## Proves

- the same-day decision is deterministic and independently testable
- clock access is isolated from comparison logic
- future dates do not falsely count as applied today
- missing history remains a normal pending state
- UI and persistence share one date-format authority
- aggregate smoke remains non-mutating

## Does not prove

- automatic daily application
- a midnight notification or refresh
- next-image selection
- missed-day catch-up
- scheduling or Deskbar activation

Smoke tests prove life, not dignity.
