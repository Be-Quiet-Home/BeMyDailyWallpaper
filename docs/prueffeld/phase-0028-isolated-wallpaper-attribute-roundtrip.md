# Phase 0028: Isolated wallpaper attribute roundtrip

Date: 2026-07-14
Baseline commit: `4626212 Add Haiku wallpaper contract probe`

## Scope

This phase proves the Tracker background attribute wire format on an isolated
temporary BFS node.

It does not write the Desktop directory attribute and does not notify Tracker.

## New node seam

`HaikuWallpaperContract` gains:

```cpp
static status_t WriteMessage(BNode& node, const BMessage& message);
static status_t ReadMessage(const BNode& node, BMessage& message);
```

The caller owns and selects the node.

No method combines `DesktopTarget()` with `WriteMessage()`.

## Write contract

`WriteMessage()` performs:

```text
BNode::InitCheck()
BMessage::FlattenedSize()
BMessage::Flatten()
BNode::WriteAttr(
    B_BACKGROUND_INFO,
    B_MESSAGE_TYPE,
    offset 0,
    flattened bytes)
BNode::Sync()
```

Status handling:

```text
invalid node          -> node InitCheck status
allocation failure    -> B_NO_MEMORY
flatten failure       -> flatten status
attribute write error -> WriteAttr status
short write           -> B_IO_ERROR
sync error            -> Sync status
```

The method does not remove unrelated attributes.

## Read contract

`ReadMessage()` first clears the supplied target message.

It then performs:

```text
BNode::InitCheck()
BNode::GetAttrInfo(B_BACKGROUND_INFO)
require B_MESSAGE_TYPE
require positive size
BNode::ReadAttr()
BMessage::Unflatten()
```

Status handling:

```text
missing attribute     -> B_ENTRY_NOT_FOUND
wrong attribute type  -> B_BAD_TYPE
non-positive size     -> B_BAD_DATA
allocation failure    -> B_NO_MEMORY
attribute read error  -> ReadAttr status
short read            -> B_IO_ERROR
unflatten failure     -> Unflatten status
```

Failed reads leave no stale fields in the output message.

## Isolated smoke fixture

The existing Haiku wallpaper contract smoke creates one temporary regular file
under:

```text
B_SYSTEM_TEMP_DIRECTORY
```

The fixture is removed on exit.

The smoke never opens the Desktop directory as a writable `BNode`.

## Negative probes

Before the successful roundtrip the smoke verifies:

```text
no attribute
    -> B_ENTRY_NOT_FOUND
    -> target message cleared

B_BACKGROUND_INFO with B_STRING_TYPE
    -> B_BAD_TYPE
    -> target message cleared
```

The wrong-type fixture is removed before the real message is written.

## Roundtrip probe

The smoke builds the existing public wallpaper message, writes it as:

```text
attribute name: B_BACKGROUND_INFO
attribute type: B_MESSAGE_TYPE
```

It then verifies:

- stored attribute type,
- stored byte size,
- successful read and unflatten,
- removal of stale output fields,
- exact field names and single-value counts,
- unchanged image path,
- scaled placement,
- zero origin,
- enabled icon label outline,
- all-workspaces targeting.

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

The existing smoke target and Makefile source boundaries remain unchanged.

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

- the prepared Tracker message can be flattened,
- the public attribute name accepts `B_MESSAGE_TYPE`,
- the flattened bytes survive a BFS attribute roundtrip,
- wrong attribute types are rejected,
- missing attributes remain visible,
- failed reads cannot leak stale message fields,
- the write seam can be tested without touching the Desktop.

## What this phase does not prove

This phase does not prove that:

- the Desktop directory attribute can be replaced safely,
- an existing Desktop background can be restored after failure,
- Tracker notification can use only public authority,
- Tracker accepts a newly written Desktop attribute,
- the wallpaper changes,
- the application is release-ready.

Smoke tests prove life, not dignity.
