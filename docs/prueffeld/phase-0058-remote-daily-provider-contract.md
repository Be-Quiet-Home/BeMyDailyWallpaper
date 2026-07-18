# Phase 0058: Remote daily-provider contract

Date: 2026-07-18
Baseline commit: `210089e Define used image history contract`

## Trigger

The local-folder provider received disproportionate roadmap weight even though
it is one provider among several.

The product goal is not perfect image-library management.

```text
rotate to a different provider-supplied image each day
```

## Scope

This phase restores product weighting and defines the remote-provider boundary.

It changes documentation only.

No source, settings schema, Makefile, CatKey, network request, cache file, or
provider mapping changes.

## Product hierarchy

```text
BeMyDailyWall
    -> provider-neutral daily orchestration

DailyImageProvider
    -> source contract

remote daily providers
    -> primary product direction

LocalFolderProvider
    -> offline provider
    -> reference implementation
    -> deterministic smoke foundation

DemoProvider
    -> metadata-only contract probe
```

`UsedImageHistory` remains documented but parked.

## Stable public seam

```cpp
virtual const char* Name() const = 0;
virtual status_t Fetch(ProviderResult& result) = 0;
```

No URL, HTTP response, cache object, or parser type enters the common interface.

## Local artifact invariant

`ProviderResult::ImagePath()` is always empty or one resolved local filesystem
path. It is never a network URL.

A remote provider turns remote source data into a validated local image before
returning `B_OK`.

```text
ProviderResult
    -> DailyWallpaperAction
    -> WallpaperSetter
```

## Remote sequence

```text
fetch metadata
validate one current descriptor
resolve HTTPS asset URL
reuse or populate bounded cache
validate image through Translation Kit
atomically publish cache file
return local ProviderResult
```

No partial download becomes visible.

## Offline rule

A stale cache entry must not be reported as the current remote candidate merely
because the network is unavailable.

Only an exact validated cache hit for the current provider asset may satisfy the
request. Otherwise the provider reports no new candidate and the Desktop remains
unchanged.

## Bing position

Bing is the first concrete source candidate and product inspiration.

It is not a framework dependency.

Current reality:

- Microsoft offers a daily Bing Wallpaper experience with metadata and recent
  navigation
- `HPImageArchive.aspx` is a small public interface for current/recent homepage
  image metadata
- it is not treated as a versioned supported archive API
- market selection affects content
- current Microsoft terms create a distribution/use stop condition before a
  default distributable adapter

The architecture targets a Bing-like product while keeping the source
replaceable.

## Research target

```text
https://www.bing.com/HPImageArchive.aspx?format=js&idx=0&n=1&mkt=de-DE
```

The first implementation must not assume more than one current entry.

## Next gate

The next brick is a Haiku network reality map:

```text
available public Network Kit API
HTTPS and certificate behavior
synchronous versus asynchronous request ownership
response body ownership
redirect behavior
status and cancellation
package/runtime dependencies on Völundr
```

No parser or downloader is selected before that gate.

## Validation

```sh
git diff --check
git status
git diff
```

No build or CatKey generation is required.

## Proves

- local folder is one provider, not the product center
- the public seam remains source-neutral
- remote providers return validated local artifacts
- wallpaper application needs no network knowledge
- Bing-specific instability stays inside one adapter
- cache and offline behavior fail closed
- source-usage constraints are release gates
- no implementation begins before Haiku network reality is known

## Does not prove

- a usable Haiku HTTPS request
- a JSON parser
- a cache implementation
- live Bing compatibility
- permission to distribute a Bing-backed provider
- provider selection UI

Smoke tests prove life, not dignity.
