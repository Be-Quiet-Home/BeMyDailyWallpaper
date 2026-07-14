#ifndef HAIKU_WALLPAPER_CONTRACT_H
#define HAIKU_WALLPAPER_CONTRACT_H

#include <SupportDefs.h>


class BMessage;
class BNode;
class BPath;


class HaikuWallpaperContract {
public:
	static status_t DesktopTarget(BPath& path);
	static status_t BuildMessage(const char* imagePath, BMessage& message);

	static status_t WriteMessage(BNode& node, const BMessage& message);
	static status_t ReadMessage(const BNode& node, BMessage& message);

	static const char* AttributeName();
	static int32 RestoreMessage();
};


#endif
