# Phase 0020: Provider result info replacement

Date: 2026-07-14
Baseline commit: `295783c Test provider info copy contract`

## Scope

This phase completes the `ProviderResult::SetInfo()` value contract by proving
that a second call replaces every previously stored metadata field.

It changes no production behavior.

## Replacement contract

The existing copy-contract probe first stores:

```text
title:       Original title
description: Original description.
source:      Original source
attribution: Original attribution.
date:        2026-07-14
```

It then calls `SetInfo()` again with:

```text
title:       Replacement title
description: Replacement description.
source:      Replacement source
attribution: Replacement attribution.
date:        2027-01-01
```

The result must expose all five replacement values.

## Complete replacement

`SetInfo()` replaces one complete `WallpaperInfo` value with another.

The smoke checks each field separately:

```text
Info().Title()
Info().Description()
Info().Source()
Info().Copyright()
Info().Date()
```

This prevents a later partial-update implementation from accidentally retaining
metadata from a previous provider result.

## Provider result contract closure

With this phase, the current `ProviderResult` contract covers:

```text
neutral default state
metadata value copy
metadata source independence
complete metadata replacement
image path set
image path clear
image path presence
```

Further provider work should now move to concrete provider behavior rather than
adding more container-level micro-contracts without a demonstrated need.

## Existing provider checks

The same smoke continues to verify:

```text
successful DemoProvider output
all five demo metadata fields
empty demo image path through both accessors
failing provider status
```

## Build impact

This phase changes only:

```text
tests/provider_contract_smoke.cpp
docs/architecture.md
this Prüffeld document
```

It introduces no production source change, Makefile rule, catalog key, or
visible UI change.

## Validation

```sh
make clean
make
make smoke-provider
make smoke
```

No visual gate or catalog regeneration is required.

## What this phase proves

This phase proves that:

- a second `SetInfo()` call replaces the first value,
- all five metadata fields are replaced together,
- no previous metadata survives a complete replacement,
- existing provider-result contracts remain active.

## What this phase does not prove

This phase does not prove that:

- provider results require additional mutation APIs,
- partial metadata updates are needed,
- metadata values are semantically valid,
- custom move semantics are required,
- a real provider exists,
- the application is release-ready.

Smoke tests prove life, not dignity.
