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

`DemoProvider.cpp` defines:

```text
B_TRANSLATION_CONTEXT = DemoProvider
```

That context owns the user-facing demo metadata:

```text
Somewhere else
Your daily window to somewhere else.
Not an affiliated provider.
```

`DemoProvider::Name()` remains the stable provider identifier `Demo provider`.
It is not translated because settings and source identity currently use the same
value. A future display-name abstraction must be explicit rather than changing
the persisted identifier by locale.

`WallpaperSetter.cpp` defines:

```text
B_TRANSLATION_CONTEXT = WallpaperSetter
```

That context owns the two current component errors:

```text
No wallpaper image path available.
Wallpaper backend is not implemented yet.
```

`MainWindow` translates only the sentence shell around `%error%`. The error
value itself is produced and translated by `WallpaperSetter`.

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

`make smoke-provider` and `make smoke-setter` link `localestub` because their
test images directly include components that use Locale Kit macros. With no
bound non-English catalog, the smokes verify the English source fallback
together with each component contract.
