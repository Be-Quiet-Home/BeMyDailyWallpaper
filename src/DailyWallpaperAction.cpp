#include "DailyWallpaperAction.h"

#include "AppSettings.h"
#include "ProviderResult.h"

#include <Errors.h>


DailyWallpaperActionResult
DailyWallpaperAction::Execute(
	AppSettings& settings,
	const ProviderResult& result,
	const DailyWallpaperActionCallbacks& callbacks)
{
	DailyWallpaperActionResult actionResult = {
		B_NO_INIT,
		B_NO_INIT,
		B_NO_INIT
	};

	if (!result.HasImagePath()
		|| callbacks.apply == 0
		|| callbacks.currentDate == 0
		|| callbacks.save == 0) {
		actionResult.applyStatus = B_BAD_VALUE;
		return actionResult;
	}

	actionResult.applyStatus = callbacks.apply(
		callbacks.context, result, actionResult.rollbackStatus);
	if (actionResult.applyStatus != B_OK)
		return actionResult;

	BString updateDate;
	actionResult.historyStatus = callbacks.currentDate(
		callbacks.context, updateDate);
	if (actionResult.historyStatus != B_OK)
		return actionResult;

	if (updateDate.IsEmpty()) {
		actionResult.historyStatus = B_BAD_VALUE;
		return actionResult;
	}

	BString previousImagePath(settings.LastImagePath());
	BString previousUpdateDate(settings.LastUpdateDate());

	settings.SetLastImagePath(result.ImagePath().String());
	settings.SetLastUpdateDate(updateDate.String());

	actionResult.historyStatus = callbacks.save(
		callbacks.context, settings);
	if (actionResult.historyStatus != B_OK) {
		settings.SetLastImagePath(previousImagePath.String());
		settings.SetLastUpdateDate(previousUpdateDate.String());
	}

	return actionResult;
}
