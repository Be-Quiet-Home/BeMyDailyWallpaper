# Phase 0010: Main window locale baseline

Date: 2026-07-14
Baseline commit: `dad1e5d Use native layout for main window`

## Scope

This phase registers the sentences owned by `MainWindow` with Haiku's Locale
Kit.

It configures only the English source catalog. No translated language is added.

## Native integration

The application links:

```text
be
localestub
```

Locale Kit classes come from `libbe`. `liblocalestub` identifies the
application image to the catalog system.

The Generic Makefile declares:

```text
APP_MIME_SIG = x-vnd.BeQuietHome-BeMyDailyWall
LOCALES       = en
```

The runtime `BApplication` signature remains:

```text
application/x-vnd.BeQuietHome-BeMyDailyWall
```

`MainWindow.cpp` includes `Catalog.h`, defines the translation context
`MainWindow`, and uses `B_TRANSLATE` or `B_TRANSLATE_COMMENT` for its visible
sentences.

## Placeholder policy

Status sentences are collected as complete translatable sentences rather than
as punctuation fragments.

Dynamic values use named placeholders:

```text
%provider%
%archive%
%error%
```

The code replaces those placeholders only after catalog lookup.

This keeps sentence order under translator control while leaving provider names
and component error values unchanged.

## Ownership boundary

Included:

- `MainWindow` liveness sentence
- complete settings-status variants
- enabled and disabled archive states
- preview descriptions
- complete provider-status variants
- complete setter-status shell sentences

Not included:

- product name
- `DemoProvider` metadata
- `WallpaperSetter` error details
- `WallpaperInfo` tooltip field formatting

Those strings stay with the components that own their meaning.

## Validation

```sh
make clean
make
make catkeys
test -s locales/en.catkeys
make catalogs
make bindcatalogs
make smoke
```

Manual visual check after binding:

```text
launch BeMyDailyWall
verify all diagnostic text remains English
verify placeholders are replaced
verify no %provider%, %archive%, or %error% token is visible
verify the window still closes normally
```

## What this phase proves

This phase proves that:

- the application links Locale Kit support through `be` and `localestub`,
- the catalog signature follows Haiku's `x-vnd` convention while the runtime
  application signature retains the `application/` supertype,
- `MainWindow` strings are collectable into an English `.catkeys` file,
- complete status sentences preserve translator-controlled word order,
- compiled catalogs can be bound into the local application binary,
- application liveness remains covered after catalog binding.

## What this phase does not prove

This phase does not prove that:

- a non-English translation exists,
- provider metadata is localized,
- setter-owned errors are localized,
- tooltip formatting is localized,
- every future visible string is automatically covered,
- right-to-left layouts have been reviewed,
- localization quality has been reviewed by a translator,
- the application is release-ready.

Smoke tests prove life, not dignity.
