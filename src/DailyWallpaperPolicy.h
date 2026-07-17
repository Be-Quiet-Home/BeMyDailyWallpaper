#ifndef DAILY_WALLPAPER_POLICY_H
#define DAILY_WALLPAPER_POLICY_H

#include <String.h>
#include <SupportDefs.h>


enum DailyWallpaperState {
	DAILY_WALLPAPER_UNAVAILABLE = 0,
	DAILY_WALLPAPER_PENDING,
	DAILY_WALLPAPER_APPLIED_TODAY
};


enum DailyWallpaperReadiness {
	DAILY_WALLPAPER_READINESS_UNAVAILABLE = 0,
	DAILY_WALLPAPER_READINESS_APPLIED_TODAY,
	DAILY_WALLPAPER_READINESS_NO_CANDIDATE,
	DAILY_WALLPAPER_READINESS_READY
};


class DailyWallpaperPolicy {
public:
	static status_t CurrentLocalDate(BString& date);
	static DailyWallpaperState Evaluate(
		const char* lastUpdateDate, const char* currentDate);
	static DailyWallpaperReadiness EvaluateReadiness(
		DailyWallpaperState state, bool hasCandidate);
};


#endif
