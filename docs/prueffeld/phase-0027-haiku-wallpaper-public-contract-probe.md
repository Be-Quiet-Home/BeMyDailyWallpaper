# Phase 0027: Haiku wallpaper public contract probe

Date: 2026-07-14
Baseline commit: `8b67e80 Resolve MainWindow provider from settings`

## Scope

This phase records and tests Haiku's public Tracker background contract without
changing the desktop.

It introduces `HaikuWallpaperContract`, a non-mutating preparation component.
`WallpaperSetter::Apply()` remains unchanged and still returns
`B_NOT_SUPPORTED` for a result with an image path.

## Local authority

The contract uses the public Haiku header:

```cpp
#include <be_apps/Tracker/Background.h>
```

That header defines:

```text
B_BACKGROUND_INFO
B_BACKGROUND_IMAGE
B_BACKGROUND_MODE
B_BACKGROUND_ORIGIN
B_BACKGROUND_ERASE_TEXT
B_BACKGROUND_WORKSPACES
B_BACKGROUND_MODE_SCALED
B_RESTORE_BACKGROUND_IMAGE
```

No private Tracker header is included by production or smoke code.

## Desktop target

`DesktopTarget()` resolves:

```text
B_DESKTOP_DIRECTORY
```

It verifies through `BEntry` that the result:

```text
exists
is a directory
```

The method returns the path only. It does not open the node for writing and
does not inspect or change its background attribute.

## Prepared message

`BuildMessage()` accepts one non-empty image path and creates exactly one value
for each public Tracker field:

```text
B_BACKGROUND_IMAGE       string  supplied path
B_BACKGROUND_MODE        int32   B_BACKGROUND_MODE_SCALED
B_BACKGROUND_ORIGIN      point   (0, 0)
B_BACKGROUND_ERASE_TEXT  bool    true
B_BACKGROUND_WORKSPACES  int32   B_ALL_WORKSPACES
```

A null or empty image path returns:

```text
B_BAD_VALUE
```

The output message is cleared before every build.

## Public attribute contract

`AttributeName()` returns:

```text
B_BACKGROUND_INFO
```

Haiku's Backgrounds preflet flattens the same field structure and writes it as
`B_MESSAGE_TYPE` into that attribute on the selected node.

This phase stops before performing that write.

## Restore message boundary

`RestoreMessage()` returns the public message code:

```text
B_RESTORE_BACKGROUND_IMAGE
```

The Backgrounds preflet sends that code to Tracker after writing the Desktop
attribute.

Tracker's application signature is not added to this contract. In the current
Haiku source tree it is declared in `tracker_private.h`, so the notification
target requires a separate authority decision before implementation.

## New smoke

New target:

```sh
make smoke-haiku-wallpaper-contract
```

It verifies:

- public attribute-name identity,
- public restore-message identity,
- Desktop path resolution,
- Desktop entry existence and directory type,
- null and empty path rejection,
- exact field names, types, and counts,
- unchanged image path,
- scaled placement,
- zero origin,
- enabled icon label outline,
- all-workspaces targeting.

The smoke does not write an attribute and sends no message.

The aggregate `make smoke` includes this target.

## Build impact

New production files:

```text
src/HaikuWallpaperContract.h
src/HaikuWallpaperContract.cpp
```

New smoke:

```text
tests/haiku_wallpaper_contract_smoke.cpp
```

Changed build file:

```text
Makefile
```

The contract is compiled into the application but is not yet called by
`WallpaperSetter`.

## Catalog impact

This phase adds no user-facing text and no Locale Kit key.

No Catkey or catalog regeneration is required.

## Validation

```sh
make clean
make
make smoke-haiku-wallpaper-contract
make smoke
```

No visual gate is required because neither application behavior nor the desktop
background changes.

## What this phase proves

This phase proves that:

- the public Tracker background header is available to the project,
- the Desktop target can be resolved without mutation,
- BeMyDailyWall can build the same public field schema Tracker consumes,
- the placement and workspace policy is explicit,
- no private Tracker header is required for message preparation,
- the restore message code is public.

## What this phase does not prove

This phase does not prove that:

- the message has been flattened,
- an attribute can be written and read back,
- the Desktop attribute can be replaced safely,
- Tracker notification can use only public authority,
- Tracker accepts the prepared message,
- the wallpaper changes,
- the application is release-ready.

Smoke tests prove life, not dignity.
