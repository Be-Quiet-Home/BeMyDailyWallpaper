# Architecture

BeMyDailyWall is a small Haiku-native daily wallpaper companion.

It is built in pedantic mode: small explicit components, clear ownership,
visible status paths, and no premature feature jumps.

## Current components

### App

`BeMyDailyWallApp` owns the Haiku application lifecycle.

It creates the main window during startup.

### MainWindow

`MainWindow` is the current visible test bench.

It shows:

- application liveness
- settings status
- Deskbar icon preview
- provider status
- current daily application status
- one native startup-apply preference checkbox
- selected wallpaper-folder name
- one native directory-selection action
- wallpaper action status
- one explicit `Apply wallpaper` button

The folder action opens a single-selection `BFilePanel` restricted to directory
nodes. A successful selection changes the persisted provider to `Local folder`,
stores the chosen path, reloads the provider in the same window, refreshes the
preview and status labels, and enables `Apply wallpaper` when an image was
recognized.

The apply button is enabled only after a successful provider fetch with a
non-empty image path. Application startup remains non-mutating. A button message
resolves the real Desktop and Tracker targets and constructs the injected
`WallpaperSetter`. `MainWindow` then supplies small callbacks to
`DailyWallpaperAction`, which owns the apply/date/history flow while the window
retains control presentation and provider reload.

The window is not the final product center. It is a development and diagnostic
surface while the small system parts are being wired together.

Its ordinary child views are arranged through Haiku's `BGroupLayout` and
`BLayoutBuilder`. Fixed `BRect` coordinates are not used for labels or the
Deskbar preview.

The window-owned diagnostic sentences use Haiku's Locale Kit with the
`MainWindow` translation context. Provider metadata and setter-owned error text
remain owned by their source components.

The window retains `AppSettings` and `ProviderResult`, but provider objects
remain short-lived. `ReloadProvider()` asks `ProviderResolver` for the selected
provider, calls one synchronous `Fetch()`, copies the result into window-owned
state, deletes the provider, and updates the visible controls.

Folder selection preserves the previous in-memory provider and path when
settings persistence fails. After a successful save, the settings status changes
to loaded and the local-folder provider is reloaded without restarting the app.

The startup-apply checkbox is initialized from
`AppSettings::StartupApplyEnabled()`. A user change is saved immediately through
`AppSettings::Save()`. Save failure restores both the previous in-memory setting
and the visible checkbox value. The checkbox does not invoke
`DailyWallpaperStartupPlan` and cannot change the Desktop in this phase.

### DailyWallpaperPolicy

`DailyWallpaperPolicy` owns the pure daily-date decision boundary.

Current state:

- produces the current local calendar date as stable `YYYY-MM-DD`
- evaluates caller-supplied history and current-date strings
- reports unavailable when no current date is available
- reports applied-today only for an exact date match
- reports pending for empty, older, future, or otherwise non-matching history
- combines that daily state with caller-supplied candidate availability
- reports unavailable, already-applied, no-candidate, or ready
- gives date unavailability and already-applied state authority over candidates
- has no settings, provider, Desktop, Tracker, UI, timer, or scheduler knowledge

`MainWindow` supplies `ProviderResult::HasImagePath()` only after provider
loading has completed and translates the returned readiness into visible text.
The policy smoke injects fixed dates and candidate flags and separately verifies
the live date format.

### DailyWallpaperStartupPlan

`DailyWallpaperStartupPlan` owns the narrow one-shot startup decision boundary.

Current state:

- maps only `DAILY_WALLPAPER_READINESS_READY` to `APPLY_ONCE`
- maps unavailable, already-applied, no-candidate, and unknown readiness to
  `DO_NOTHING`
- executes `DO_NOTHING` without requiring or invoking a callback
- requires one caller-supplied function pointer for `APPLY_ONCE`
- invokes that callback exactly once and returns its `status_t` unchanged
- performs no retry, settings write, provider fetch, Desktop operation, or
  Tracker notification
