#include "ProviderResult.h"
#include "WallpaperSetter.h"

#include <Errors.h>

#include <stdio.h>


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall wallpaper setter smoke: %s\n", message);
	return 1;
}


int
main()
{
	WallpaperSetter setter;
	ProviderResult result;

	if (setter.Apply(result) != B_BAD_VALUE)
		return Fail("missing image path did not return B_BAD_VALUE");

	if (setter.LastError().Compare(
		"No wallpaper image path available.") != 0) {
		return Fail("missing image path returned an unexpected error");
	}

	result.SetImagePath("/boot/home/test-wallpaper.jpg");

	if (setter.Apply(result) != B_NOT_SUPPORTED)
		return Fail("unimplemented backend did not return B_NOT_SUPPORTED");

	if (setter.LastError().Compare(
		"Wallpaper backend is not implemented yet.") != 0) {
		return Fail("unimplemented backend returned an unexpected error");
	}

	printf("BeMyDailyWall wallpaper setter smoke: ok\n");
	return 0;
}
