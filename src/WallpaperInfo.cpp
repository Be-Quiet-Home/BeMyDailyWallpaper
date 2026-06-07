#include "WallpaperInfo.h"


WallpaperInfo::WallpaperInfo()
	:
	fTitle("Somewhere else"),
	fDescription("Your daily window to somewhere else."),
	fSource("BeMyDailyWall"),
	fCopyright(""),
	fDate("")
{
}


WallpaperInfo::WallpaperInfo(const char* title,
	const char* description,
	const char* source,
	const char* copyright,
	const char* date)
	:
	fTitle(title),
	fDescription(description),
	fSource(source),
	fCopyright(copyright),
	fDate(date)
{
}


BString
WallpaperInfo::TooltipText() const
{
	BString text("BeMyDailyWall");

	if (fTitle.Length() > 0)
		text << "\nToday: " << fTitle;

	if (fDescription.Length() > 0)
		text << "\n" << fDescription;

	if (fSource.Length() > 0)
		text << "\nSource: " << fSource;

	if (fCopyright.Length() > 0)
		text << "\n" << fCopyright;

	if (fDate.Length() > 0)
		text << "\nDate: " << fDate;

	return text;
}


const BString&
WallpaperInfo::Title() const
{
	return fTitle;
}


const BString&
WallpaperInfo::Description() const
{
	return fDescription;
}


const BString&
WallpaperInfo::Source() const
{
	return fSource;
}


const BString&
WallpaperInfo::Copyright() const
{
	return fCopyright;
}


const BString&
WallpaperInfo::Date() const
{
	return fDate;
}
