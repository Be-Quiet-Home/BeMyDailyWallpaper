# Phase 0032: Injected Tracker notifier contract

Date: 2026-07-15
Baseline commit: `42e93a8 Extract internal BettributeStore boundary`

## Scope

This phase adds one narrow Tracker restore-notification boundary.

It does not write the Desktop background attribute and does not notify the real
Tracker during automated smoke tests.

## Local authority result

The local Haiku development system reported:

```text
public header:
    /system/develop/headers/os/be_apps/Tracker/Background.h

public background attribute:
    B_BACKGROUND_INFO = "be:bgndimginfo"

public restore command:
    B_RESTORE_BACKGROUND_IMAGE = 'Tbgr'

private Tracker header:
    /system/develop/headers/private/tracker/tracker_private.h

private signature constant:
    kTrackerSignature = "application/x-vnd.Be-TRAK"
```

A non-mutating local probe constructed a `BMessenger` for the literal
signature and observed:

```text
constructor_status=0
is_valid=true
team=152
BeMyDailyWall Tracker authority probe: ok
```

The probe did not send a message and did not touch the Desktop.

## Authority decision

`B_RESTORE_BACKGROUND_IMAGE` remains sourced from the public
`<be_apps/Tracker/Background.h>` header.

The project does not include `tracker_private.h`.

The verified Tracker signature literal is isolated in:

```text
src/TrackerNotifier.cpp
```

This is a compatibility fact with one owner, not a new public Haiku API claim.

## New boundary

New files:

```text
src/TrackerNotifier.h
src/TrackerNotifier.cpp
```

Public methods:

```cpp
static const char* Signature();
static int32 RestoreMessage();

static status_t ResolveTarget(BMessenger& target);
static status_t NotifyRestore(const BMessenger& target);
```

## Target resolution

`ResolveTarget()`:

```text
resets the output messenger
constructs a messenger for the verified Tracker signature
returns the constructor status on failure
rejects an invalid target or negative team with B_BAD_VALUE
returns B_OK with the resolved messenger
```

Target resolution performs no mutation and sends no message.

## Injected notification

`NotifyRestore()` accepts a caller-supplied `BMessenger`.

```text
invalid target
    -> B_BAD_VALUE

valid target
    -> SendMessage(B_RESTORE_BACKGROUND_IMAGE)
    -> return the exact SendMessage status
```

The notifier does not resolve a target implicitly. This keeps the mutation
boundary visible to the caller.

## Responsibility move

`HaikuWallpaperContract` no longer exposes:

```cpp
RestoreMessage()
```

The public restore command belongs to `TrackerNotifier`.

The wallpaper contract continues to own message construction, verification,
attribute identity, isolated read/write, and replace-or-rollback.

## Smoke

New target:

```sh
make smoke-tracker-notifier
```

The smoke:

```text
checks the verified signature literal
checks the public restore command
rejects a default invalid BMessenger
runs one local BLooper
injects a messenger for that local target
sends the restore notification
waits for the local target to receive it
verifies the exact message code
```

It never calls `ResolveTarget()` and never addresses the real Tracker.

## Build impact

Added:

```text
src/TrackerNotifier.h
src/TrackerNotifier.cpp
tests/tracker_notifier_smoke.cpp
```

Changed:

```text
Makefile
src/HaikuWallpaperContract.h
src/HaikuWallpaperContract.cpp
tests/haiku_wallpaper_contract_smoke.cpp
docs/architecture.md
this Prüffeld document
```

The new source is part of the application build and the new smoke is part of
the aggregate smoke target.

## Catalog impact

No user-visible string is added.

No Catkey or catalog regeneration is required.

## Validation

```sh
make clean
make
make smoke-tracker-notifier
make smoke
```

No visual gate is required.

## Proves

- the local Tracker target identity is isolated in one implementation file
- no private Tracker header enters project source
- the public restore command has one notifier owner
- invalid injected targets fail explicitly
- a valid injected Haiku message target receives the exact restore command
- automated tests do not notify the real Tracker
- the wallpaper contract no longer owns notification responsibility

## Does not prove

- the real Tracker accepts the restore command from this application
- a Desktop attribute change becomes visible
- target resolution works after Tracker restart
- notification and attribute replacement form one successful user operation
- rollback after a real Tracker notification failure
- release readiness

Smoke tests prove life, not dignity.
