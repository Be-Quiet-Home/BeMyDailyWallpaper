# Phase 0023: Local folder path status matrix

Date: 2026-07-14
Baseline commit: `db3ad8a Validate local images with Translation Kit`

## Scope

This phase defines the input-path status contract of `LocalFolderProvider`.

It does not change image inventory, Translation Kit validation, deterministic
selection, or application wiring.

## Path status matrix

`Fetch()` now distinguishes these inputs explicitly:

```text
empty path             B_BAD_VALUE
missing path           B_ENTRY_NOT_FOUND
path naming a file     B_NOT_A_DIRECTORY
existing directory     continue with provider inventory
```

These statuses are established before directory enumeration begins.

## Storage Kit boundary

The provider first resolves the configured path through a traversing `BEntry`.

It then checks:

```text
InitCheck()
Exists()
IsDirectory()
```

Only a confirmed directory is passed to `BDirectory`.

This keeps path-shape errors explicit instead of depending on lower-level open
behavior.

## Missing path and empty result

`B_ENTRY_NOT_FOUND` has two provider-level meanings:

```text
the configured path does not exist
the configured directory contains no recognized image
```

Both mean that the local source currently cannot produce an image.

Configuration absence and wrong node type remain separately visible through
`B_BAD_VALUE` and `B_NOT_A_DIRECTORY`.

## Smoke extension

The existing isolated local-folder smoke now verifies:

```text
LocalFolderProvider("")
    -> B_BAD_VALUE

LocalFolderProvider(missingPath)
    -> B_ENTRY_NOT_FOUND

LocalFolderProvider(regularFilePath)
    -> B_NOT_A_DIRECTORY
```

The previous checks remain active:

```text
empty directory
unsupported files
directory-shaped image name
invalid image content
Translation Kit-recognized image content
deterministic selection
complete ProviderResult
```

## Build impact

Changed production file:

```text
src/LocalFolderProvider.cpp
```

Changed smoke:

```text
tests/local_folder_provider_smoke.cpp
```

No new source file, public method, target, library, or catalog key is introduced.

## Validation

```sh
make clean
make
make smoke-local-folder-provider
make smoke
```

No visual gate or catalog regeneration is required.

## What this phase proves

This phase proves that:

- an absent configuration path is distinguishable,
- a missing source path is reported consistently,
- a regular file cannot masquerade as the configured folder,
- valid directories still enter the existing inventory path,
- previous local-image validation remains active.

## What this phase does not prove

This phase does not prove that:

- permissions errors are normalized,
- every filesystem failure has a provider-specific status,
- symlink policy requires a separate user option,
- the path is stored in `AppSettings`,
- the provider is selected by `MainWindow`,
- the application is release-ready.

Smoke tests prove life, not dignity.
