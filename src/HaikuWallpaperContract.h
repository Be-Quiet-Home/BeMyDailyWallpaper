#ifndef HAIKU_WALLPAPER_CONTRACT_H
#define HAIKU_WALLPAPER_CONTRACT_H

#include <SupportDefs.h>


class BMessage;
class BPath;


class HaikuWallpaperContract {
public:
	static status_t DesktopTarget(BPath& path);
	static status_t BuildMessage(const char* imagePath, BMessage& message);

	static const char* AttributeName();
	static int32 RestoreMessage();
};


#endif
