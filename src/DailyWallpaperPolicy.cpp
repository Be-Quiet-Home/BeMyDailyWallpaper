#include "DailyWallpaperPolicy.h"

#include <Errors.h>

#include <string.h>
#include <time.h>


status_t
DailyWallpaperPolicy::CurrentLocalDate(BString& date)
{
	date = "";

	time_t now = time(NULL);
	if (now == (time_t)-1)
		return B_ERROR;

	struct tm localTime;
	if (localtime_r(&now, &localTime) == NULL)
		return B_ERROR;

	char buffer[11];
	if (strftime(buffer, sizeof(buffer), "%Y-%m-%d", &localTime) != 10)
		return B_ERROR;

	date = buffer;
	return B_OK;
}


DailyWallpaperState
DailyWallpaperPolicy::Evaluate(const char* lastUpdateDate,
	const char* currentDate)
{
	if (currentDate == NULL || currentDate[0] == '\0')
		return DAILY_WALLPAPER_UNAVAILABLE;

	if (lastUpdateDate != NULL
		&& strcmp(lastUpdateDate, currentDate) == 0) {
		return DAILY_WALLPAPER_APPLIED_TODAY;
	}

	return DAILY_WALLPAPER_PENDING;
}
