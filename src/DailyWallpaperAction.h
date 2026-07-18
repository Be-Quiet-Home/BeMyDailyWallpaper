#ifndef DAILY_WALLPAPER_ACTION_H
#define DAILY_WALLPAPER_ACTION_H

#include <String.h>
#include <SupportDefs.h>


class AppSettings;
class ProviderResult;


typedef status_t (*DailyWallpaperApplyCallback)(
	void* context,
	const ProviderResult& result,
	status_t& rollbackStatus);

typedef status_t (*DailyWallpaperDateCallback)(
	void* context,
	BString& date);

typedef status_t (*DailyWallpaperSaveCallback)(
	void* context,
	const AppSettings& settings);


struct DailyWallpaperActionCallbacks {
	DailyWallpaperApplyCallback apply;
	DailyWallpaperDateCallback currentDate;
	DailyWallpaperSaveCallback save;
	void* context;
};


struct DailyWallpaperActionResult {
	status_t applyStatus;
	status_t historyStatus;
	status_t rollbackStatus;
};


class DailyWallpaperAction {
public:
	static DailyWallpaperActionResult Execute(
		AppSettings& settings,
		const ProviderResult& result,
		const DailyWallpaperActionCallbacks& callbacks);
};


#endif
