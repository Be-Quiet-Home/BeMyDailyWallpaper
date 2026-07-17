#ifndef DESKTOP_WALLPAPER_TARGET_H
#define DESKTOP_WALLPAPER_TARGET_H

#include <Messenger.h>
#include <Node.h>
#include <SupportDefs.h>


class DesktopWallpaperTarget {
public:
	DesktopWallpaperTarget();

	status_t Resolve();
	bool IsReady() const;

	BNode& Node();
	const BMessenger& Messenger() const;

private:
	BNode fNode;
	BMessenger fMessenger;
};


#endif
