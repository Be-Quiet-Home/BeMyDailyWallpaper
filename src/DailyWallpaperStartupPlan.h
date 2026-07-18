#ifndef DAILY_WALLPAPER_STARTUP_PLAN_H
#define DAILY_WALLPAPER_STARTUP_PLAN_H

#include "DailyWallpaperPolicy.h"

#include <SupportDefs.h>


enum DailyWallpaperStartupAction {
	DAILY_WALLPAPER_STARTUP_DO_NOTHING = 0,
	DAILY_WALLPAPER_STARTUP_APPLY_ONCE
};


typedef status_t (*DailyWallpaperStartupExecutor)(void* context);


class DailyWallpaperStartupPlan {
public:
	static DailyWallpaperStartupAction Plan(
		DailyWallpaperReadiness readiness,
		bool startupApplyEnabled);
	static status_t Execute(
		DailyWallpaperStartupAction action,
		DailyWallpaperStartupExecutor executor,
		void* context);
};


#endif
