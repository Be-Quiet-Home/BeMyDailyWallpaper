# Phase 0002: Provider contract smoke

Date: 2026-07-14
Baseline commit: `eb86db8 Stop work after provider failure`

## Scope

This phase adds a small standalone smoke binary for the provider result
contract.

The smoke code lives under `tests/`. It is not compiled into BeMyDailyWall and
does not add a test mode, hidden setting, or provider switch to the application.

## Commands

Run only the provider contract smoke:

```sh
make smoke-provider
```

Run all current smoke checks:

```sh
make smoke
```

`make smoke` runs the provider contract smoke before launching the application
runtime smoke.

## Expected output

```text
BeMyDailyWall provider contract smoke: ok
BeMyDailyWall smoke: ok
```

## Provider cases

The smoke checks two providers.

### DemoProvider

Expected contract:

```text
Fetch() returns B_OK.
The result title is "Somewhere else".
The result source matches DemoProvider::Name().
The result has no image path.
```

The empty image path is intentional. It does not turn the successful provider
result into a provider failure.

### FailingProvider

`FailingProvider` exists only inside the smoke source.

Expected contract:

```text
Fetch() returns B_ERROR.
```

It supplies a controlled failure without adding failure behavior to the
application or production providers.

## Build behavior

The smoke binary is written to:

```text
$(OBJ_DIR)/provider-contract-smoke
```

It uses the same project include paths, compiler flags, optimization policy, and
warning policy as the main build.

`make clean` removes the smoke binary with the normal Generic Makefile output
directory.

## What this phase proves

This phase proves that:

- `DailyImageProvider::Fetch()` can report success through `B_OK`,
- `DailyImageProvider::Fetch()` can report failure through a non-`B_OK`
  `status_t`,
- `DemoProvider` returns the documented stable metadata,
- `DemoProvider` intentionally returns no image path,
- the provider contract can be exercised without launching the GUI,
- the provider smoke builds with the strict project warning policy,
- the root `make smoke` target includes the provider contract smoke.

## What this phase does not prove

This phase does not prove that:

- `MainWindow` visually enters its provider-failure branch,
- `MainWindow` skips `WallpaperSetter` at runtime after a provider failure,
- a failed provider leaves `ProviderResult` unchanged,
- callers may inspect or consume `ProviderResult` after a failed `Fetch()`,
- network, file, timeout, cancellation, or parsing failures work,
- every future provider returns the most specific possible status code,
- wallpaper application works,
- the application is release-ready.

The provider contract states that callers must not consume `ProviderResult`
after `Fetch()` fails. This smoke does not weaken that rule by inspecting the
failed result.

Smoke tests prove life, not dignity.
