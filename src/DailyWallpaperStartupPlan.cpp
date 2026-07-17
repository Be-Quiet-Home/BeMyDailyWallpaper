#include "DailyWallpaperStartupPlan.h"

#include <Errors.h>


DailyWallpaperStartupAction
DailyWallpaperStartupPlan::Plan(DailyWallpaperReadiness readiness)
{
	if (readiness == DAILY_WALLPAPER_READINESS_READY)
		return DAILY_WALLPAPER_STARTUP_APPLY_ONCE;

	return DAILY_WALLPAPER_STARTUP_DO_NOTHING;
}


status_t
DailyWallpaperStartupPlan::Execute(
	DailyWallpaperStartupAction action,
	DailyWallpaperStartupExecutor executor,
	void* context)
{
	switch (action) {
		case DAILY_WALLPAPER_STARTUP_DO_NOTHING:
			return B_OK;

		case DAILY_WALLPAPER_STARTUP_APPLY_ONCE:
			if (executor == 0)
				return B_BAD_VALUE;

			return executor(context);

		default:
			return B_BAD_VALUE;
	}
}
