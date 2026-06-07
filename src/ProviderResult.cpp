#include "ProviderResult.h"


ProviderResult::ProviderResult()
	:
	fInfo(),
	fImagePath("")
{
}


void
ProviderResult::SetInfo(const WallpaperInfo& info)
{
	fInfo = info;
}


const WallpaperInfo&
ProviderResult::Info() const
{
	return fInfo;
}


void
ProviderResult::SetImagePath(const char* imagePath)
{
	fImagePath = imagePath;
}


const BString&
ProviderResult::ImagePath() const
{
	return fImagePath;
}


bool
ProviderResult::HasImagePath() const
{
	return fImagePath.Length() > 0;
}
