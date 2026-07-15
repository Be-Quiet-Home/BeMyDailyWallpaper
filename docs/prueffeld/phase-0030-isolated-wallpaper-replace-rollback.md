# Phase 0030: Isolated wallpaper replace or rollback

Date: 2026-07-15
Baseline commit: `ef8b0f1 Add wallpaper attribute backup restore`

## Scope

This phase composes the existing message, attribute, backup, and restore seams
into one verified replace-or-rollback operation.

The operation still accepts only a caller-supplied `BNode`.

The Desktop node is not opened for writing and Tracker is not notified.

## Stable message verification

New method:

```cpp
static status_t VerifyMessage(
    const BMessage& expected,
    const BMessage& actual);
```

The project does not use Haiku's experimental `BMessage::HasSameData()` API.

Verification instead requires exactly the five public wallpaper fields:

```text
B_BACKGROUND_IMAGE
B_BACKGROUND_MODE
B_BACKGROUND_ORIGIN
B_BACKGROUND_ERASE_TEXT
B_BACKGROUND_WORKSPACES
```

For both messages it verifies:

```text
same what code
exactly five names
expected field type
exactly one value per field
equal field values
```

Unexpected additional fields return `B_BAD_DATA`.

## Optional commit action

New callback type:

```cpp
typedef status_t (*HaikuWallpaperCommitAction)(
    const BMessage& storedMessage,
    const void* cookie);
```

The action is optional.

It runs only after:

```text
attribute write
attribute sync
attribute read
message unflatten
stable message verification
```

A future phase may use this boundary for the Tracker acceptance step. This
phase uses it only as a deterministic rejection seam in the isolated smoke.

## Replace contract

New method:

```cpp
static status_t ReplaceMessage(
    BNode& node,
    const BMessage& message,
    status_t& rollbackStatus,
    HaikuWallpaperCommitAction commitAction = NULL,
    const void* cookie = NULL);
```

Execution:

```text
rollbackStatus = B_NO_INIT
capture current raw attribute state
write new message
read stored message
verify stored message
run optional commit action
return B_OK
```

Any failure after successful capture performs:

```text
RestoreAttribute(node, backup)
```

## Two status channels

The return value reports the primary operation:

```text
capture
write
read
verify
commit action
```

`rollbackStatus` reports restoration separately:

```text
B_NO_INIT  no rollback was attempted
B_OK       rollback succeeded
other      rollback failed with that status
```

The primary failure is never replaced by the rollback result.

## Successful replacement probe

The smoke starts with the original wallpaper message and replaces it with a
different image path.

It verifies:

```text
ReplaceMessage returns B_OK
rollbackStatus remains B_NO_INIT
the replacement message is stored
```

## Deterministic rejection probe

The smoke provides a commit action that first verifies the stored replacement
image path and then returns:

```text
B_CANCELED
```

This guarantees that rejection occurs only after successful write, read, and
built-in verification.

## Rollback states

The deterministic rejection probe runs against three original states.

### Message state

```text
original wallpaper message
    -> replacement written and verified
    -> commit action returns B_CANCELED
    -> original message restored
```

### Missing state

```text
no B_BACKGROUND_INFO attribute
    -> replacement written and verified
    -> commit action returns B_CANCELED
    -> attribute absent again
```

### Raw state

```text
B_STRING_TYPE raw fixture
    -> replacement written and verified
    -> commit action returns B_CANCELED
    -> original type, size, and bytes restored
```

Every rollback must return `B_OK`.

## Build impact

Changed production files:

```text
src/HaikuWallpaperContract.h
src/HaikuWallpaperContract.cpp
```

Changed smoke:

```text
tests/haiku_wallpaper_contract_smoke.cpp
```

Changed documentation:

```text
docs/architecture.md
this Prüffeld document
```

The Makefile and existing smoke target remain unchanged.

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

No visual gate is required because the Desktop is not changed.

## What this phase proves

This phase proves that:

- stored wallpaper messages can be verified without experimental APIs,
- successful replacement commits without rollback,
- downstream rejection triggers rollback,
- message, missing, and raw original states are restored,
- primary and rollback failures remain separately observable,
- the entire operation remains isolated from the Desktop.

## What this phase does not prove

This phase does not prove that:

- rollback itself is atomic,
- the Desktop attribute can be changed safely,
- Tracker notification can use only public authority,
- Tracker accepts a new Desktop message,
- the wallpaper changes,
- the application is release-ready.

Smoke tests prove life, not dignity.
