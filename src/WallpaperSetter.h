#ifndef WALLPAPER_SETTER_H
#define WALLPAPER_SETTER_H

#include "ProviderResult.h"

#include <String.h>
#include <SupportDefs.h>


class BMessenger;
class BNode;


class WallpaperSetter {
public:
	WallpaperSetter();
	WallpaperSetter(BNode& node, const BMessenger& target);

	status_t Apply(const ProviderResult& result);

	const BString& LastError() const;
	status_t LastRollbackStatus() const;

private:
	void SetLastError(const char* message);

	BNode* fNode;
	const BMessenger* fTarget;
	BString fLastError;
	status_t fLastRollbackStatus;
};

#endif
