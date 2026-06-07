#ifndef WALLPAPER_SETTER_H
#define WALLPAPER_SETTER_H

#include "ProviderResult.h"

#include <String.h>
#include <SupportDefs.h>


class WallpaperSetter {
public:
	WallpaperSetter();

	status_t Apply(const ProviderResult& result);

	const BString& LastError() const;

private:
	void SetLastError(const char* message);

	BString fLastError;
};

#endif
