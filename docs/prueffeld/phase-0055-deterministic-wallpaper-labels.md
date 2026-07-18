# Phase 0055: Deterministic wallpaper labels

Date: 2026-07-18
Baseline commit: `e0145e3 Extract reusable wallpaper action`

## Observation

After a successful apply, the Desktop shows the image that was just consumed.
`ReloadProvider()` then immediately advances to the next provider candidate.

The previous interface displayed the next candidate in the Deskbar-preview
tooltip without naming that distinction. The tooltip could therefore appear to
contradict the Desktop.

## Scope

This phase makes the two existing image authorities explicit.

It does not change selection, apply, history, or startup behavior.

## Visible state contract

```text
Last applied wallpaper: <filename>
Next wallpaper: <filename>
```

Authority:

```text
Last applied wallpaper
    -> AppSettings::LastImagePath()

Next wallpaper
    -> ProviderResult::ImagePath()
```

The last-applied label describes app-recorded history. It does not claim to
observe wallpaper changes performed outside BeMyDailyWall.

The next label describes exactly the candidate used by the next manual apply.

Both labels expose the corresponding absolute path through a tooltip.

## Empty states

```text
Last applied wallpaper: none recorded.
Next wallpaper: none available.
```

## Action wording

The manual button becomes:

```text
Apply next wallpaper
```

Provider readiness and failure messages also use `Next wallpaper` so the
operation target remains unambiguous.

## Deskbar-preview tooltip

Old header and title wording:

```text
BeMyDailyWall
Today: <title>
```

New wording:

```text
Next wallpaper
Image: <title>
```

This tooltip is about the provider candidate, not about the image already shown
on the Desktop.

## Post-apply transition

```text
before apply
    Last applied wallpaper: A
    Next wallpaper: B

apply next wallpaper
    -> B is applied
    -> B is recorded
    -> provider reloads

after apply
    Last applied wallpaper: B
    Next wallpaper: C
```

The two filenames may legitimately differ.

## Restored and new files

`LocalFolderProvider` still sorts candidates bytewise by filename and remembers
only `LastImagePath()`.

A previously used file that was deleted and later restored is not recognized as
historically used unless it is the immediately recorded last path. Full usage
history is a separate future policy brick.

## Locale impact

New or changed CatKeys include:

```text
Last applied wallpaper: none recorded.
Last applied wallpaper: %image%
Next wallpaper: none available.
Next wallpaper: %image%
Apply next wallpaper
Next wallpaper
Image: %title%
```

The previous generic preview and `Today:` wording disappears.

Run `make catkeys` and commit the regenerated English CatKeys file.

## Validation

```sh
make catkeys
make clean
make
make smoke-wallpaper-info
make smoke
```

## Real gate

Use a local folder with at least two images.

Before apply:

```text
Last applied wallpaper: <previous filename>
Next wallpaper: <candidate filename>
```

Hover the preview:

```text
Next wallpaper
Image: <candidate filename>
```

Click `Apply next wallpaper`.

After apply:

```text
Last applied wallpaper: <consumed candidate>
Next wallpaper: <following candidate>
```

The Desktop must show the last-applied filename, while the tooltip intentionally
shows the next filename.

## Proves

- app-recorded history and provider candidate are visibly distinct
- the button names the state it consumes
- the tooltip names the state it describes
- post-apply advancement is no longer presented as a contradiction
- full paths remain available without cluttering the primary labels
- selection and execution contracts remain unchanged

## Does not prove

- direct inspection of the current Desktop wallpaper
- complete used-image history
- avoidance of restored historical images
- automatic startup application
- retry, midnight refresh, or scheduling

Smoke tests prove life, not dignity.
