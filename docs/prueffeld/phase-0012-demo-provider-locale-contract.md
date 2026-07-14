# Phase 0012: Demo provider locale contract

Date: 2026-07-14
Baseline commit: `7be604e Localize wallpaper setter errors`

## Scope

This phase gives the user-facing `DemoProvider` metadata its own Locale Kit
context.

It does not translate or rename the provider identifier.

## Translation ownership

`DemoProvider.cpp` defines:

```text
B_TRANSLATION_CONTEXT = DemoProvider
```

The context owns:

```text
Somewhere else
Your daily window to somewhere else.
Not an affiliated provider.
```

These values populate the `WallpaperInfo` title, description, and attribution.

## Stable provider identity

`DemoProvider::Name()` remains:

```text
Demo provider
```

That value currently serves as both persisted provider selection and
`WallpaperInfo` source identity. Translating it would make persisted settings
depend on the active language.

A future distinction between provider identifier and display name must be an
explicit contract.

## Provider smoke

`make smoke-provider` now links `be` and `localestub`.

The smoke verifies the English source fallback for the stable provider name,
title, description, source, attribution, empty image path, and the existing
failing-provider status.

## Catalog update

Regenerate the English source catalog natively:

```sh
rm -f locales/en.catkeys
make catkeys
```

Expected new entries:

```text
Somewhere else                          DemoProvider
Your daily window to somewhere else.   DemoProvider
Not an affiliated provider.            DemoProvider
```

The stable name `Demo provider` must not appear as a `DemoProvider` catalog
entry.

## Validation

```sh
make clean
make
rm -f locales/en.catkeys
make catkeys
test -s locales/en.catkeys
grep -F "DemoProvider" locales/en.catkeys
make catalogs
make bindcatalogs
make smoke-provider
make smoke
```

Manual visual check:

```text
launch BeMyDailyWall
hover the Deskbar preview
verify title, description, source, and attribution remain visible
verify the provider status still names Demo provider
close the window normally
```

## What this phase proves

This phase proves that user-facing demo metadata remains owned by
`DemoProvider`, all three translatable fields are collectable under its context,
the provider identifier remains stable, and the provider smoke covers every
current demo metadata field.

## What this phase does not prove

This phase does not prove that a non-English translation or separate localized
provider display name exists, that a real network provider exists, that dates
are localized, or that remote attribution contracts are settled.

Smoke tests prove life, not dignity.
