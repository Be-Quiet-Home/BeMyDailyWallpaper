#include "DailyWallpaperAction.h"
#include "DailyWallpaperStartupPlan.h"

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
};


struct CoordinationProbe {
	int32 actionCalls;
	AppSettings* settings;
	const ProviderResult* result;
	DailyWallpaperActionCallbacks callbacks;
	DailyWallpaperActionResult actionResult;
};


static int
Fail(const char* message)
{
	fprintf(stderr,
		"BeMyDailyWall startup action coordination smoke: %s\n",
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
SaveProbe(void* context, const AppSettings&)
{
	if (context == 0)
		return B_BAD_VALUE;

	ActionProbe* probe = static_cast<ActionProbe*>(context);
	probe->saveCalls++;
	return probe->saveStatus;
}


static status_t
ExecuteDailyAction(void* context)
{
	if (context == 0)
		return B_BAD_VALUE;

	CoordinationProbe* probe
		= static_cast<CoordinationProbe*>(context);
	if (probe->settings == 0 || probe->result == 0)
		return B_BAD_VALUE;

	probe->actionCalls++;
	probe->actionResult = DailyWallpaperAction::Execute(
		*probe->settings, *probe->result, probe->callbacks);

	if (probe->actionResult.applyStatus != B_OK)
		return probe->actionResult.applyStatus;

	return probe->actionResult.historyStatus;
}


static ActionProbe
NewActionProbe()
{
	ActionProbe probe = {
		0,
		0,
		0,
		B_OK,
		B_OK,
		B_OK,
		B_NO_INIT,
		"2026-07-17"
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


static AppSettings
PreviousSettings(bool startupApplyEnabled)
{
	AppSettings settings;
	settings.SetStartupApplyEnabled(startupApplyEnabled);
	settings.SetLastImagePath("/boot/home/wallpapers/previous.png");
	settings.SetLastUpdateDate("2026-07-16");
	return settings;
}


static ProviderResult
Candidate()
{
	ProviderResult result;
	result.SetImagePath("/boot/home/wallpapers/next.png");
	return result;
}


static CoordinationProbe
CoordinationFor(
	AppSettings& settings,
	const ProviderResult& result,
	ActionProbe& actionProbe)
{
	CoordinationProbe coordination = {
		0,
		&settings,
		&result,
		CallbacksFor(actionProbe),
		{B_NO_INIT, B_NO_INIT, B_NO_INIT}
	};
	return coordination;
}


static bool
HistoryWasPreserved(const AppSettings& settings)
{
	return settings.LastImagePath().Compare(
			"/boot/home/wallpapers/previous.png") == 0
		&& settings.LastUpdateDate().Compare("2026-07-16") == 0;
}


static status_t
Run(
	DailyWallpaperReadiness readiness,
	CoordinationProbe& coordination)
{
	const bool startupApplyEnabled
		= coordination.settings != 0
			&& coordination.settings->StartupApplyEnabled();

	DailyWallpaperStartupAction action
		= DailyWallpaperStartupPlan::Plan(
			readiness, startupApplyEnabled);
	return DailyWallpaperStartupPlan::Execute(
		action, ExecuteDailyAction, &coordination);
}


int
main()
{
	const DailyWallpaperReadiness idleStates[] = {
		DAILY_WALLPAPER_READINESS_UNAVAILABLE,
		DAILY_WALLPAPER_READINESS_APPLIED_TODAY,
		DAILY_WALLPAPER_READINESS_NO_CANDIDATE,
		(DailyWallpaperReadiness)99
	};

	for (size_t index = 0;
		index < sizeof(idleStates) / sizeof(idleStates[0]);
		index++) {
		AppSettings settings = PreviousSettings(true);
		ProviderResult candidate = Candidate();
		ActionProbe actionProbe = NewActionProbe();
		CoordinationProbe coordination
			= CoordinationFor(settings, candidate, actionProbe);

		if (Run(idleStates[index], coordination) != B_OK)
			return Fail("idle readiness did not return B_OK");
		if (coordination.actionCalls != 0)
			return Fail("idle readiness invoked the daily action");
		if (actionProbe.applyCalls != 0
			|| actionProbe.dateCalls != 0
			|| actionProbe.saveCalls != 0) {
			return Fail("idle readiness invoked an action callback");
		}
		if (!HistoryWasPreserved(settings))
			return Fail("idle readiness changed history");
	}

	AppSettings disabledSettings = PreviousSettings(false);
	ProviderResult disabledCandidate = Candidate();
	ActionProbe disabledProbe = NewActionProbe();
	CoordinationProbe disabledCoordination = CoordinationFor(
		disabledSettings, disabledCandidate, disabledProbe);

	if (Run(
			DAILY_WALLPAPER_READINESS_READY,
			disabledCoordination) != B_OK) {
		return Fail("disabled ready state did not return B_OK");
	}
	if (disabledCoordination.actionCalls != 0)
		return Fail("disabled ready state invoked the daily action");
	if (disabledProbe.applyCalls != 0
		|| disabledProbe.dateCalls != 0
		|| disabledProbe.saveCalls != 0) {
		return Fail("disabled ready state invoked an action callback");
	}
	if (!HistoryWasPreserved(disabledSettings))
		return Fail("disabled ready state changed history");

	AppSettings emptySettings = PreviousSettings(true);
	ProviderResult emptyResult;
	ActionProbe emptyProbe = NewActionProbe();
	CoordinationProbe emptyCoordination
		= CoordinationFor(emptySettings, emptyResult, emptyProbe);

	if (Run(
			DAILY_WALLPAPER_READINESS_READY,
			emptyCoordination) != B_BAD_VALUE) {
		return Fail("ready state with no candidate was not rejected");
	}
	if (emptyCoordination.actionCalls != 1)
		return Fail("ready state did not invoke the daily action once");
	if (emptyCoordination.actionResult.applyStatus != B_BAD_VALUE)
		return Fail("missing candidate action status was not preserved");
	if (emptyProbe.applyCalls != 0
		|| emptyProbe.dateCalls != 0
		|| emptyProbe.saveCalls != 0) {
		return Fail("missing candidate invoked an action callback");
	}
	if (!HistoryWasPreserved(emptySettings))
		return Fail("missing candidate changed history");

	ProviderResult candidate = Candidate();

	AppSettings applyFailureSettings = PreviousSettings(true);
	ActionProbe applyFailureProbe = NewActionProbe();
	applyFailureProbe.applyStatus = B_ERROR;
	applyFailureProbe.rollbackStatus = B_IO_ERROR;
	CoordinationProbe applyFailureCoordination = CoordinationFor(
		applyFailureSettings, candidate, applyFailureProbe);

	if (Run(
			DAILY_WALLPAPER_READINESS_READY,
			applyFailureCoordination) != B_ERROR) {
		return Fail("apply failure did not reach the startup executor");
	}
	if (applyFailureCoordination.actionCalls != 1
		|| applyFailureProbe.applyCalls != 1
		|| applyFailureProbe.dateCalls != 0
		|| applyFailureProbe.saveCalls != 0) {
		return Fail("apply failure used an unexpected call count");
	}
	if (applyFailureCoordination.actionResult.rollbackStatus
			!= B_IO_ERROR) {
		return Fail("apply rollback status was not preserved");
	}
	if (!HistoryWasPreserved(applyFailureSettings))
		return Fail("apply failure changed history");

	AppSettings saveFailureSettings = PreviousSettings(true);
	ActionProbe saveFailureProbe = NewActionProbe();
	saveFailureProbe.saveStatus = B_IO_ERROR;
	CoordinationProbe saveFailureCoordination = CoordinationFor(
		saveFailureSettings, candidate, saveFailureProbe);

	if (Run(
			DAILY_WALLPAPER_READINESS_READY,
			saveFailureCoordination) != B_IO_ERROR) {
		return Fail("history failure did not reach the startup executor");
	}
	if (saveFailureCoordination.actionCalls != 1
		|| saveFailureProbe.applyCalls != 1
		|| saveFailureProbe.dateCalls != 1
		|| saveFailureProbe.saveCalls != 1) {
		return Fail("history failure used an unexpected call count");
	}
	if (saveFailureCoordination.actionResult.applyStatus != B_OK
		|| saveFailureCoordination.actionResult.historyStatus
			!= B_IO_ERROR) {
		return Fail("history failure statuses were not preserved");
	}
	if (!HistoryWasPreserved(saveFailureSettings))
		return Fail("history failure did not restore previous values");

	AppSettings successSettings = PreviousSettings(true);
	ActionProbe successProbe = NewActionProbe();
	CoordinationProbe successCoordination = CoordinationFor(
		successSettings, candidate, successProbe);

	if (Run(
			DAILY_WALLPAPER_READINESS_READY,
			successCoordination) != B_OK) {
		return Fail("ready success did not return B_OK");
	}
	if (successCoordination.actionCalls != 1
		|| successProbe.applyCalls != 1
		|| successProbe.dateCalls != 1
		|| successProbe.saveCalls != 1) {
		return Fail("ready success did not execute exactly once");
	}
	if (successCoordination.actionResult.applyStatus != B_OK
		|| successCoordination.actionResult.historyStatus != B_OK) {
		return Fail("ready success lost action status");
	}
	if (successSettings.LastImagePath().Compare(
			"/boot/home/wallpapers/next.png") != 0
		|| successSettings.LastUpdateDate().Compare(
			"2026-07-17") != 0) {
		return Fail("ready success did not retain new history");
	}

	printf("BeMyDailyWall startup action coordination smoke: ok\n");
	return 0;
}
