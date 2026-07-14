#include "WallpaperInfo.h"

#include <stdio.h>


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall wallpaper info smoke: %s\n", message);
	return 1;
}


int
main()
{
	WallpaperInfo complete(
		"Aurora",
		"Northern lights.",
		"Test source",
		"Test attribution.",
		"2026-07-14");

	const char* expectedComplete
		= "BeMyDailyWall\n"
		  "Today: Aurora\n"
		  "Northern lights.\n"
		  "Source: Test source\n"
		  "Test attribution.\n"
		  "Date: 2026-07-14";

	if (complete.TooltipText().Compare(expectedComplete) != 0)
		return Fail("complete tooltip text is unexpected");

	WallpaperInfo titleOnly("Aurora", "", "", "", "");
	if (titleOnly.TooltipText().Compare(
		"BeMyDailyWall\nToday: Aurora") != 0) {
		return Fail("empty optional fields were not omitted");
	}

	WallpaperInfo empty("", "", "", "", "");
	if (empty.TooltipText().Compare("BeMyDailyWall") != 0)
		return Fail("empty metadata changed the tooltip header");

	printf("BeMyDailyWall wallpaper info smoke: ok\n");
	return 0;
}
