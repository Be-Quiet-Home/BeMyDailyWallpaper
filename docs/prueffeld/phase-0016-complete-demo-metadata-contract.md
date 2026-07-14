# Phase 0016: Complete demo metadata contract

Date: 2026-07-14
Baseline commit: `a3e982e Test neutral provider result state`

## Scope

This phase completes the successful `DemoProvider::Fetch()` metadata contract by
checking its fifth `WallpaperInfo` field: the date.

It changes no production behavior.

## Existing demo output

After a successful fetch, `DemoProvider` supplies:

```text
title:       Somewhere else
description: Your daily window to somewhere else.
source:      Demo provider
attribution: Not an affiliated provider.
date:        empty
image path:  empty
```

The provider smoke already checked every item except the date.

## New assertion

After `DemoProvider::Fetch()` returns `B_OK`, the provider smoke now requires:

```text
Info().Date().Length() == 0
```

This is an explicit field contract, not an incidental consequence of neutral
construction. A later change that gives the demo provider a date must therefore
update both implementation and test deliberately.

## Coverage boundary

The successful demo-provider probe now covers all five metadata fields:

```text
Title()
Description()
Source()
Copyright()
Date()
```

It also continues to verify:

```text
stable provider name
empty image path
B_OK status
```

The neutral precondition and failing-provider status checks remain unchanged.

## Build impact

This phase changes only:

```text
tests/provider_contract_smoke.cpp
docs/architecture.md
this Prüffeld document
```

It introduces no source component, Makefile rule, catalog key, or visible UI
change.

## Validation

```sh
make clean
make
make smoke-provider
make smoke
```

No visual gate is required because production behavior is unchanged.

## What this phase proves

This phase proves that:

- every current `WallpaperInfo` field is covered after a successful demo fetch,
- the empty demo date is intentional,
- future demo-date changes cannot silently alter the provider contract,
- existing provider precondition and failure checks remain active.

## What this phase does not prove

This phase does not prove that:

- real provider dates are parsed or locale-formatted,
- a date is mandatory for successful providers,
- an image path is mandatory for successful providers,
- all provider metadata is valid for every source,
- a real provider exists,
- the application is release-ready.

Smoke tests prove life, not dignity.
