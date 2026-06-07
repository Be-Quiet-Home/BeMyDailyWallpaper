#ifndef WALLPAPER_INFO_H
#define WALLPAPER_INFO_H

#include <String.h>


class WallpaperInfo {
public:
	WallpaperInfo();
	WallpaperInfo(const char* title,
		const char* description,
		const char* source,
		const char* copyright,
		const char* date);

	BString TooltipText() const;

	const BString& Title() const;
	const BString& Description() const;
	const BString& Source() const;
	const BString& Copyright() const;
	const BString& Date() const;

private:
	BString fTitle;
	BString fDescription;
	BString fSource;
	BString fCopyright;
	BString fDate;
};

#endif