- is compiled into the application but is not called by `MainWindow`

The startup-plan smoke uses a counting callback. No real wallpaper target is
resolved or mutated. A separate cross-brick smoke now executes
`DailyWallpaperAction` through the startup-plan callback seam. This composition
still has no production caller or real target.

### DailyWallpaperAction

`DailyWallpaperAction` owns the reusable apply-and-history sequence.

Current state:

- rejects a missing image path before invoking callbacks
- requires injected apply, current-date, and settings-save callbacks
- invokes the apply callback exactly once
- preserves the callback-provided rollback status separately
- obtains the update date only after successful wallpaper application
- rejects an empty successful date result
- updates `LastImagePath` and `LastUpdateDate` before the save callback
- restores the previous in-memory history values when saving fails
- reports apply, history, and rollback statuses independently
- performs no provider reload, UI update, target resolution, retry, or scheduling

`MainWindow` provides the real adapters: `WallpaperSetter::Apply()`,
`DailyWallpaperPolicy::CurrentLocalDate()`, and `AppSettings::Save()`. The action
smoke supplies counting in-memory callbacks and never opens the Desktop or the
user settings file.

The startup-action coordination smoke composes this action with
`DailyWallpaperStartupPlan` using only in-memory callbacks. It proves the two
bricks fit without adding a production coordinator class.

### AppSettings

`AppSettings` owns application defaults and persisted settings state.

Current state:

- default provider name
- local folder path
- archive enabled flag
- startup apply enabled flag, defaulting to false
- last image path
- last update date
- Haiku-native flattened `BMessage` persistence
- default storage under `B_USER_SETTINGS_DIRECTORY`
- explicit-path storage seam for isolated smoke tests

Normal application code uses the default settings path. Tests use a temporary
path and do not touch the user's settings file. The local folder path defaults
to empty and is stored as one required `B_STRING_TYPE` field named
`local_folder_path`.

`startup_apply_enabled` is persisted as `B_BOOL_TYPE` and defaults to false. It
is optional while reading older settings files, but validated as a single bool
when present and always written by current saves. `MainWindow` exposes the value
through one native checkbox and persists user changes immediately. No production
startup path reads the flag yet.

### DeskbarView

`DeskbarView` is the future Deskbar/Replicant view.

Current state:

- draws a small placeholder icon
- accepts `WallpaperInfo`
- exposes the wallpaper information through a tooltip
- is currently previewed inside `MainWindow`
- provides a layout-aware 32 x 32 constructor for that preview

The placeholder circle is not the final icon.

### WallpaperInfo

`WallpaperInfo` stores user-facing wallpaper metadata:

- title
- description
- source
- copyright / attribution
- date

Its default constructor leaves every metadata field empty. Demo or provider
content must enter through an explicit provider result and is never implied by
container construction.

It builds the tooltip text used by `DeskbarView`.

The tooltip header remains the product name. Complete title, source, and date
lines are localized by `WallpaperInfo`; provider metadata is inserted after
catalog lookup.

### ProviderResult

`ProviderResult` carries provider output:

- `WallpaperInfo`
- image path

A default `ProviderResult` therefore contains neutral empty metadata and an
empty image path. `SetInfo()` copies all five metadata fields into result-owned
storage; later replacement of the source `WallpaperInfo` does not change the
stored result. A subsequent `SetInfo()` call replaces all five stored fields.
The provider contract smoke verifies these copy and replacement boundaries and
the neutral state before both its successful and failing provider probes.

The image path is optional and may remain empty after a successful
metadata-only provider fetch. `SetImagePath()` replaces the stored string;
`HasImagePath()` reflects whether that string is non-empty. The provider
contract smoke verifies setting and clearing this state.

Only a result returned with `B_OK` is consumed; this contract does not require
providers to preserve the input object after a failed fetch.

