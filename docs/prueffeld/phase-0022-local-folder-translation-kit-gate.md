# Phase 0022: Local folder Translation Kit gate

Date: 2026-07-14
Baseline commit: `683b1af Add deterministic local folder provider`

## Scope

This phase adds a Haiku-native content gate to `LocalFolderProvider`.

A supported filename suffix remains the cheap first filter. A candidate is only
eligible when Haiku's Translation Kit also recognizes its contents as
translatable to a bitmap.

The provider still does not change the desktop wallpaper.

## Local authority

The gate uses:

```text
BFile
BTranslatorRoster::Default()
BTranslatorRoster::Identify()
B_TRANSLATOR_BITMAP
```

The default roster represents the translators installed on the running Haiku
system.

The provider does not ship its own JPEG or PNG parser.

## Identification contract

For each regular file whose name ends in:

```text
.jpg
.jpeg
.png
```

the provider:

1. opens the file read-only,
2. asks the default translator roster to identify a route to
   `B_TRANSLATOR_BITMAP`,
3. skips the candidate when the roster returns `B_NO_TRANSLATOR`,
4. propagates other file or Translation Kit errors,
5. includes recognized candidates in deterministic filename selection.

This keeps malformed image-shaped files out while preserving operational
errors.

## Selection contract

Deterministic selection remains bytewise lexicographic by leaf filename, but
only among Translation Kit-recognized images.

Example:

```text
000-empty.jpg       empty and not recognized
Alpha.JPG           recognized one-pixel PNG content
middle.jpeg         recognized one-pixel PNG content
zeta.png            recognized one-pixel PNG content
```

Selected:

```text
Alpha.JPG
```

The file suffix is a candidate policy. Translation Kit-recognized content is
the authority.

## Smoke fixture

The smoke embeds one complete 1 x 1 PNG fixture.

It first creates only:

```text
000-not-an-image.txt
001-fake.jpg/       directory
000-empty.jpg       invalid image content
```

The provider must return:

```text
B_ENTRY_NOT_FOUND
```

It then writes valid PNG content under three supported filenames and verifies
that deterministic selection still chooses `Alpha.JPG`.

The smoke does not depend on repository fixture files or user data.

## Build impact

The application and local-folder smoke now link Haiku's Translation Kit:

```text
translation
```

Changed production file:

```text
src/LocalFolderProvider.cpp
```

Changed smoke:

```text
tests/local_folder_provider_smoke.cpp
```

No new public header or provider interface is introduced.

## Catalog impact

This phase introduces no user-facing text and no Locale Kit keys.

No Catkey or catalog regeneration is required.

## Efficiency boundary

Identification asks translators to inspect the source and determine whether a
bitmap conversion is available.

This phase deliberately does not allocate and retain a decoded `BBitmap` for
every candidate. Full decode belongs at the later consumption or application
boundary.

## Explicit boundaries

This phase does not:

- validate every pixel by a complete decode,
- accept every image format known to Translation Kit,
- remove the `.jpg`, `.jpeg`, and `.png` filename policy,
- recurse into subdirectories,
- randomize or rotate selection,
- wire `LocalFolderProvider` into settings or `MainWindow`,
- set the desktop wallpaper.

## Validation

```sh
make clean
make
make smoke-local-folder-provider
make smoke
```

No visual gate is required because application provider selection is unchanged.

## What this phase proves

This phase proves that:

- empty or malformed image-shaped files are not selected,
- installed Haiku translators define image recognition,
- candidate validation stays inside Haiku's native Translation Kit,
- deterministic selection operates only on recognized images,
- a valid local image path still reaches `ProviderResult`.

## What this phase does not prove

This phase does not prove that:

- every recognized file can be fully decoded without later failure,
- image dimensions are suitable for a desktop,
- image data is trustworthy beyond translator parsing,
- the selected path can be applied as wallpaper,
- daily rotation is defined,
- the application is release-ready.

Smoke tests prove life, not dignity.
