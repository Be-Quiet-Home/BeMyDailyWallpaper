# Phase 0008: Atomic settings replacement

Date: 2026-07-14
Baseline commit: `f27edbe Reject duplicate settings fields`

## Scope

This phase changes settings saving from direct target truncation to a
rename-based replacement flow.

The target file is not opened for writing.

## Save sequence

`AppSettings::SaveTo()` now performs:

```text
build complete BMessage
    -> open <target>.tmp
    -> flatten message
    -> sync temporary file
    -> close temporary file
    -> rename temporary file over target
```

The temporary path is a sibling of the target, so the rename stays on the same
filesystem.

`BEntry::Rename(path, true)` performs the final clobbering replacement.

## Failure boundary

Failures before the rename return their Haiku `status_t`.

When flattening or syncing fails, the temporary file is removed when possible.

The existing target is not erased or truncated before the temporary message is
complete.

A rename failure also triggers best-effort temporary-file cleanup.

## Smoke case: blocked temporary path

The smoke first saves a complete protected settings message.

It then creates a directory at:

```text
<target>.tmp
```

This prevents `BFile` from opening the temporary path as a settings file.

Expected result:

```text
replacement SaveTo() != B_OK
the original target remains readable
all four protected values remain unchanged
```

The directory is then removed.

## Smoke case: completed replacement

The normal round-trip save follows the blocked-path case.

Expected result:

```text
SaveTo() == B_OK
the target contains the new four-field message
<target>.tmp does not exist
```

## What this phase proves

This phase proves that:

- production saving no longer truncates the target directly,
- a complete message is flattened to a sibling temporary file,
- the temporary file is synchronized before rename,
- inability to open the temporary path leaves the old target intact,
- successful replacement leaves no temporary sibling,
- the resulting target still passes the complete settings round trip,
- the existing read-validation smokes remain active.

## What this phase does not prove

This phase does not prove that:

- an actual mid-`Flatten()` disk failure was induced,
- an actual `Sync()` failure was induced,
- an actual rename failure was induced,
- the parent directory is synchronized,
- every filesystem provides identical rename guarantees,
- power loss after rename is fully durable,
- concurrent writers are serialized,
- invalid production settings are quarantined automatically,
- the application is release-ready.

Smoke tests prove life, not dignity.
