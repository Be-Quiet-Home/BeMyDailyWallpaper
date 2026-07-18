# Providers

## Product role

A provider answers one question:

```text
What locally usable image represents this provider's current daily candidate?
```

BeMyDailyWall does not require every provider to use the same discovery method.

```text
remote daily source
    -> fetch metadata
    -> download and cache one image

local folder
    -> enumerate and select one existing image

demo
    -> return metadata without an image
```

The shared boundary is `ProviderResult`, not transport or selection machinery.

## Provider weighting

```text
remote daily-image providers
    -> primary product direction

LocalFolderProvider
    -> offline provider
    -> reference implementation
    -> deterministic smoke foundation

DemoProvider
    -> metadata-only contract probe
```

Local-folder rotation refinements are useful but do not block the primary
remote-provider path.

## Current provider interface

```cpp
class DailyImageProvider {
public:
    virtual ~DailyImageProvider();

    virtual const char* Name() const = 0;
    virtual status_t Fetch(ProviderResult& result) = 0;
};
```

The interface remains unchanged in this phase.

`B_OK` means the provider produced a valid result. Any other Haiku status means
the caller must not consume or forward the result.

## ProviderResult authority

A successful result may contain:

```text
WallpaperInfo
    title
    description
    source
    copyright / attribution
    provider display date

ImagePath
    resolved absolute local path
```

`ImagePath` may be empty for a metadata-only provider. When present, it must
never contain `http:`, `https:`, or another remote locator.

```text
provider
    -> makes the candidate local

WallpaperSetter
    -> applies a local image to Haiku
```

The setter therefore needs no network knowledge.

## Remote daily-provider contract

A remote provider owns this internal sequence:

```text
1. determine market / locale input
2. request one current metadata document
3. validate exactly one daily entry
4. preserve provider day, title, attribution, and information link
5. resolve one HTTPS image URL
6. reuse a validated cache entry or download to a temporary file
7. validate the completed file through Haiku's Translation Kit
8. atomically publish the cache file
9. return ProviderResult with the final local path
```

A provider must not expose a partial download.

Metadata success without a valid image asset is not a complete wallpaper
candidate.

## Planned remote descriptor

Before `ProviderResult`, a remote provider needs an internal descriptor with
source-specific values equivalent to:

```text
provider identity
market
provider day
title
description
attribution
information URL
image URL
stable remote asset identity
```

The descriptor stays provider-internal and does not expand
`DailyImageProvider`.

## Daily identity

The provider-supplied day is authority for source metadata and cache identity.

The application's local date remains authority for once-per-day application.

```text
provider day
    -> which remote asset was published

local day
    -> whether BeMyDailyWall already performed today's action
```

A market may publish a new asset at a time that does not match local midnight.

## Cache contract

The first cache must be provider-owned and bounded.

```text
validated exact asset already cached
    -> reuse without downloading

download required
    -> write sibling temporary file
    -> synchronize
    -> validate as image
    -> rename to final cache path

metadata or download failure
    -> do not expose temporary data
    -> do not replace a valid cache entry
```

A stale image must not masquerade as today's provider candidate.

If the exact current provider asset is unavailable and the network fails, the
provider reports no new candidate. The current Desktop wallpaper remains
untouched.

Cache size, eviction count, and filename format require a separate policy brick.

## Failure boundary

The initial remote provider must distinguish internally between:

```text
network unavailable
HTTP response failure
metadata parse failure
missing required field
unsupported or unsafe URL
download failure
invalid image data
cache publication failure
```

Several internal reasons may initially map onto existing Haiku status values,
but `B_OK` must never accompany an unusable image path.

No automatic retry, timer, or background worker belongs to this provider
contract.

## Bing reality map

Bing is the first concrete remote-source candidate and product inspiration, not
an architectural dependency.

Microsoft's Bing Wallpaper product confirms the intended experience:

```text
a new image each day
image information
manual next / previous navigation
recent-image browsing
market-dependent availability
```

Candidate German-market metadata request:

```text
https://www.bing.com/HPImageArchive.aspx?format=js&idx=0&n=1&mkt=de-DE
```

Commonly observed entry fields include:

```text
startdate
fullstartdate
enddate
url
urlbase
copyright
copyrightlink
title
```

The endpoint is not treated as a versioned stable API contract. Microsoft Q&A
describes `HPImageArchive.aspx` as a public interface and states that one request
supports only a small recent range, with `n` from 1 through 8. It does not offer
a supported complete historical archive.

Consequences:

- request only the smallest current entry needed by the product
- parse required fields defensively
- reject empty or duplicate current entries
- do not build archive synchronization around the feed
- keep endpoint and field mapping inside one provider
- preserve the market parameter explicitly
- expect source behavior to change independently

## Bing usage stop condition

Microsoft's current Services Agreement describes Bing and MSN materials as
personal, non-commercial material and restricts downloading, copying,
redistribution, and using those materials to build products unless separately
authorized or otherwise lawful.

Therefore:

- Bing images must not be bundled in the repository or package
- downloaded assets remain user-local cache data
- attribution must be preserved
- the adapter must remain replaceable
- a distributable default Microsoft daily-image provider requires a documented
  source-usage decision before release

This is an engineering stop condition, not legal advice.

The provider name and UI must not imply Microsoft or Bing affiliation. No
third-party logo becomes BeMyDailyWall branding.

## Alternative providers

The provider-neutral design must allow sources with clearer machine-use and
redistribution terms.

A future source does not need Bing-shaped fields. It only needs to produce the
common local `ProviderResult`.

Examples already worth separate later gates include NASA APOD, which may return
non-image media and therefore needs its own media-type rejection.

## Implementation order

```text
1. Haiku network capability reality map
2. fixed metadata fixture and parser contract
3. provider-owned bounded cache contract
4. isolated download/cache smoke with injected transport
5. live source probe
6. ProviderResolver mapping
7. product UI selection
```

No remote provider code is added until step 1 identifies the native Haiku
transport and its TLS/runtime behavior.

## Research references

Checked 2026-07-18:

- https://www.microsoft.com/de-de/bing/features/bing-wallpaper/
- https://learn.microsoft.com/en-us/answers/questions/1473961/bing-hpimagearchive-aspx-question
- https://www.microsoft.com/de-de/servicesagreement
