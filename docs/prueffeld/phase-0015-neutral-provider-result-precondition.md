# Phase 0015: Neutral provider result precondition

Date: 2026-07-14
Baseline commit: `1dc4858 Make wallpaper info default neutral`

## Scope

This phase makes the neutral default state of `ProviderResult` an explicit
provider-smoke precondition.

It adds no production behavior.

## Neutral result contract

Before a provider receives a newly constructed `ProviderResult`, the following
must all be true:

```text
Info().Title()       is empty
Info().Description() is empty
Info().Source()      is empty
Info().Copyright()   is empty
Info().Date()        is empty
ImagePath()          is empty
HasImagePath()       is false
```

The provider smoke checks this state independently before:

```text
DemoProvider::Fetch()
FailingProvider::Fetch()
```

This prevents future constructor changes from silently seeding provider output
with stale, demo, or presentation-owned data.

## Failure boundary

This phase does not require a provider to leave `ProviderResult` unchanged after
returning an error.

The current consumer contract remains:

```text
B_OK:
    ProviderResult may be consumed

non-B_OK:
    ProviderResult is not consumed
```

A stronger transactional provider-output contract would require a separate
design decision and proof.

## Existing success contract

After `DemoProvider::Fetch()` returns `B_OK`, the smoke still verifies:

```text
stable provider name
title
description
source
attribution
empty image path
```

The failing provider still verifies the explicit `B_ERROR` status.

## Build impact

This phase changes only:

```text
tests/provider_contract_smoke.cpp
docs/architecture.md
this Prüffeld document
```

It introduces:

```text
no new source file
no new smoke target
no Makefile change
no catalog key
```

## Validation

```sh
make clean
make
make smoke-provider
make smoke
```

No Catkey or catalog regeneration is required.

## What this phase proves

This phase proves that:

- every new provider output object starts neutral,
- both success and failure probes receive the same clean precondition,
- image-path emptiness is visible through both accessors,
- demo metadata enters only during `DemoProvider::Fetch()`,
- the existing provider result contract remains stable.

## What this phase does not prove

This phase does not prove that:

- failed providers preserve or clear partially written output,
- provider output is committed transactionally,
- a real provider exists,
- all metadata combinations are valid,
- an image path exists after every successful fetch,
- the application is release-ready.

Smoke tests prove life, not dignity.
