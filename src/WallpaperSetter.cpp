#include "WallpaperSetter.h"

#include <Errors.h>


WallpaperSetter::WallpaperSetter()
	:
	fLastError("")
{
}


status_t
WallpaperSetter::Apply(const ProviderResult& result)
{
	if (!result.HasImagePath()) {
		SetLastError("No wallpaper image path available.");
		return B_BAD_VALUE;
	}

	SetLastError("Wallpaper backend is not implemented yet.");
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
