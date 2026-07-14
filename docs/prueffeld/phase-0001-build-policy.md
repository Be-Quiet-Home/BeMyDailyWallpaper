# Phase 0001: Build policy and runtime smoke

Date: 2026-07-14
Commit: `3f1d25c Add build help and smoke targets`

## Scope

This phase establishes the repository-root build contract for BeMyDailyWall.

It changes build and test infrastructure only. Application behavior, providers,
settings data, wallpaper handling, Deskbar integration, and user interface code
remain unchanged.

## Environment

Validated on the E73 Haiku development host with:

```text
Haiku hrev57937+129, x86_64
GCC 13.3.0
Haiku Generic Makefile / makefile-engine
```

## Root build contract

The repository root provides:

```sh
make
make clean
make help
make smoke
```

`make help` reports:

```text
BeMyDailyWall build targets:
  make        Build the application
  make clean  Remove build artifacts
  make smoke  Launch and verify the application stays alive
  make help   Show this help
```

## Compiler policy

Haiku's local makefile-engine maps:

```text
OPTIMIZE = FULL  -> -O3
OPTIMIZE = SOME  -> -O1
OPTIMIZE = NONE  -> -O0
```

The project requires an explicit generic `-O2` baseline.

The Makefile therefore uses:

```make
OPTIMIZE := NONE
WARNINGS := ALL
COMPILER_FLAGS += -O2 -Wextra -Werror
```

The compile command contains both `-O0` and the later project-owned `-O2`.
The later option is effective.

Project-owned source files build with:

```text
-Wall
-Wextra
-Werror
```

## Build smoke

Command:

```sh
make clean
make
```

Observed result:

```text
Build completed successfully.
No project-owned compiler warnings were emitted.
```

## Runtime smoke

Command:

```sh
make smoke
```

Expected output:

```text
BeMyDailyWall smoke: ok
```

The smoke target:

1. verifies that the built application is executable,
2. launches BeMyDailyWall,
3. waits one second,
4. verifies that the process is still alive,
5. terminates the process,
6. reports success.

## What this phase proves

This phase proves that:

- the project builds from the repository root,
- `make clean` removes the Generic Makefile build directory,
- the generic `-O2` compiler policy is explicit,
- strict project warning flags are accepted by the current source tree,
- `make help` documents the root targets,
- the application executable starts,
- the application process remains alive for the smoke interval,
- the smoke target cleans up the process it started.

## What this phase does not prove

This phase does not prove that:

- the main window content is correct,
- the window uses final Haiku layout behavior,
- the application quits through its normal `B_QUIT_REQUESTED` path,
- settings values load or save correctly,
- corrupt settings are handled correctly,
- the provider result is correct,
- the tooltip is visible or correctly formatted,
- wallpaper application works,
- Deskbar installation or Replicant behavior works,
- archive or gallery behavior exists,
- network access works,
- the application is release-ready.

Smoke tests prove life, not dignity.

## Local patch staging

The repository ignores:

```text
/patches/
```

This directory is local staging for assistant-provided patch files and is not
part of the versioned project.
