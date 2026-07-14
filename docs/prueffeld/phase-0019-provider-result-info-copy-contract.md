# Phase 0019: Provider result info copy contract

Date: 2026-07-14
Baseline commit: `7d63a93 Test provider image path mutation`

## Scope

This phase makes the copy behavior of `ProviderResult::SetInfo()` explicit in
the existing provider contract smoke.

It changes no production behavior.

## Copy contract

A fully populated source object is created:

```text
title:       Original title
description: Original description.
source:      Original source
attribution: Original attribution.
date:        2026-07-14
```

After:

```text
infoResult.SetInfo(sourceInfo)
```

the source object is replaced with completely different metadata.

The result must still expose every original value through:

```text
Info().Title()
Info().Description()
Info().Source()
Info().Copyright()
Info().Date()
```

This proves that `ProviderResult` owns a value copy rather than observing the
later state of the caller's object.

## Ownership boundary

`SetInfo()` accepts a `const WallpaperInfo&` but stores its own
`WallpaperInfo`.

The caller may therefore reuse, replace, or destroy the original value after
the call without changing the result.

This phase tests value independence. It does not introduce custom copy or move
operators.

## Existing provider checks

The same smoke continues to verify:

```text
image path mutation
neutral ProviderResult precondition
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

- `SetInfo()` transfers all five metadata values,
- `ProviderResult` retains those values after the source is replaced,
- provider results do not alias caller-owned metadata state,
- existing provider and image-path contracts remain active.

## What this phase does not prove

This phase does not prove that:

- metadata values are semantically valid,
- copy allocation failures are recoverable,
- custom move semantics are required,
- `WallpaperInfo` should become immutable,
- a real provider exists,
- the application is release-ready.

Smoke tests prove life, not dignity.
