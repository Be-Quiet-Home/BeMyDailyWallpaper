# Phase 0018: Provider result image path mutation

Date: 2026-07-14
Baseline commit: `0e2ef51 Complete demo image path contract`

## Scope

This phase makes the mutation behavior of `ProviderResult::SetImagePath()`
explicit in the existing provider contract smoke.

It changes no production behavior.

## Mutation contract

A new `ProviderResult` begins with no image path.

After:

```text
SetImagePath("/boot/home/test-wallpaper.jpg")
```

the result must report:

```text
ImagePath() == /boot/home/test-wallpaper.jpg
HasImagePath() == true
```

After:

```text
SetImagePath("")
```

the same result must report:

```text
ImagePath().Length() == 0
HasImagePath() == false
```

The setter replaces the previous value; clearing is therefore a supported state
transition rather than a separate API.

## Test boundary

The path is treated only as a stored string.

This phase does not access the filesystem and does not require the test path to
exist. Validation of readability, file type, normalization, or ownership belongs
to later components.

## Existing provider checks

The same smoke continues to verify:

```text
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

- `SetImagePath()` preserves a supplied non-empty string,
- a non-empty stored path activates `HasImagePath()`,
- setting an empty string clears the previous path,
- clearing deactivates `HasImagePath()`,
- both image-path accessors remain consistent across mutation.

## What this phase does not prove

This phase does not prove that:

- a stored path exists,
- a stored path is absolute or normalized,
- a stored path identifies a supported image,
- `NULL` is a valid setter argument,
- path ownership and lifetime beyond `BString` storage need special handling,
- a real provider exists,
- the application is release-ready.

Smoke tests prove life, not dignity.
