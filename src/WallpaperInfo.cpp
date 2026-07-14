#include "WallpaperInfo.h"

#include <Catalog.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "WallpaperInfo"


WallpaperInfo::WallpaperInfo()
	:
	fTitle(""),
	fDescription(""),
	fSource(""),
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

	if (fTitle.Length() > 0) {
		BString titleLine(B_TRANSLATE_COMMENT(
			"Today: %title%",
			"%title% is wallpaper metadata supplied by a provider."));
		titleLine.ReplaceFirst("%title%", fTitle.String());
		text << "\n" << titleLine;
	}

	if (fDescription.Length() > 0)
		text << "\n" << fDescription;

	if (fSource.Length() > 0) {
		BString sourceLine(B_TRANSLATE_COMMENT(
			"Source: %source%",
			"%source% is a stable or provider-supplied source name."));
		sourceLine.ReplaceFirst("%source%", fSource.String());
		text << "\n" << sourceLine;
	}

	if (fCopyright.Length() > 0)
		text << "\n" << fCopyright;

	if (fDate.Length() > 0) {
		BString dateLine(B_TRANSLATE_COMMENT(
			"Date: %date%",
			"%date% is a provider-supplied display date."));
		dateLine.ReplaceFirst("%date%", fDate.String());
		text << "\n" << dateLine;
	}

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
