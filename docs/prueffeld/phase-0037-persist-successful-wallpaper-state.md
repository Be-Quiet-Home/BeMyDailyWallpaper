# Phase 0037: Persist successful wallpaper state

Date: 2026-07-17
Baseline commit: `771698b Add local folder configuration action`

## Scope

This phase records the already completed wallpaper operation in the existing
settings fields:

```text
last_image_path
last_update_date
```

No scheduling or rotation policy is added.

## Commit boundary

History is written only after:

```text
Desktop target resolution succeeded
WallpaperSetter::Apply() returned B_OK
Tracker restore notification succeeded
```

Provider fetch, folder selection, or failed wallpaper operations never update
the history fields.

## Date contract

The update date is the local calendar date at confirmed wallpaper success.

Stable stored format:

```text
YYYY-MM-DD
```

The value is produced from the system clock with:

```text
time()
localtime_r()
strftime("%Y-%m-%d")
```

No localized display string is stored.

## Persistence flow

After setter success:

```text
capture previous in-memory history values
build current local date
set LastImagePath to ProviderResult::ImagePath()
set LastUpdateDate to YYYY-MM-DD
AppSettings::Save()
```

On save failure, the previous in-memory values are restored.

The wallpaper itself is not rolled back because its operation already completed
successfully. Settings history is a secondary record, not part of the Desktop
transaction.

## User-visible states

Complete success:

```text
Wallpaper applied and history saved.
```

Completed wallpaper operation with failed history persistence:

```text
Wallpaper applied, but history save failed: <Haiku error>
```

The second message intentionally reports both truths.

## Locale impact

New `MainWindow` keys:

```text
Wallpaper applied and history saved.
Wallpaper applied, but history save failed: %error%
```

The earlier key:

```text
Wallpaper applied.
```

is no longer used and should disappear after `make catkeys`.

## Validation

```sh
make catkeys
make clean
make
make smoke
```

## Real gate

Apply a wallpaper successfully, close the application, and inspect the
persisted settings through the existing application behavior or a later
dedicated state display.

The wallpaper must change before the history fields are written.

## Proves

- successful wallpaper application records the exact applied image path
- the stored date is locale-independent and day-comparable
- failed wallpaper operations cannot advance history
- history-save failure does not deny an already completed Desktop change
- failed persistence restores previous in-memory history values

## Does not prove

- automatic once-per-day decisions
- selection of a different image from the previous path
- missed-day catch-up
- settings history presentation in the UI
- Deskbar scheduling

Smoke tests prove life, not dignity.
