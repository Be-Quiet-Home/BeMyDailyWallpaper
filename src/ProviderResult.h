#ifndef PROVIDER_RESULT_H
#define PROVIDER_RESULT_H

#include "WallpaperInfo.h"

#include <String.h>


class ProviderResult {
public:
	ProviderResult();

	void SetInfo(const WallpaperInfo& info);
	const WallpaperInfo& Info() const;

	void SetImagePath(const char* imagePath);
	const BString& ImagePath() const;
	bool HasImagePath() const;

private:
	WallpaperInfo fInfo;
	BString fImagePath;
};

#endif
