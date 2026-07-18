# BeMyDailyWallpaper

BeMyDailyWall exists because not every journey starts with money, a suitcase, or a plane ticket.

Sometimes it starts with a small icon in the Deskbar and a picture of somewhere else.

## Product goal

BeMyDailyWall is a Haiku-native daily wallpaper companion.

Its primary job is deliberately small:

```text
ask the selected provider for today's image
make that image locally usable
preserve title and attribution
apply it at most once per local day
```

Providers are interchangeable sources. A remote daily-image provider is the
main product direction. `LocalFolderProvider` remains valuable as an offline
provider, reference implementation, and deterministic smoke-test fixture, but it
is not the product center.

The application is Bing-Wallpaper-like rather than architecturally dependent on
Bing. Source-specific network, metadata, licensing, and cache rules stay behind
their provider boundary.
