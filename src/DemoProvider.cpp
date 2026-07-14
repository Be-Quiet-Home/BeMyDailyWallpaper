#include "DemoProvider.h"

#include "WallpaperInfo.h"

#include <Catalog.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DemoProvider"


const char*
DemoProvider::Name() const
{
	return "Demo provider";
}


status_t
DemoProvider::Fetch(ProviderResult& result)
{
	result.SetInfo(WallpaperInfo(
		B_TRANSLATE("Somewhere else"),
		B_TRANSLATE("Your daily window to somewhere else."),
		Name(),
		B_TRANSLATE("Not an affiliated provider."),
		""));

	result.SetImagePath("");
	return B_OK;
}
