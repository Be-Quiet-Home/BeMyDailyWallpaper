# Phase 0036: Local-folder configuration action

Date: 2026-07-17
Baseline commit: `9eb5e70 Add explicit wallpaper apply action`

## Scope

This phase makes the already implemented `LocalFolderProvider` reachable from
the native window.

It adds one directory-selection action, persists the selected path, reloads the
provider immediately, and updates the existing preview and apply controls.

It does not add a generic provider framework or scheduling.

## Native folder panel

`MainWindow` owns one `BFilePanel` configured as:

```text
mode: B_OPEN_PANEL
node flavor: B_DIRECTORY_NODE
multiple selection: false
target: MainWindow
selection message: 'FdSl'
hide when done: true
```

The panel's default button is labeled:

```text
Select folder
```

The window action is labeled:

```text
Choose folder…
```

## Settings ownership

`MainWindow` now retains its `AppSettings` object.

After a valid directory selection:

```text
provider_name = Local folder
local_folder_path = selected absolute path
Save()
```

If `Save()` fails, the previous in-memory provider name and folder path are
restored and the error is shown. No provider reload occurs.

## Provider reload

`ReloadProvider()` performs:

```text
reset ProviderResult to neutral state
resolve provider from retained settings
fetch synchronously
copy result into window-owned ProviderResult
delete provider
update Deskbar preview
update provider status
update wallpaper action status
enable Apply wallpaper only with a valid image path
```

The same path is used during construction and after folder selection.

No application restart is required.

## Folder display

The window displays the selected directory's leaf name:

```text
Wallpaper folder: <folder>
```

The full absolute path is available as the label tooltip.

With no configured directory:

```text
Wallpaper folder: not selected.
```

Using the leaf name avoids expanding the window to the length of an arbitrary
absolute path.

## Apply interaction

While a wallpaper operation is running synchronously, both:

```text
Choose folder…
Apply wallpaper
```

are disabled.

They are restored after success or failure.

## Locale impact

New `MainWindow` keys cover:

```text
Choose folder…
Choose wallpaper folder
Select folder
Wallpaper folder: not selected.
Wallpaper folder: %folder%
Folder selection failed: %error%
Settings save failed: %error%
```

Run:

```sh
make catkeys
```

and commit the regenerated English CatKeys file.

## Automated validation

```sh
make catkeys
make clean
make
make smoke
```

The aggregate smoke does not open the folder panel or post the apply action.

## Visual and functional gate

Start the application.

With default settings:

```text
Wallpaper folder: not selected.
Choose folder… is enabled
Apply wallpaper is disabled
```

Choose a directory containing at least one valid `.jpg`, `.jpeg`, or `.png`
recognized by Haiku's Translation Kit.

Expected immediately after selection:

```text
Settings: loaded, provider=Local folder, ...
Wallpaper folder: <selected leaf>
Provider: Local folder loaded.
Wallpaper: ready to apply.
Apply wallpaper is enabled
```

Hovering the folder label must show the full path.

Clicking `Apply wallpaper` is the existing deliberate real-mutation gate.

Close and reopen the application. The same local folder and provider must load
from settings without another selection.

## Proves

- the local-folder provider is reachable from native UI
- directory selection is restricted to one directory
- the selected absolute path is persisted
- persistence failure preserves previous in-memory settings
- the provider reloads without an application restart
- preview and action availability follow the reloaded result
- long absolute paths do not determine preferred window width
- application startup remains non-mutating

## Does not prove

- automatic daily rotation
- general provider selection
- archive/gallery behavior
- Deskbar installation
- release packaging

Smoke tests prove life, not dignity.
