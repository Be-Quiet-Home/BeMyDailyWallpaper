#include "HaikuWallpaperContract.h"

#include <be_apps/Tracker/Background.h>

#include <Entry.h>
#include <Errors.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>
#include <Point.h>
#include <Window.h>


status_t
HaikuWallpaperContract::DesktopTarget(BPath& path)
{
	status_t status = find_directory(B_DESKTOP_DIRECTORY, &path);
	if (status != B_OK)
		return status;

	BEntry entry(path.Path(), true);
	status = entry.InitCheck();
	if (status != B_OK)
		return status;

	if (!entry.Exists())
		return B_ENTRY_NOT_FOUND;

	if (!entry.IsDirectory())
		return B_NOT_A_DIRECTORY;

	return B_OK;
}


status_t
HaikuWallpaperContract::BuildMessage(const char* imagePath, BMessage& message)
{
	message.MakeEmpty();

	if (imagePath == NULL || imagePath[0] == '\0')
		return B_BAD_VALUE;

	status_t status = message.AddString(B_BACKGROUND_IMAGE, imagePath);
	if (status != B_OK)
		return status;

	status = message.AddInt32(
		B_BACKGROUND_MODE, B_BACKGROUND_MODE_SCALED);
	if (status != B_OK)
		return status;

	status = message.AddPoint(B_BACKGROUND_ORIGIN, BPoint(0, 0));
	if (status != B_OK)
		return status;

	status = message.AddBool(B_BACKGROUND_ERASE_TEXT, true);
	if (status != B_OK)
		return status;

	return message.AddInt32(
		B_BACKGROUND_WORKSPACES, (int32)B_ALL_WORKSPACES);
}


const char*
HaikuWallpaperContract::AttributeName()
{
	return B_BACKGROUND_INFO;
}


int32
HaikuWallpaperContract::RestoreMessage()
{
	return B_RESTORE_BACKGROUND_IMAGE;
}
