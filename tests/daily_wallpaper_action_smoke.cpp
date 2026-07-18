#include "DailyWallpaperAction.h"

#include "AppSettings.h"
#include "ProviderResult.h"

#include <Errors.h>
#include <String.h>
#include <SupportDefs.h>

#include <stdio.h>


struct ActionProbe {
	int32 applyCalls;
	int32 dateCalls;
	int32 saveCalls;
	status_t applyStatus;
	status_t dateStatus;
	status_t saveStatus;
	status_t rollbackStatus;
	BString date;
	BString savedImagePath;
	BString savedUpdateDate;
};


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall daily wallpaper action smoke: %s\n",
		message);
	return 1;
}


static status_t
ApplyProbe(void* context, const ProviderResult&, status_t& rollbackStatus)
{
	if (context == 0)
		return B_BAD_VALUE;

	ActionProbe* probe = static_cast<ActionProbe*>(context);
	probe->applyCalls++;
	rollbackStatus = probe->rollbackStatus;
	return probe->applyStatus;
}


static status_t
DateProbe(void* context, BString& date)
{
	if (context == 0)
		return B_BAD_VALUE;

	ActionProbe* probe = static_cast<ActionProbe*>(context);
	probe->dateCalls++;
	if (probe->dateStatus == B_OK)
		date = probe->date.String();

	return probe->dateStatus;
}


static status_t
SaveProbe(void* context, const AppSettings& settings)
{
	if (context == 0)
		return B_BAD_VALUE;

	ActionProbe* probe = static_cast<ActionProbe*>(context);
	probe->saveCalls++;
	probe->savedImagePath = settings.LastImagePath();
	probe->savedUpdateDate = settings.LastUpdateDate();
	return probe->saveStatus;
}


static ActionProbe
NewProbe()
{
	ActionProbe probe = {
		0,
		0,
		0,
		B_OK,
		B_OK,
		B_OK,
		B_NO_INIT,
		"2026-07-17",
		"",
		""
	};
	return probe;
}


static DailyWallpaperActionCallbacks
CallbacksFor(ActionProbe& probe)
{
	DailyWallpaperActionCallbacks callbacks = {
		ApplyProbe,
		DateProbe,
		SaveProbe,
		&probe
	};
	return callbacks;
}


static ProviderResult
Candidate()
{
	ProviderResult result;
	result.SetImagePath("/boot/home/wallpapers/next.png");
	return result;
}


static AppSettings
PreviousSettings()
{
	AppSettings settings;
	settings.SetLastImagePath("/boot/home/wallpapers/previous.png");
	settings.SetLastUpdateDate("2026-07-16");
	return settings;
}


static bool
HistoryWasPreserved(const AppSettings& settings)
{
	return settings.LastImagePath().Compare(
			"/boot/home/wallpapers/previous.png") == 0
		&& settings.LastUpdateDate().Compare("2026-07-16") == 0;
}


