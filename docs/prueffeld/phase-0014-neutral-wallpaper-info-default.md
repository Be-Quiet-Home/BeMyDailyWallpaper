# Phase 0014: Neutral wallpaper info default

Date: 2026-07-14
Baseline commit: `f2efbfc Localize wallpaper tooltip labels`

## Scope

This phase removes hidden demo content from the parameterless
`WallpaperInfo` constructor.

The default object becomes a neutral empty metadata container.

## Previous behavior

A default `WallpaperInfo` silently contained:

```text
title:       Somewhere else
description: Your daily window to somewhere else.
source:      BeMyDailyWall
```

That mixed demo policy into a general-purpose value object.

Because both `ProviderResult` and `DeskbarView` construct `WallpaperInfo`
without arguments, those demo values existed before a provider had produced any
result.

## New default contract

The parameterless constructor initializes all five metadata fields to empty
strings:

```text
title
description
source
copyright / attribution
date
```

`TooltipText()` still returns the stable product header:

```text
BeMyDailyWall
```

The header is presentation structure, not provider metadata.

## Ownership

Demo content remains owned exclusively by `DemoProvider`.

The data flow is now explicit:

```text
neutral WallpaperInfo
    -> provider Fetch()
    -> provider-owned WallpaperInfo metadata
    -> ProviderResult
    -> DeskbarView
```

Container construction no longer invents user-facing provider data.

## Smoke extension

The existing target remains:

```sh
make smoke-wallpaper-info
```

It now additionally verifies:

- every field of a default `WallpaperInfo` is empty,
- a neutral default produces only the tooltip header,
- explicit complete and partial metadata formatting remains unchanged.

No new smoke binary or build rule is introduced.

## Catalog impact

This phase does not add, remove, or alter translation keys.

The removed constructor defaults were not Locale Kit keys. Regenerating
`locales/en.catkeys` is therefore unnecessary.

## Validation

```sh
make clean
make
make smoke-wallpaper-info
make smoke
```

Manual visual check:

```text
launch BeMyDailyWall
hover the Deskbar preview
verify DemoProvider metadata still appears after its successful fetch
close the window normally
```

## What this phase proves

This phase proves that:

- `WallpaperInfo` is neutral after default construction,
- `ProviderResult` begins with neutral metadata,
- `DeskbarView` cannot inherit hidden demo metadata from construction,
- demo data enters only through `DemoProvider`,
- tooltip formatting for explicit metadata remains stable.

## What this phase does not prove

This phase does not prove that:

- provider failure presentation is final,
- an entirely blank tooltip should replace the product header,
- a real provider exists,
- metadata validity rules are complete,
- the final Deskbar lifecycle is settled,
- the application is release-ready.

Smoke tests prove life, not dignity.
