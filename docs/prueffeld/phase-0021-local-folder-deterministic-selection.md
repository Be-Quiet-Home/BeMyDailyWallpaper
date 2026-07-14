# Phase 0021: Local folder deterministic selection

Date: 2026-07-14
Baseline commit: `57ae565 Test provider info replacement`

## Scope

This phase starts the first real product path by adding
`LocalFolderProvider`.

The provider reads an actual directory and returns an actual local file path.
It does not change the desktop wallpaper.

## Provider identity

The stable provider name is:

```text
Local folder
```

The name is used as source metadata and is not localized in this phase.

## Constructor seam

The provider receives one directory path:

```cpp
LocalFolderProvider provider(directoryPath);
```

Keeping the path explicit gives the smoke test an isolated temporary directory
and avoids touching user folders.

Settings and UI wiring come later.

## Candidate inventory

The provider uses Haiku Storage Kit objects:

```text
BDirectory
BEntry
BPath
```

A candidate must be a regular file with one of these filename suffixes:

```text
.jpg
.jpeg
.png
```

Suffix matching is case-insensitive.

Directories are not candidates even when their names end in an image suffix.
Unsupported files are ignored.

## Deterministic selection

Among supported regular files, the provider chooses the bytewise
lexicographically smallest filename.

Example fixture:

```text
000-not-an-image.txt
001-fake.jpg/        directory
zeta.png
Alpha.JPG
middle.jpeg
```

Selected:

```text
Alpha.JPG
```

The result does not depend on the order returned by `BDirectory`.

## Provider result

On success the provider returns:

```text
status:       B_OK
title:        selected leaf filename
description:  empty
source:       Local folder
attribution:  empty
date:         empty
image path:   selected absolute path
```

When the directory contains no supported regular file:

```text
B_ENTRY_NOT_FOUND
```

## Smoke target

New target:

```sh
make smoke-local-folder-provider
```

The smoke creates and cleans an isolated directory under
`B_SYSTEM_TEMP_DIRECTORY`.

It verifies:

- stable provider name,
- empty-directory status,
- unsupported-file omission,
- directory omission,
- case-insensitive suffix matching,
- deterministic selection,
- absolute selected path,
- complete metadata result.

The aggregate `make smoke` includes this target.

## Build impact

New production files:

```text
src/LocalFolderProvider.h
src/LocalFolderProvider.cpp
```

New smoke:

```text
tests/local_folder_provider_smoke.cpp
```

The application build compiles the provider, but `MainWindow` still uses
`DemoProvider`.

## Catalog impact

This phase introduces no Locale Kit keys.

Filenames are provider data. `Local folder` is a stable provider identifier in
the same sense as `Demo provider`.

No Catkey or catalog regeneration is required.

## Explicit boundaries

This phase does not:

- inspect image bytes,
- consult the Translation Kit,
- recurse into subdirectories,
- follow symlink policy beyond the current non-traversing entry enumeration,
- randomize selection,
- rotate by date,
- read provider settings,
- wire the provider into `MainWindow`,
- set the desktop wallpaper.

Those are later bricks.

## Validation

```sh
make clean
make
make smoke-local-folder-provider
make smoke
```

No visual gate is required because the provider is not yet selected by the
application.

## What this phase proves

This phase proves that:

- a provider can read a real Haiku directory,
- supported local image names can be inventoried,
- selection is deterministic,
- directories and unrelated files are excluded,
- a real absolute image path reaches `ProviderResult`,
- the existing provider seam supports a filesystem-backed implementation.

## What this phase does not prove

This phase does not prove that:

- the selected file contains a valid image,
- every Haiku-supported image format is recognized,
- the selected file can be decoded,
- the selected file can be applied as wallpaper,
- daily rotation is defined,
- user folder settings exist,
- the application is release-ready.

Smoke tests prove life, not dignity.
