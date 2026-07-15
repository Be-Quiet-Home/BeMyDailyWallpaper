# Phase 0031: Internal BettributeStore boundary

Date: 2026-07-15
Baseline commit: `f38460a Add wallpaper replace rollback contract`

## Scope

This phase separates generic Haiku node-attribute storage mechanics from the
wallpaper and Tracker contract.

It is a responsibility refactor. Existing runtime behavior and smoke
expectations remain unchanged.

The external `quietcode/bettributestore` repository remains a seed only. It is
not a dependency, submodule, package, or build input.

## Internal brick

New files:

```text
src/BettributeStore.h
src/BettributeStore.cpp
```

New types:

```cpp
BettributeSnapshot
BettributeStore
```

`BettributeSnapshot` owns whether the attribute existed plus its original
`type_code`, size, and raw bytes. It is non-copyable and releases its buffer in
its destructor.

Current store methods:

```cpp
Write(BNode&, name, type, data, size)
Capture(const BNode&, name, snapshot)
Restore(BNode&, name, snapshot)
```

The store knows only:

```text
BNode
attribute name
type_code
raw bytes
attribute absence
```

It does not know wallpaper fields, Tracker messages, Desktop paths, MIME
meaning, settings, UI, sidecars, databases, queries, retries, or multiple
nodes.

## Wallpaper contract after extraction

`HaikuWallpaperContract` continues to own:

```text
DesktopTarget
wallpaper BMessage construction and verification
B_BACKGROUND_INFO identity
B_RESTORE_BACKGROUND_IMAGE identity
message read/write adaptation
verified replace-or-rollback
optional post-verification commit action
```

The generic capture and restore methods leave the wallpaper public header.
Message read and write adapt flattened `BMessage` bytes to the raw store.

## Smoke preservation

The existing wallpaper contract smoke continues to cover:

```text
message write/read
missing state
typed raw backup/restore
message backup/restore
verified replacement
rollback after deterministic rejection
separate primary and rollback statuses
```

Its raw backup and restore probes call `BettributeStore` directly with the
wallpaper attribute name.

## Build impact

Added:

```text
src/BettributeStore.h
src/BettributeStore.cpp
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

The existing smoke target is reused. No user-visible string or catalog key is
added.

## Completion boundary

This is the only BettributeStore expansion planned inside BeMyDailyWall before
a useful wallpaper-setting milestone.

After this phase, work returns to:

```text
Tracker authority
real WallpaperSetter backend
end-to-end local-folder wallpaper application
Menlo and packaging closure
```

The seed repository remains parked until a second real consumer exists.

## Validation

```sh
make clean
make
make smoke-haiku-wallpaper-contract
make smoke
```

No visual gate is required because the Desktop is not changed.

## Proves

- raw node-attribute storage no longer belongs to the wallpaper contract
- the internal store accepts an explicit attribute name
- snapshots own data independently of the node
- wallpaper behavior survives the responsibility split
- the seed repository is not an application dependency

## Does not prove

- extraction readiness for the separate seed repository
- zero-length attribute support
- concurrent-change detection
- filesystem-atomic replacement or rollback
- a second consumer
- Desktop wallpaper mutation
- Tracker restore notification
- release readiness

Smoke tests prove life, not dignity.
