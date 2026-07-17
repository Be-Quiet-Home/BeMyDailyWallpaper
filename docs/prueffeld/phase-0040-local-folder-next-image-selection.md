# Phase 0040: Local-folder next-image selection

Date: 2026-07-17
Baseline commit: `b4507d8 Extract daily wallpaper policy`

## Scope

This phase adds a deterministic previous-image selection seam to
`LocalFolderProvider`.

It does not connect the seam to settings, the resolver, the daily policy, or an
automatic wallpaper operation.

## Constructor contract

The existing constructor remains:

```cpp
explicit LocalFolderProvider(const char* directoryPath);
```

It preserves the current behavior and selects the first recognized image.

New injected constructor:

```cpp
LocalFolderProvider(
    const char* directoryPath,
    const char* previousImagePath);
```

Both strings are copied into provider-owned `BString` storage.

## Candidate order

The provider continues to:

```text
enumerate non-recursively
accept jpg, jpeg, and png suffixes case-insensitively
verify image structure through Translation Kit
ignore unsupported and invalid files
```

All recognized candidates are then sorted by bytewise filename.

Directory enumeration order therefore cannot affect selection.

## Selection rule

```text
no previous path
    -> first candidate

exact previous absolute path found
    -> next candidate

previous path is final candidate
    -> first candidate

previous path not found
    -> first candidate
```

Only an exact absolute-path match advances the selection. A same-named image
from another directory cannot affect the result.

## Default product behavior

`ProviderResolver` still constructs:

```cpp
LocalFolderProvider(settings.LocalFolderPath())
```

The application therefore keeps its current first-image behavior in this phase.

The new constructor is injected only by the smoke. Wiring `LastImagePath` into
the resolver is a separate later gate.

## Smoke extension

The existing local-folder provider smoke now proves:

```text
default constructor -> Alpha.JPG
previous Alpha.JPG -> middle.jpeg
previous middle.jpeg -> zeta.png
previous zeta.png -> Alpha.JPG
missing previous path -> Alpha.JPG
```

Existing path, extension, Translation Kit, metadata, and error checks remain.

## Locale impact

No user-visible string changes.

`make catkeys` is not required.

## Validation

```sh
make clean
make
make smoke-local-folder-provider
make smoke
```

No visual gate is required because application behavior is unchanged.

## Proves

- recognized candidates have a deterministic total order
- an injected prior path advances exactly one image
- the final image wraps to the first
- a removed previous image has a safe deterministic fallback
- the existing application path remains unchanged
- no random generator, database, or scheduler is required

## Does not prove

- `LastImagePath` is passed through `ProviderResolver`
- the window refreshes to the next image after success
- the daily policy triggers provider fetching
- an automatic wallpaper operation
- recursive directory traversal
- Deskbar scheduling

Smoke tests prove life, not dignity.
