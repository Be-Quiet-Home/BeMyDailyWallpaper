# Phase 0017: Complete demo image path contract

Date: 2026-07-14
Baseline commit: `2081118 Complete demo metadata contract`

## Scope

This phase completes the successful `DemoProvider::Fetch()` result contract for
the optional image path.

It changes no production behavior.

## Existing demo output

`DemoProvider::Fetch()` explicitly writes:

```text
result.SetImagePath("");
```

The provider smoke already required:

```text
HasImagePath() == false
```

It did not independently verify the stored `BString`.

## New assertions

After `DemoProvider::Fetch()` returns `B_OK`, the provider smoke now requires:

```text
ImagePath().Length() == 0
HasImagePath() == false
```

The accessors are intentionally checked separately.

This proves both:

```text
stored image path is empty
boolean image-path report is false
```

A future implementation change cannot silently make those two views disagree.

## Coverage boundary

The successful demo-provider probe now covers:

```text
status
stable provider name
all five WallpaperInfo fields
raw image path
image path presence
```

The neutral precondition and failing-provider status checks remain unchanged.

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

- the demo provider stores an empty image path,
- the presence accessor agrees with the stored path,
- metadata-only success remains intentional,
- the complete successful demo result is now covered.

## What this phase does not prove

This phase does not prove that:

- successful providers generally may never return image paths,
- all non-empty paths identify readable image files,
- paths are absolute or normalized,
- image ownership and lifetime are settled,
- a real provider exists,
- the application is release-ready.

Smoke tests prove life, not dignity.
