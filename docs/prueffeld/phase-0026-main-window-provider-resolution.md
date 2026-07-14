# Phase 0026: MainWindow provider resolution

Date: 2026-07-14
Baseline commit: `15f1fb4 Add settings-based provider resolver`

## Scope

This phase connects `MainWindow` to `ProviderResolver`.

The window no longer constructs `DemoProvider` directly. Persisted provider
identity now reaches the visible application path.

The real wallpaper backend remains unimplemented.

## Startup flow

The window now performs:

```text
load AppSettings
    -> resolve selected provider
    -> fetch ProviderResult
    -> copy stable provider name
    -> delete provider instance
    -> update preview and status
    -> call WallpaperSetter only after B_OK
```

Resolution and fetch share one status gate.

If resolution fails, no `Fetch()` call occurs.

## Default behavior

Default settings remain:

```text
provider_name: Demo provider
local_folder_path: empty
```

The resolver therefore creates `DemoProvider`, and the visible default window
continues to show the same demo metadata and status text as before.

No UI string or layout changes in this phase.

## Local folder behavior

When persisted settings contain:

```text
provider_name: Local folder
local_folder_path: configured path
```

`MainWindow` now resolves and fetches `LocalFolderProvider`.

On success:

```text
Deskbar preview receives local image metadata
provider status reports Local folder loaded
WallpaperSetter receives the local ProviderResult
```

Because the wallpaper backend is still a stub, a result with an image path
continues to produce its existing not-supported setter status.

When the path or inventory fails, the existing provider failure path is used
and the setter is skipped.

## Unknown provider behavior

An unknown persisted provider name causes `ProviderResolver` to return
`B_NAME_NOT_FOUND`.

The window:

```text
does not call Fetch()
keeps the stored provider name in the status line
shows no provider tooltip data
skips WallpaperSetter
```

There is no silent fallback to `DemoProvider`.

## Ownership boundary

`ProviderResolver` returns a caller-owned `DailyImageProvider`.

`MainWindow` retains the instance only through the synchronous `Fetch()` call.
It copies the provider name into a `BString`, deletes the provider, and then
continues with the value-owned `ProviderResult`.

This keeps concrete provider lifetime short and explicit.

## Build impact

Changed production file:

```text
src/MainWindow.cpp
```

Changed build file:

```text
Makefile
```

`ProviderResolver.cpp` is added to the application sources. The resolver smoke
is also declared phony and included in the aggregate smoke dependency chain.

Changed documentation:

```text
docs/architecture.md
this Prüffeld document
```

No new class, linked library, catalog key, or layout change is introduced.

## Validation

```sh
make clean
make
make smoke-provider-resolver
make smoke
```

The aggregate smoke builds and launches the application with its normal
settings path. Existing provider-resolver and local-folder smokes continue to
cover both construction branches independently.

No catalog regeneration is required.

## Visual gate

The default-settings window should remain visually unchanged:

```text
Settings reports Demo provider
provider reports Demo provider loaded
Deskbar preview contains demo tooltip data
WallpaperSetter reports the existing no-image-path error
```

This is a wiring gate, not a layout-review phase.

## What this phase proves

This phase proves that:

- persisted provider identity reaches `MainWindow`,
- concrete provider construction stays inside `ProviderResolver`,
- the default demo path remains intact,
- local-folder settings can enter the visible provider path,
- resolution failure prevents fetch and setter work,
- resolved provider ownership is released after one fetch.

## What this phase does not prove

This phase does not prove that:

- users can edit provider settings in the UI,
- a local folder has been configured on the test machine,
- local images rotate by date,
- `WallpaperSetter` changes the desktop,
- Deskbar installation works,
- the application is release-ready.

Smoke tests prove life, not dignity.
