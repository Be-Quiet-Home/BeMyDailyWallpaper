# Architecture

BeMyDailyWall is a small Haiku-native daily wallpaper companion.

The application is intentionally split into small responsibilities:

- App: startup, settings, lifecycle
- Deskbar item: icon, tooltip, context menu
- Worker: daily update check and download
- Wallpaper setter: applies the selected image as desktop background
- Archive: stores downloaded images and metadata
- Providers: fetch daily image metadata and image files

The first implementation may use existing Haiku tools such as bgswitch where useful.
Native Haiku APIs should be preferred whenever they are practical and stable.
