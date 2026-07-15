# Phase 0029: Isolated wallpaper attribute backup and restore

Date: 2026-07-15
Baseline commit: `222b034 Test wallpaper attribute roundtrip`

## Scope

This phase adds an exact backup and restore contract for
`B_BACKGROUND_INFO` on a caller-supplied `BNode`.

The smoke continues to use only one temporary file under
`B_SYSTEM_TEMP_DIRECTORY`.

The Desktop node is not opened for writing and Tracker is not notified.

## Backup object

New value owner:

```cpp
HaikuWallpaperAttributeBackup
```

It owns:

```text
whether the attribute existed
the original type_code
the original byte size
the original raw bytes
```

The object is non-copyable and releases its byte buffer in its destructor.

Public inspection:

```cpp
bool HasAttribute() const;
type_code Type() const;
ssize_t Size() const;
```

Raw data remains private to the restore contract.

## Capture contract

New method:

```cpp
static status_t CaptureAttribute(
    const BNode& node,
    HaikuWallpaperAttributeBackup& backup);
```

The backup is reset before capture.

A missing attribute is a successful state capture:

```text
status:       B_OK
HasAttribute: false
Type:         0
Size:         0
```

For an existing attribute, capture preserves its actual type and raw bytes.
The attribute is not required to contain a valid `BMessage`.

Status handling:

```text
invalid node          -> node InitCheck status
metadata error        -> GetAttrInfo status
non-positive size     -> B_BAD_DATA
allocation failure    -> B_NO_MEMORY
attribute read error  -> ReadAttr status
short read            -> B_IO_ERROR
```

## Restore contract

New method:

```cpp
static status_t RestoreAttribute(
    BNode& node,
    const HaikuWallpaperAttributeBackup& backup);
```

Restore first removes the current `B_BACKGROUND_INFO` attribute.

Then:

```text
backup says absent
    -> leave the attribute absent
    -> Sync()

backup contains data
    -> write original type, size, and bytes
    -> Sync()
```

A missing current attribute during removal is accepted.

This is a complete state restoration contract on the isolated node. It is not
yet an atomic filesystem transaction: a write failure after removal can still
leave the target without the original attribute.

## Missing-state probe

The smoke captures a node with no background attribute.

It then:

```text
writes a wallpaper message
restores the backup
verifies B_ENTRY_NOT_FOUND
```

This proves that backup includes absence, not only data.

## Message-state probe

The smoke writes the original wallpaper message and captures it.

It then:

```text
writes a different image path
verifies the replacement message
restores the backup
verifies the original image path and field contract
```

The backup type and size must match the original `B_MESSAGE_TYPE` attribute.

## Raw-state probe

The smoke writes a deliberately non-message attribute:

```text
name: B_BACKGROUND_INFO
type: B_STRING_TYPE
data: fixed raw byte fixture
```

It captures that state, replaces it with a wallpaper `BMessage`, and restores
the backup.

The final attribute must have exactly the original:

```text
type
size
bytes
```

This proves that backup and restore do not reinterpret existing state.

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

- attribute absence can be captured and restored,
- a valid wallpaper message can be replaced and restored,
- an unexpected raw attribute type can be preserved exactly,
- backup owns its bytes independently of the node,
- repeated capture safely replaces earlier backup data,
- the entire contract remains isolated from the Desktop.

## What this phase does not prove

This phase does not prove that:

- restore is atomic after the current attribute is removed,
- the Desktop attribute can be changed safely,
- Tracker notification can use only public authority,
- Tracker accepts a new Desktop message,
- the wallpaper changes,
- the application is release-ready.

Smoke tests prove life, not dignity.
