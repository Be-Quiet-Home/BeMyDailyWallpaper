#include "DailyWallpaperPolicy.h"

#include <Errors.h>
#include <String.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall daily wallpaper policy smoke: %s\n",
		message);
	return 1;
}


static bool
IsStableDate(const char* date)
{
	if (date == NULL || strlen(date) != 10)
		return false;

	for (int32 index = 0; index < 10; index++) {
		if (index == 4 || index == 7) {
			if (date[index] != '-')
				return false;
		} else if (!isdigit((unsigned char)date[index])) {
			return false;
		}
	}

	return true;
}


int
main()
{
	if (DailyWallpaperPolicy::Evaluate(
			"2026-07-17", NULL) != DAILY_WALLPAPER_UNAVAILABLE) {
		return Fail("null current date was not unavailable");
	}

	if (DailyWallpaperPolicy::Evaluate(
			"2026-07-17", "") != DAILY_WALLPAPER_UNAVAILABLE) {
		return Fail("empty current date was not unavailable");
	}

	if (DailyWallpaperPolicy::Evaluate(
			NULL, "2026-07-17") != DAILY_WALLPAPER_PENDING) {
		return Fail("null history was not pending");
	}

	if (DailyWallpaperPolicy::Evaluate(
			"", "2026-07-17") != DAILY_WALLPAPER_PENDING) {
		return Fail("empty history was not pending");
	}

	if (DailyWallpaperPolicy::Evaluate(
			"2026-07-16", "2026-07-17") != DAILY_WALLPAPER_PENDING) {
		return Fail("older history was not pending");
	}

	if (DailyWallpaperPolicy::Evaluate(
			"2026-07-18", "2026-07-17") != DAILY_WALLPAPER_PENDING) {
		return Fail("future history was not pending");
	}

	if (DailyWallpaperPolicy::Evaluate(
			"2026-07-17", "2026-07-17")
			!= DAILY_WALLPAPER_APPLIED_TODAY) {
		return Fail("matching dates were not applied today");
	}

	BString currentDate;
	if (DailyWallpaperPolicy::CurrentLocalDate(currentDate) != B_OK)
		return Fail("could not obtain the current local date");

	if (!IsStableDate(currentDate.String()))
		return Fail("current local date did not use YYYY-MM-DD");

	printf("BeMyDailyWall daily wallpaper policy smoke: ok\n");
	return 0;
}
