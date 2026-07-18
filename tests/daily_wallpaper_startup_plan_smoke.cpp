#include "DailyWallpaperStartupPlan.h"

#include <Errors.h>
#include <SupportDefs.h>

#include <stdio.h>


struct ExecutionProbe {
	int32 calls;
	status_t result;
};


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall startup plan smoke: %s\n", message);
	return 1;
}


static status_t
ExecuteProbe(void* context)
{
	if (context == 0)
		return B_BAD_VALUE;

	ExecutionProbe* probe = static_cast<ExecutionProbe*>(context);
	probe->calls++;
	return probe->result;
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
		if (DailyWallpaperStartupPlan::Plan(
				idleStates[index], false)
				!= DAILY_WALLPAPER_STARTUP_DO_NOTHING) {
			return Fail("disabled idle state did not plan no action");
		}

		if (DailyWallpaperStartupPlan::Plan(
				idleStates[index], true)
				!= DAILY_WALLPAPER_STARTUP_DO_NOTHING) {
			return Fail("enabled idle state did not plan no action");
		}
	}

	if (DailyWallpaperStartupPlan::Plan(
			DAILY_WALLPAPER_READINESS_READY, false)
			!= DAILY_WALLPAPER_STARTUP_DO_NOTHING) {
		return Fail("disabled ready state did not plan no action");
	}

	if (DailyWallpaperStartupPlan::Plan(
			DAILY_WALLPAPER_READINESS_READY, true)
			!= DAILY_WALLPAPER_STARTUP_APPLY_ONCE) {
		return Fail("enabled ready state did not plan one apply");
	}

	ExecutionProbe idleProbe = {0, B_OK};
	if (DailyWallpaperStartupPlan::Execute(
			DAILY_WALLPAPER_STARTUP_DO_NOTHING,
			ExecuteProbe, &idleProbe) != B_OK) {
		return Fail("no-action execution did not return B_OK");
	}
	if (idleProbe.calls != 0)
		return Fail("no-action execution invoked the executor");

	if (DailyWallpaperStartupPlan::Execute(
			DAILY_WALLPAPER_STARTUP_DO_NOTHING, 0, 0) != B_OK) {
		return Fail("no-action execution required an executor");
	}

	if (DailyWallpaperStartupPlan::Execute(
			DAILY_WALLPAPER_STARTUP_APPLY_ONCE, 0, 0)
			!= B_BAD_VALUE) {
		return Fail("apply-once accepted a missing executor");
	}

	ExecutionProbe successProbe = {0, B_OK};
	if (DailyWallpaperStartupPlan::Execute(
			DAILY_WALLPAPER_STARTUP_APPLY_ONCE,
			ExecuteProbe, &successProbe) != B_OK) {
		return Fail("successful executor status was not returned");
	}
	if (successProbe.calls != 1)
		return Fail("successful executor was not called exactly once");

	ExecutionProbe failureProbe = {0, B_IO_ERROR};
	if (DailyWallpaperStartupPlan::Execute(
			DAILY_WALLPAPER_STARTUP_APPLY_ONCE,
			ExecuteProbe, &failureProbe) != B_IO_ERROR) {
		return Fail("failing executor status was not returned");
	}
	if (failureProbe.calls != 1)
		return Fail("failing executor was retried or skipped");

	ExecutionProbe unknownProbe = {0, B_OK};
	if (DailyWallpaperStartupPlan::Execute(
			(DailyWallpaperStartupAction)99,
			ExecuteProbe, &unknownProbe) != B_BAD_VALUE) {
		return Fail("unknown action did not return B_BAD_VALUE");
	}
	if (unknownProbe.calls != 0)
		return Fail("unknown action invoked the executor");

	printf("BeMyDailyWall startup plan smoke: ok\n");
	return 0;
}
