# Phase 0034: Desktop wallpaper target resolution

Date: 2026-07-15
Baseline commit: `9e121a6 Add injected wallpaper setter backend`

## Scope

This phase adds one owner for the two real Haiku resources required by the
already injected `WallpaperSetter` backend:

```text
Desktop BNode
Tracker BMessenger
```

Resolution is real. Mutation is still absent.

## New boundary

New files:

```text
src/DesktopWallpaperTarget.h
src/DesktopWallpaperTarget.cpp
```

New type:

```cpp
DesktopWallpaperTarget
```

Public methods:

```cpp
status_t Resolve();
bool IsReady() const;

BNode& Node();
const BMessenger& Messenger() const;
```

## Ownership

The target owns its `BNode` and `BMessenger` by value.

A caller may use the returned references only while the
`DesktopWallpaperTarget` remains alive.

The type is deliberately product-specific. It is not a general node or
application-target resolver.

## Resolution flow

`Resolve()` performs:

```text
unset the current BNode
replace the current messenger with an invalid default messenger
resolve and validate B_DESKTOP_DIRECTORY
open the Desktop path as BNode
resolve the running Tracker messenger
publish both resources
```

If Desktop resolution or node opening fails, the exact status is returned.

If Tracker resolution fails after the Desktop node was opened, the node is
unset before returning the Tracker status.

## Readiness

`IsReady()` requires:

```text
Desktop node InitCheck() == B_OK
Tracker messenger IsValid() == true
Tracker messenger Team() >= 0
```

It does not inspect or modify the background attribute.

## Smoke

New target:

```sh
make smoke-desktop-wallpaper-target
```

The smoke runs on the actual Haiku desktop session and verifies:

```text
a new target is not ready
a new target has no valid messenger
Resolve() returns B_OK
the target becomes ready
the Desktop BNode is valid
the Tracker BMessenger is valid
the Tracker team is non-negative
```

The smoke contains no call to:

```text
WriteAttr
RemoveAttr
Sync
SendMessage
WallpaperSetter::Apply
```

It opens the real Desktop node and resolves the real Tracker only.

## Build impact

Added:

```text
src/DesktopWallpaperTarget.h
src/DesktopWallpaperTarget.cpp
tests/desktop_wallpaper_target_smoke.cpp
```

Changed:

```text
Makefile
docs/architecture.md
this Prüffeld document
```

The application build includes the target owner.

The new smoke is included in the aggregate smoke target.

## Catalog impact

No user-visible string is added.

No Catkey or catalog regeneration is required.

## Validation

```sh
make clean
make
make smoke-desktop-wallpaper-target
make smoke
```

No visual gate is required because the wallpaper does not change.

## Proves

- the real Desktop directory can be opened as a valid `BNode`
- the running Tracker can be resolved as a valid `BMessenger`
- both resources have one explicit lifetime owner
- failed partial resolution does not retain a ready Desktop node
- real resolution is separable from real mutation
- aggregate smoke remains non-mutating

## Does not prove

- the injected setter is called with the real targets
- the Desktop background attribute can be changed successfully
- the real Tracker observes a changed attribute
- a deliberate user action exists
- final error presentation is localized
- release readiness

Smoke tests prove life, not dignity.
