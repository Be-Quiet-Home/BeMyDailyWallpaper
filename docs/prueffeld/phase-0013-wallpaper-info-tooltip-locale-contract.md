# Phase 0013: Wallpaper info tooltip locale contract

Date: 2026-07-14
Baseline commit: `14ec284 Localize demo provider metadata`

## Scope

This phase gives `WallpaperInfo::TooltipText()` its own Locale Kit context for
tooltip field lines.

Provider metadata itself remains unchanged and provider-owned.

## Translation ownership

`WallpaperInfo.cpp` defines:

```text
B_TRANSLATION_CONTEXT = WallpaperInfo
```

The context owns these complete lines:

```text
Today: %title%
Source: %source%
Date: %date%
```

The placeholders are replaced after catalog lookup.

The following values are not translated by `WallpaperInfo`:

```text
title
description
source
attribution
date value
```

They are supplied by a provider or another upstream component.

The tooltip header `BeMyDailyWall` also remains untranslated.

## Omission contract

Empty fields do not create blank labeled lines.

The formatting contract is:

```text
always:
    BeMyDailyWall

when present:
    Today: <title>
    <description>
    Source: <source>
    <attribution>
    Date: <date>
```

Description and attribution remain unlabeled free text.

## Wallpaper info smoke

The new target is:

```sh
make smoke-wallpaper-info
```

It verifies:

- complete tooltip order and punctuation,
- English source fallback for all three localized field lines,
- omission of empty optional fields,
- header-only output for entirely empty metadata.

The smoke image links `be` and `localestub`.

## Catalog update

Regenerate the English source catalog natively:

```sh
rm -f locales/en.catkeys
make catkeys
```

Expected new entries:

```text
Today: %title%     WallpaperInfo
Source: %source%   WallpaperInfo
Date: %date%       WallpaperInfo
```

## Validation

```sh
make clean
make
rm -f locales/en.catkeys
make catkeys
test -s locales/en.catkeys
grep -F "WallpaperInfo" locales/en.catkeys
make catalogs
make bindcatalogs
make smoke-wallpaper-info
make smoke
```

Manual visual check:

```text
launch BeMyDailyWall
hover the Deskbar preview
verify Today and Source labels remain visible
verify no placeholder token is visible
close the window normally
```

The demo provider currently supplies no date, so the dedicated smoke proves the
date line separately.

## What this phase proves

This phase proves that:

- tooltip labels remain owned by `WallpaperInfo`,
- complete field lines preserve translator-controlled word order,
- provider values are inserted only after translation,
- empty metadata does not create stray labels,
- the date line is covered even though the demo provider omits it,
- aggregate smoke includes the tooltip contract.

## What this phase does not prove

This phase does not prove that:

- a non-English translation exists,
- provider date values are locale-formatted,
- long translated labels have been visually reviewed,
- right-to-left tooltip presentation has been reviewed,
- the tooltip header should eventually be translated,
- final Deskbar presentation is settled,
- the application is release-ready.

Smoke tests prove life, not dignity.
