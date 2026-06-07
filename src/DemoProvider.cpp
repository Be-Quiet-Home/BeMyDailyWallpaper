#include "DemoProvider.h"

#include "WallpaperInfo.h"


const char*
DemoProvider::Name() const
{
	return "Demo provider";
}


bool
DemoProvider::Fetch(ProviderResult& result)
{
	result.SetInfo(WallpaperInfo(
		"Somewhere else",
		"Your daily window to somewhere else.",
		Name(),
		"Not an affiliated provider.",
		""));

	result.SetImagePath("");
	return true;
}