### DailyImageProvider

`DailyImageProvider` is the provider interface.

Providers supply daily image metadata and later image files.

### DemoProvider

`DemoProvider` is the current dry test provider.

It returns localized user-facing demo metadata, an explicitly empty date, and
no image path. The provider contract smoke covers all five `WallpaperInfo`
fields and verifies image-path emptiness through both `ImagePath()` and
`HasImagePath()` after a successful fetch.

Its `Name()` value remains the stable provider identifier used by settings and
source metadata.

### LocalFolderProvider

`LocalFolderProvider` is the first filesystem-backed provider.

Current state:

- receives one directory path at construction
- optionally receives the previously applied absolute image path
- returns `B_BAD_VALUE` for an empty directory path
- returns `B_ENTRY_NOT_FOUND` for a missing path
- returns `B_NOT_A_DIRECTORY` when the path names a regular file
- resolves the path through `BEntry` before directory enumeration
- enumerates entries with Haiku's Storage Kit
- considers regular `.jpg`, `.jpeg`, and `.png` files
- matches those suffixes without case sensitivity
- asks Haiku's Translation Kit whether each candidate can become a bitmap
- skips files that no installed translator recognizes as an image
- sorts recognized candidates by bytewise filename
- selects the first image when no prior image is injected
- selects the next image after an exact prior-path match
- wraps from the final image back to the first
- falls back to the first image when the prior image is no longer present
- returns the filename as title, `Local folder` as source, and the absolute path
- returns `B_ENTRY_NOT_FOUND` when no recognized image is present

Selection is intentionally independent of directory enumeration order.
`ProviderResolver` now passes both the configured directory and
`LastImagePath()`. The one-argument constructor remains available for isolated
first-image use and existing direct callers.

Translation Kit identification validates image structure without fully decoding
the selected image into application-owned bitmap memory. Recursive traversal
and automatic daily execution are not part of the current provider.

### ProviderResolver

`ProviderResolver` maps persisted provider identity to one concrete provider
instance.

Current mappings:

```text
Demo provider -> DemoProvider
Local folder  -> LocalFolderProvider(
                     LocalFolderPath(),
                     LastImagePath())
```

The caller passes an empty output pointer and owns the returned provider.
An occupied output pointer returns `B_BAD_VALUE`. An unknown provider name
returns `B_NAME_NOT_FOUND` without creating an object.

The resolver does not fetch provider data. `MainWindow` owns each resolved
instance only for one synchronous `Fetch()` call and then deletes it. The
resolver smoke uses two real Translation-Kit-recognized images to prove that the
persisted last path reaches the Local-folder selection seam.

### BettributeStore

`BettributeStore` is the current internal proving-ground boundary for one named
Haiku node attribute.

Current state:

- accepts a caller-supplied `BNode` and attribute name
- writes one typed raw value with exact-size checks and `Sync()`
- captures attribute absence as a neutral `BettributeSnapshot`
- captures an existing attribute as its actual type, size, and owned bytes
- restores either the exact raw attribute or its previous absence
- has no knowledge of wallpaper fields, Tracker messages, MIME meaning, or UI
- provides no sidecar, database, query, retry, or cross-node abstraction

The separately named BettributeStore seed repository is not a dependency of this
application. BeMyDailyWall remains the proving ground until the boundary has a
second real consumer and a dedicated capability contract.

### TrackerNotifier

`TrackerNotifier` owns the narrow Tracker restore-notification boundary.

Current state:

- names the locally verified Tracker signature without including private headers
- exposes the public `B_RESTORE_BACKGROUND_IMAGE` command
- resolves a `BMessenger` for the running Tracker when explicitly requested
- sends the restore command only to a caller-supplied valid `BMessenger`
- returns `B_BAD_VALUE` for an invalid injected target
- has no knowledge of wallpaper attributes, image paths, providers, settings, or UI

