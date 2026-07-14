# Phase 0011: Wallpaper setter locale contract

Date: 2026-07-14
Baseline commit: `dcd25a5 Add MainWindow locale baseline`

## Scope

This phase moves the two existing `WallpaperSetter` error strings into the
component's own Locale Kit context.

It also gives the setter an explicit smoke target.

No wallpaper backend is implemented.

## Translation ownership

`WallpaperSetter.cpp` defines:

```text
B_TRANSLATION_CONTEXT = WallpaperSetter
```

The component owns:

```text
No wallpaper image path available.
Wallpaper backend is not implemented yet.
```

`MainWindow` continues to own only:

```text
Wallpaper setter: %error%
```

The translated component error is inserted into that translated sentence shell.

## Setter smoke

The new target is:

```sh
make smoke-setter
```

The smoke verifies:

```text
empty image path
    -> B_BAD_VALUE
    -> No wallpaper image path available.

non-empty image path
    -> B_NOT_SUPPORTED
    -> Wallpaper backend is not implemented yet.
```

The smoke executable links `be` and `localestub` because it directly contains
translation macros through `WallpaperSetter.cpp`.

The exact English text is the expected source fallback when no translated
catalog is bound into the smoke executable.

## Catalog update

The versioned English source catalog is regenerated natively:

```sh
rm -f locales/en.catkeys
make catkeys
```

Expected new context entries:

```text
No wallpaper image path available.          WallpaperSetter
Wallpaper backend is not implemented yet.  WallpaperSetter
```

## Validation

```sh
make clean
make
make catkeys
test -s locales/en.catkeys
grep -F "WallpaperSetter" locales/en.catkeys
make catalogs
make bindcatalogs
make smoke-setter
make smoke
```

Manual visual check:

```text
launch BeMyDailyWall
verify the setter status still shows the missing-image error
verify no %error% placeholder is visible
close the window normally
```

## What this phase proves

This phase proves that:

- setter-specific visible errors remain owned by `WallpaperSetter`,
- both current setter errors are collectable under the `WallpaperSetter`
  translation context,
- both existing setter status contracts remain explicit,
- the English source fallback remains stable,
- aggregate smoke includes the setter contract,
- `MainWindow` does not absorb component-owned error translations.

## What this phase does not prove

This phase does not prove that:

- a real wallpaper backend exists,
- a non-English translation exists,
- translated text has been reviewed,
- all future setter errors are automatically covered,
- runtime backend failures have been classified,
- error presentation beyond the diagnostic window is settled,
- the application is release-ready.

Smoke tests prove life, not dignity.
