# Localization

BeMyDailyWall uses Haiku's Locale Kit and Generic Makefile localization targets.

## Build contract

The Makefile declares:

```text
APP_MIME_SIG = x-vnd.BeQuietHome-BeMyDailyWall
LIBS          = be localestub ...
LOCALES       = en
```

The Generic Makefile catalog signature intentionally omits the `application/`
MIME supertype. Its localization rules use the value as a directory name and as
the catalog signature.

The runtime application signature remains:

```text
application/x-vnd.BeQuietHome-BeMyDailyWall
```

Locale Kit classes are provided by `libbe`. Images that use the translation
macros additionally link `liblocalestub` so Haiku can associate the image with
its catalogs.

English is the source catalog and the only configured locale in the current
brick. Additional languages require a deliberate project decision and a
reviewed `.catkeys` file; they are not added speculatively.

## Current ownership boundary

`MainWindow.cpp` defines:

```text
B_TRANSLATION_CONTEXT = MainWindow
```

The context owns:

- liveness text
- settings status sentences
- archive state labels
- Deskbar preview descriptions
- provider status sentences around the provider name
- wallpaper setter status sentences around component results

Dynamic values use named placeholders such as:

```text
%provider%
%archive%
%error%
```

Translator comments explain those placeholders.

Provider-owned metadata remains in `DemoProvider`.

Setter-owned error details remain in `WallpaperSetter` and are inserted into a
translated `MainWindow` sentence. Their own translation context will be added
when those components receive a dedicated localization brick.

## Native commands

Collect the English source keys:

```sh
make catkeys
```

Compile configured catalogs:

```sh
make catalogs
```

Bind catalogs into the application for a self-contained local test binary:

```sh
make bindcatalogs
```

The generated source file is:

```text
locales/en.catkeys
```

It is versioned because it is the reviewable source contract for translators.
Compiled `.catalog` files remain build artifacts under the object directory.

## Current boundary

This brick establishes the Locale Kit path; it does not add a translated
language.

The product name `BeMyDailyWall` remains untranslated.

Localization completeness is evaluated component by component. A string is not
moved into `MainWindow` merely to make it translatable there.
