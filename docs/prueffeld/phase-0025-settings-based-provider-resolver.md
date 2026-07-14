# Phase 0025: Settings-based provider resolver

Date: 2026-07-14
Baseline commit: `f2eff8d Persist local folder path`

## Scope

This phase adds `ProviderResolver`, the construction seam between persisted
settings and concrete provider implementations.

It does not change `MainWindow` provider selection and does not fetch or apply a
wallpaper.

## Mapping contract

The resolver recognizes the stable provider names already used by settings:

```text
Demo provider
Local folder
```

Mappings:

```text
Demo provider
    -> new DemoProvider

Local folder
    -> new LocalFolderProvider(settings.LocalFolderPath())
```

An unrecognized provider name returns:

```text
B_NAME_NOT_FOUND
```

No fallback to the demo provider is performed. Unknown persisted identity stays
visible as a configuration error.

## Ownership contract

API:

```cpp
status_t ProviderResolver::Create(
    const AppSettings& settings,
    DailyImageProvider*& provider);
```

The output pointer must be `NULL`.

On success:

```text
status:   B_OK
provider: caller-owned object
```

The caller must delete the provider.

An already occupied output pointer returns `B_BAD_VALUE` and is left unchanged.
This prevents the resolver from silently overwriting and leaking caller-owned
state.

Allocation failure returns `B_NO_MEMORY`.

## Responsibility boundary

`ProviderResolver` owns only construction and mapping.

It does not:

- load settings,
- save settings,
- call `Fetch()`,
- validate local image files,
- apply the wallpaper,
- format UI status text,
- retain the created provider.

Provider behavior remains inside each provider class.

## Resolver smoke

New target:

```sh
make smoke-provider-resolver
```

The smoke verifies:

- default settings create `DemoProvider`,
- the resolved demo provider fetches successfully,
- occupied output is rejected without mutation,
- `Local folder` creates a local provider,
- `LocalFolderPath()` reaches the provider constructor,
- unknown provider identity returns `B_NAME_NOT_FOUND`,
- unknown identity creates no object.

Path forwarding is proven with a temporary regular file. The resolved local
provider must return `B_NOT_A_DIRECTORY` when fetching that configured path.

The aggregate `make smoke` includes the resolver smoke.

## Build impact

New production files:

```text
src/ProviderResolver.h
src/ProviderResolver.cpp
```

New smoke:

```text
tests/provider_resolver_smoke.cpp
```

The application build compiles the resolver, but `MainWindow` still constructs
`DemoProvider` directly.

## Catalog impact

This phase adds no user-facing strings and no Locale Kit keys.

No Catkey or catalog regeneration is required.

## Validation

```sh
make clean
make
make smoke-provider-resolver
make smoke
```

No visual gate is required because visible application behavior is unchanged.

## What this phase proves

This phase proves that:

- persisted provider identity has one construction owner,
- default settings resolve to the existing demo provider,
- local folder settings reach `LocalFolderProvider`,
- unknown identities are not silently accepted,
- provider ownership remains explicit,
- future application wiring need not know concrete provider classes.

## What this phase does not prove

This phase does not prove that:

- `MainWindow` uses the resolver,
- provider selection can be changed through the UI,
- a stored local folder path is valid,
- a resolved provider fetch succeeds in every environment,
- the desktop wallpaper can be changed,
- the application is release-ready.

Smoke tests prove life, not dignity.
