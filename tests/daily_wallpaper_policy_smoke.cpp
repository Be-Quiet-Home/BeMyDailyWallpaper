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

	if (DailyWallpaperPolicy::EvaluateReadiness(
			DAILY_WALLPAPER_UNAVAILABLE, false)
			!= DAILY_WALLPAPER_READINESS_UNAVAILABLE) {
		return Fail("unavailable date without candidate was not unavailable");
	}

	if (DailyWallpaperPolicy::EvaluateReadiness(
			DAILY_WALLPAPER_UNAVAILABLE, true)
			!= DAILY_WALLPAPER_READINESS_UNAVAILABLE) {
		return Fail("unavailable date with candidate was not unavailable");
	}

	if (DailyWallpaperPolicy::EvaluateReadiness(
			DAILY_WALLPAPER_APPLIED_TODAY, false)
			!= DAILY_WALLPAPER_READINESS_APPLIED_TODAY) {
		return Fail("applied state without candidate was changed");
	}

	if (DailyWallpaperPolicy::EvaluateReadiness(
			DAILY_WALLPAPER_APPLIED_TODAY, true)
			!= DAILY_WALLPAPER_READINESS_APPLIED_TODAY) {
		return Fail("applied state with candidate was changed");
	}

	if (DailyWallpaperPolicy::EvaluateReadiness(
			DAILY_WALLPAPER_PENDING, false)
			!= DAILY_WALLPAPER_READINESS_NO_CANDIDATE) {
		return Fail("pending state without candidate was not blocked");
	}

	if (DailyWallpaperPolicy::EvaluateReadiness(
			DAILY_WALLPAPER_PENDING, true)
			!= DAILY_WALLPAPER_READINESS_READY) {
		return Fail("pending state with candidate was not ready");
	}

	if (DailyWallpaperPolicy::EvaluateReadiness(
			(DailyWallpaperState)99, true)
			!= DAILY_WALLPAPER_READINESS_UNAVAILABLE) {
		return Fail("unknown daily state was not unavailable");
	}

	BString currentDate;
	if (DailyWallpaperPolicy::CurrentLocalDate(currentDate) != B_OK)
		return Fail("could not obtain the current local date");

	if (!IsStableDate(currentDate.String()))
		return Fail("current local date did not use YYYY-MM-DD");

	printf("BeMyDailyWall daily wallpaper policy smoke: ok\n");
	return 0;
}