int
main()
{
	ProviderResult emptyResult;
	AppSettings emptySettings = PreviousSettings();
	ActionProbe emptyProbe = NewProbe();
	DailyWallpaperActionResult emptyAction
		= DailyWallpaperAction::Execute(
			emptySettings, emptyResult, CallbacksFor(emptyProbe));

	if (emptyAction.applyStatus != B_BAD_VALUE)
		return Fail("empty candidate did not return B_BAD_VALUE");
	if (emptyAction.historyStatus != B_NO_INIT)
		return Fail("empty candidate unexpectedly attempted history");
	if (emptyProbe.applyCalls != 0
		|| emptyProbe.dateCalls != 0
		|| emptyProbe.saveCalls != 0) {
		return Fail("empty candidate invoked a callback");
	}
	if (!HistoryWasPreserved(emptySettings))
		return Fail("empty candidate changed history");

	ProviderResult candidate = Candidate();
	AppSettings missingCallbackSettings = PreviousSettings();
	ActionProbe missingCallbackProbe = NewProbe();
	DailyWallpaperActionCallbacks missingCallbacks
		= CallbacksFor(missingCallbackProbe);
	missingCallbacks.save = 0;

	DailyWallpaperActionResult missingCallbackAction
		= DailyWallpaperAction::Execute(
			missingCallbackSettings, candidate, missingCallbacks);
	if (missingCallbackAction.applyStatus != B_BAD_VALUE)
		return Fail("missing callback did not return B_BAD_VALUE");
	if (missingCallbackProbe.applyCalls != 0)
		return Fail("missing callback allowed wallpaper application");
	if (!HistoryWasPreserved(missingCallbackSettings))
		return Fail("missing callback changed history");

	AppSettings applyFailureSettings = PreviousSettings();
	ActionProbe applyFailureProbe = NewProbe();
	applyFailureProbe.applyStatus = B_ERROR;
	applyFailureProbe.rollbackStatus = B_IO_ERROR;

	DailyWallpaperActionResult applyFailureAction
		= DailyWallpaperAction::Execute(
			applyFailureSettings, candidate,
			CallbacksFor(applyFailureProbe));
	if (applyFailureAction.applyStatus != B_ERROR)
		return Fail("apply failure status was not preserved");
	if (applyFailureAction.historyStatus != B_NO_INIT)
		return Fail("apply failure attempted history persistence");
	if (applyFailureAction.rollbackStatus != B_IO_ERROR)
		return Fail("rollback status was not preserved");
	if (applyFailureProbe.applyCalls != 1
		|| applyFailureProbe.dateCalls != 0
		|| applyFailureProbe.saveCalls != 0) {
		return Fail("apply failure used an unexpected callback count");
	}
	if (!HistoryWasPreserved(applyFailureSettings))
		return Fail("apply failure changed history");

	AppSettings dateFailureSettings = PreviousSettings();
	ActionProbe dateFailureProbe = NewProbe();
	dateFailureProbe.dateStatus = B_ERROR;

	DailyWallpaperActionResult dateFailureAction
		= DailyWallpaperAction::Execute(
			dateFailureSettings, candidate,
			CallbacksFor(dateFailureProbe));
	if (dateFailureAction.applyStatus != B_OK)
		return Fail("date failure lost successful apply status");
	if (dateFailureAction.historyStatus != B_ERROR)
		return Fail("date failure status was not preserved");
	if (dateFailureProbe.applyCalls != 1
		|| dateFailureProbe.dateCalls != 1
		|| dateFailureProbe.saveCalls != 0) {
		return Fail("date failure used an unexpected callback count");
	}
	if (!HistoryWasPreserved(dateFailureSettings))
		return Fail("date failure changed history");

	AppSettings emptyDateSettings = PreviousSettings();
	ActionProbe emptyDateProbe = NewProbe();
	emptyDateProbe.date = "";

	DailyWallpaperActionResult emptyDateAction
		= DailyWallpaperAction::Execute(
			emptyDateSettings, candidate,
			CallbacksFor(emptyDateProbe));
	if (emptyDateAction.applyStatus != B_OK)
		return Fail("empty date lost successful apply status");
	if (emptyDateAction.historyStatus != B_BAD_VALUE)
		return Fail("empty date was not rejected");
	if (emptyDateProbe.saveCalls != 0)
		return Fail("empty date attempted to save history");
	if (!HistoryWasPreserved(emptyDateSettings))
		return Fail("empty date changed history");

	AppSettings saveFailureSettings = PreviousSettings();
	ActionProbe saveFailureProbe = NewProbe();
	saveFailureProbe.saveStatus = B_IO_ERROR;

	DailyWallpaperActionResult saveFailureAction
		= DailyWallpaperAction::Execute(
			saveFailureSettings, candidate,
			CallbacksFor(saveFailureProbe));
	if (saveFailureAction.applyStatus != B_OK)
		return Fail("save failure lost successful apply status");
	if (saveFailureAction.historyStatus != B_IO_ERROR)
		return Fail("save failure status was not preserved");
	if (saveFailureProbe.applyCalls != 1
		|| saveFailureProbe.dateCalls != 1
		|| saveFailureProbe.saveCalls != 1) {
		return Fail("save failure used an unexpected callback count");
	}
	if (saveFailureProbe.savedImagePath.Compare(
			"/boot/home/wallpapers/next.png") != 0
		|| saveFailureProbe.savedUpdateDate.Compare("2026-07-17") != 0) {
		return Fail("save callback did not observe new history");
	}
	if (!HistoryWasPreserved(saveFailureSettings))
		return Fail("save failure did not restore previous history");

	AppSettings successSettings = PreviousSettings();
	ActionProbe successProbe = NewProbe();

	DailyWallpaperActionResult successAction
		= DailyWallpaperAction::Execute(
			successSettings, candidate, CallbacksFor(successProbe));
	if (successAction.applyStatus != B_OK
		|| successAction.historyStatus != B_OK) {
		return Fail("successful action did not report complete success");
	}
	if (successProbe.applyCalls != 1
		|| successProbe.dateCalls != 1
		|| successProbe.saveCalls != 1) {
		return Fail("successful action used an unexpected callback count");
	}
	if (successSettings.LastImagePath().Compare(
			"/boot/home/wallpapers/next.png") != 0
		|| successSettings.LastUpdateDate().Compare("2026-07-17") != 0) {
		return Fail("successful action did not retain new history");
	}

	printf("BeMyDailyWall daily wallpaper action smoke: ok\n");
	return 0;
}