The signature string is a locally verified compatibility fact, not a public
Haiku constant. The canonical source still lives in Tracker's private header.
The notifier isolates that fact in one implementation file.

The notifier smoke injects a local `BLooper` target. It does not resolve or send
to the running Tracker.

### HaikuWallpaperContract

`HaikuWallpaperContract` captures the public Tracker background contract
without changing the desktop.

Current state:

- resolves `B_DESKTOP_DIRECTORY` and verifies that it is a directory
- uses the public `<be_apps/Tracker/Background.h>` constants
- identifies `B_BACKGROUND_INFO` as the desktop-node attribute
- builds one value-aligned `BMessage` for an image path
- requests scaled placement at origin `(0, 0)`
- enables icon label outline
- targets all workspaces
- writes a prepared message as `B_MESSAGE_TYPE` to a caller-supplied `BNode`
- reads and unflattens that attribute from a caller-supplied `BNode`
- delegates typed raw attribute write, capture, and restore to `BettributeStore`
- verifies the exact five-field wallpaper message without experimental APIs
- replaces, reads back, verifies, and optionally runs one commit action
- rolls back after every post-capture failure
- reports the primary operation and rollback statuses separately
- rejects missing attributes, wrong types, and incomplete I/O explicitly

The contract does not connect its Desktop target lookup to the write seam and
does not send messages. The attribute roundtrip smoke uses an isolated temporary
file, never the Desktop node. Tracker target resolution and restore notification
belong to `TrackerNotifier`.

### DesktopWallpaperTarget

`DesktopWallpaperTarget` owns the real, non-mutating target-resolution seam.

Current state:

- starts with an unset `BNode` and invalid `BMessenger`
- resolves the verified Desktop path through `HaikuWallpaperContract`
- opens that directory as a caller-accessible `BNode`
- resolves the running Tracker through `TrackerNotifier`
- resets both resources before each resolution attempt
- clears the node again if Tracker resolution fails
- reports readiness only when both native resources are valid
- performs no attribute read or write
- sends no Tracker message

The target owns both native objects. A later deliberate action can construct an
injected `WallpaperSetter` from `Node()` and `Messenger()` while the target
remains alive.

The target smoke performs real resolution on Haiku but no mutation.

### WallpaperSetter

`WallpaperSetter` is the wallpaper application interface.

Current state:

- accepts `ProviderResult`
- checks whether an image path exists
- preserves a safe default constructor for the current automatic window path
- accepts an injected caller-owned `BNode` and `BMessenger` for backend work
- builds the public wallpaper message through `HaikuWallpaperContract`
- performs verified replace-or-rollback on the injected node
- notifies only the injected message target after verification
- exposes the last rollback status separately
- returns Haiku `status_t`
- stores translated errors for the existing public preconditions

The injected backend is complete for one node and one notification target.
The default constructor still returns `B_NOT_SUPPORTED` for a valid image path.
`MainWindow` now uses the injected constructor only after an explicit button
message, so application startup and aggregate smoke remain non-mutating.

The window translates the action state and combines backend `status_t` values
with Haiku's status descriptions. A failed rollback remains separately visible.

After a confirmed setter success, `DailyWallpaperAction` stores the applied
image path and local calendar date in `AppSettings`. History persistence is a
secondary post-success operation: a save failure does not misreport the already
completed Desktop change, and the previous in-memory history values are
restored.

After successful history persistence, the window reloads the provider. For the
Local-folder provider this prepares the deterministic next image immediately.
`ReloadProvider()` remains the authority for preview, provider status, and
whether the apply button is enabled; the success path therefore does not
unconditionally re-enable that button afterward.

The window asks `DailyWallpaperPolicy` for the current local ISO date,
delegates the stored-date comparison, and then combines that state with the
loaded provider candidate. `ReloadProvider()` refreshes the readiness only after
the final `ProviderResult` is known. The window only translates the returned
readiness into visible text. This is informative only: it does not disable the
manual apply action and does not schedule work.

