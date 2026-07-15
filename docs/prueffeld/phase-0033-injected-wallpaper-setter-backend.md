# Phase 0033: Injected WallpaperSetter backend

Date: 2026-07-15
Baseline commit: `e129118 Clean notifier boundary whitespace`

## Scope

This phase composes the existing wallpaper message, node replacement, rollback,
and Tracker notification boundaries inside `WallpaperSetter`.

Automated tests still do not open or mutate the real Desktop.

## Safe default path

`MainWindow` currently constructs:

```cpp
WallpaperSetter setter;
setter.Apply(result);
```

The default constructor therefore remains deliberately unbound.

For a valid image path it continues to return:

```text
B_NOT_SUPPORTED
Wallpaper backend is not implemented yet.
```

This prevents application startup and `make smoke` from changing the user's
wallpaper before a deliberate UI action exists.

## Injected backend

New constructor:

```cpp
WallpaperSetter(BNode& node, const BMessenger& target);
```

Both dependencies are caller-owned and must outlive the setter.

The injected `Apply()` path performs:

```text
validate ProviderResult image path
build wallpaper BMessage
capture original raw node attribute
write new message
read stored message
verify the five wallpaper fields
notify the injected BMessenger
return B_OK
```

Notification is the commit action passed to:

```cpp
HaikuWallpaperContract::ReplaceMessage(...)
```

## Rollback

If notification fails after successful write and verification:

```text
return the notification status
restore the original raw attribute state
expose restoration through LastRollbackStatus()
```

New accessor:

```cpp
status_t LastRollbackStatus() const;
```

Values follow the established contract:

```text
B_NO_INIT  no rollback attempted
B_OK       rollback succeeded
other      rollback failed with that status
```

## Error presentation boundary

Existing public preconditions remain translated:

```text
No wallpaper image path available.
Wallpaper backend is not implemented yet.
```

The injected backend is not yet called by the UI. Post-precondition backend
failures therefore remain `status_t` plus rollback status in this phase rather
than adding speculative user-facing strings.

The later deliberate Desktop action will own final translated presentation.

## Success smoke

The setter smoke creates:

```text
temporary BFS file
local BLooper restore target
BMessenger for that local target
injected WallpaperSetter
```

It verifies:

```text
Apply returns B_OK
LastRollbackStatus remains B_NO_INIT
local target receives B_RESTORE_BACKGROUND_IMAGE
temporary node stores the requested image path
```

## Notification-failure smoke

The smoke then prepares an original wallpaper message and injects an invalid
default `BMessenger`.

It verifies:

```text
Apply returns B_BAD_VALUE from TrackerNotifier
LastRollbackStatus returns B_OK
the original wallpaper message is restored
no speculative UI error string is created
```

## Build impact

Changed production files:

```text
src/WallpaperSetter.h
src/WallpaperSetter.cpp
```

Changed smoke:

```text
tests/wallpaper_setter_smoke.cpp
```

Changed build wiring:

```text
Makefile
```

The setter smoke now links the existing production boundaries:

```text
BettributeStore
HaikuWallpaperContract
TrackerNotifier
```

Changed documentation:

```text
docs/architecture.md
this Prüffeld document
```

## Catalog impact

No new user-facing string is added.

The existing English setter precondition keys remain unchanged.

No Catkey or catalog regeneration is required.

## Validation

```sh
make clean
make
make smoke-setter
make smoke
```

No visual gate is required.

## Proves

- `WallpaperSetter` can compose the complete backend on injected dependencies
- successful replacement is followed by the exact restore notification
- notification failure triggers attribute rollback
- rollback status remains independently observable
- default construction stays non-mutating
- automated smokes do not address the real Tracker or Desktop

## Does not prove

- the real Desktop node can be opened and changed by `WallpaperSetter`
- the real Tracker observes a changed Desktop attribute
- a deliberate UI action exists
- final backend error presentation is localized
- settings are updated after a successful wallpaper change
- release readiness

Smoke tests prove life, not dignity.
