# Phase 0057: Used-image history contract

Date: 2026-07-18
Baseline commit: `d299588 Apply wallpaper automatically at startup`

## Trigger

A local-folder test deleted several files, restored one previously used file,
and added one new file.

Because current selection remembers only `LastImagePath`, the restored file
could become the first sorted candidate and be applied again.

This is valid under the current immediate-successor contract, but it reveals the
need for an explicit used-image-history authority.

## Scope

This phase defines that authority.

It changes documentation only.

No code, settings schema, provider constructor, rotation rule, CatKey, or
Makefile changes.

## Name

```text
UsedImageHistory
```

The name describes successful application history. It does not imply filesystem
observation, image-content recognition, or permanent archival.

## Identity contract

One identity is the provider-resolved absolute path.

Equality is exact bytewise string equality.

```text
delete /boot/home/Desktop/A.jpg
restore /boot/home/Desktop/A.jpg
    -> same identity

rename A.jpg to B.jpg
    -> new identity

replace the bytes inside A.jpg
    -> same identity
```

Why path identity:

- the provider already owns resolved absolute paths
- settings already persist `LastImagePath` as a path
- no content hashing cost or algorithm migration is introduced
- no BFS node identity survives deletion and restoration
- behavior is deterministic and explainable

## History contract

History is:

```text
ordered oldest -> newest
unique by exact path
cycle-scoped
recorded only after successful apply and settings save
```

A failed apply records nothing.

A failed history save restores the previous in-memory history together with the
existing `LastImagePath` and `LastUpdateDate` rollback.

## Candidate contract

Future selection:

```text
collect valid current images
sort bytewise by filename
select the first path absent from UsedImageHistory
```

Filesystem enumeration order has no authority.

## Cycle reset

When every current valid candidate is already in history:

```text
start a new cycle
retain LastImagePath as immediate-repeat guard

more than one candidate
    -> choose the first sorted path different from LastImagePath

exactly one candidate
    -> allow that candidate
```

The active cycle therefore avoids repeats while unused current candidates
exist, but it does not promise permanent lifetime exclusion.

## Missing and restored files

A history entry is not removed merely because its file is currently absent.

Restoring the same path during the active cycle therefore does not make that
image unused again.

A path may become eligible again only after a cycle reset or deterministic
history overflow rule.

## Planned native persistence

Proposed field:

```text
used_image_path : repeated B_STRING_TYPE
```

Value order preserves history order.

Legacy settings without the field mean empty history.

This is a proposal boundary only. No field is written or read in this phase.

## Resource authority

A repeated path list must not be unbounded.

Before implementation, a separate decision must define:

```text
maximum history entries
overflow behavior
interaction between overflow and cycle reset
validation of empty and duplicate fields
```

This is a hard stop condition, not deferred cleanup.

## Current behavior retained

Current code still:

```text
passes only LastImagePath to LocalFolderProvider
selects the successor when that path exists
falls back to the first sorted candidate when it does not
wraps after the final candidate
```

Manual and automatic apply behavior remain unchanged.

## Validation

```sh
git diff --check
git status
```

No build or CatKey generation is required because no source or locale input
changes.

## Proves

- restored-file behavior has an explicit future authority
- image identity is deterministic and explainable
- history belongs to successful persisted application, not provider discovery
- rotation cycles avoid permanent exclusion
- immediate repeat protection survives cycle reset
- absent paths remain meaningful during a cycle
- resource growth is blocked until bounded policy is selected
- no runtime behavior changes prematurely

## Does not prove

- a maximum history size
- settings round-trip for repeated paths
- candidate selection using history
- UI history presentation
- restored-image avoidance in the current binary
- content identity across rename or move

Smoke tests prove life, not dignity.