## Current data flow

```text
MainWindow
  -> AppSettings
      <-> flattened BMessage settings file
      -> selected/default provider settings
      -> local folder source path
  -> native directory-only BFilePanel
      -> selected entry_ref
      -> persist provider=Local folder and local folder path
      -> reload provider in the same window
  -> ProviderResolver
      -> DemoProvider
      -> LocalFolderProvider(folder path, last image path)
  -> one synchronous provider Fetch()
  -> retain ProviderResult in the window
  -> release provider instance
  -> refresh preview, provider status, and action availability
  -> DailyWallpaperPolicy
      -> current local YYYY-MM-DD
      -> unavailable / pending / applied-today decision
      -> combine with ProviderResult::HasImagePath()
      -> unavailable / already-applied / no-candidate / ready
      -> informational status only
  -> explicit Apply wallpaper button
      -> DesktopWallpaperTarget::Resolve()
      -> WallpaperSetter(real node, real messenger)
      -> DailyWallpaperAction(callbacks)
          -> apply only after the user message
          -> on success obtain YYYY-MM-DD
          -> persist last image path and date
          -> restore prior in-memory history on save failure
      -> refresh informational today/already-applied status
      -> reload provider and prepare the next manual candidate
      -> visible success / history-save failure / operation failure
      -> visible rollback status

DailyWallpaperStartupPlan
  <- DailyWallpaperReadiness
  -> DO_NOTHING / APPLY_ONCE
  -> injected executor called at most once
  -> composes with DailyWallpaperAction in an isolated smoke only
  -> not wired to MainWindow startup

DailyWallpaperAction
  <- AppSettings + ProviderResult + injected callbacks
  -> apply status + history status + rollback status
  -> used by the manual MainWindow action
  -> used through the startup executor only in the isolated smoke

AppSettings
  -> archive preference
  -> last image path and update date

DailyImageProvider
  -> DemoProvider
  -> LocalFolderProvider
      -> Haiku Storage Kit directory enumeration
      -> Haiku Translation Kit image identification
  -> status_t
  -> ProviderResult on B_OK
      -> WallpaperInfo
      -> image path

ProviderResult
  -> DeskbarView tooltip

ProviderResult
  -> HaikuWallpaperContract
      -> public Tracker background BMessage schema
      -> isolated caller-supplied BNode attribute roundtrip
      -> BettributeStore
          -> typed raw write
          -> owned snapshot of data or absence
          -> raw restore
      -> verified replace-or-rollback
      -> Desktop target remains outside the write seam
      -> no Desktop mutation yet
  -> TrackerNotifier
      -> locally verified Tracker signature
      -> injected BMessenger target
      -> public restore message
      -> no real Tracker notification in notifier smoke
  -> DesktopWallpaperTarget
      -> verified Desktop path
      -> real Desktop BNode
      -> real Tracker BMessenger
      -> resolution only; no write or notification
  -> WallpaperSetter
      -> default safe stub for current MainWindow construction
      -> injected BNode + BMessenger backend
          -> HaikuWallpaperContract replace-or-rollback
          -> TrackerNotifier restore message
          -> separate rollback status
      -> status_t / precondition error message
      -> MainWindow status display
```

## Build system

BeMyDailyWall uses Haiku's Generic Makefile / makefile-engine.

This is the default build policy for the project because it is native enough for
a standalone Haiku application while keeping the project small and readable.

Jam is reserved for cases where code is integrated into an existing Jam-based
Haiku project or where matching Haiku source tree conventions is explicitly
required.

## Near-term boundaries

The project currently does not implement:

- network access
- real wallpaper download
- full image decode before local-folder selection
- general provider-selection UI beyond the local-folder action
- automatic daily scheduling
- Deskbar installation
- archive/gallery browsing

Those will be added only after the internal seams are explicit and testable.
