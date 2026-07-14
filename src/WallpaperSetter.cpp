#include "WallpaperSetter.h"

#include <Catalog.h>
#include <Errors.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "WallpaperSetter"


WallpaperSetter::WallpaperSetter()
	:
	fLastError("")
{
}


status_t
WallpaperSetter::Apply(const ProviderResult& result)
{
	if (!result.HasImagePath()) {
		SetLastError(B_TRANSLATE("No wallpaper image path available."));
		return B_BAD_VALUE;
	}

	SetLastError(B_TRANSLATE("Wallpaper backend is not implemented yet."));
	return B_NOT_SUPPORTED;
}


const BString&
WallpaperSetter::LastError() const
{
	return fLastError;
}


void
WallpaperSetter::SetLastError(const char* message)
{
	fLastError = message;
}
