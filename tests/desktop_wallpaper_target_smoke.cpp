#include "DesktopWallpaperTarget.h"

#include <Errors.h>
#include <Messenger.h>
#include <Node.h>

#include <stdio.h>


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall Desktop wallpaper target smoke: %s\n",
		message);
	return 1;
}


int
main()
{
	DesktopWallpaperTarget target;

	if (target.IsReady())
		return Fail("new target was unexpectedly ready");

	if (target.Messenger().IsValid())
		return Fail("new target unexpectedly had a valid messenger");

	status_t status = target.Resolve();
	if (status != B_OK)
		return Fail("could not resolve the real Desktop and Tracker targets");

	if (!target.IsReady())
		return Fail("resolved target was not ready");

	if (target.Node().InitCheck() != B_OK)
		return Fail("resolved Desktop node was invalid");

	if (!target.Messenger().IsValid())
		return Fail("resolved Tracker messenger was invalid");

	if (target.Messenger().Team() < 0)
		return Fail("resolved Tracker messenger had no team");

	printf("BeMyDailyWall Desktop wallpaper target smoke: ok\n");
	return 0;
}
