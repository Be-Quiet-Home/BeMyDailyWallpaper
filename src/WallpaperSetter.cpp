#include "WallpaperSetter.h"

#include "HaikuWallpaperContract.h"
#include "TrackerNotifier.h"

#include <Catalog.h>
#include <Errors.h>
#include <Message.h>
#include <Messenger.h>
#include <Node.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "WallpaperSetter"


static status_t
NotifyStoredWallpaper(const BMessage&, const void* cookie)
{
	const BMessenger* target = static_cast<const BMessenger*>(cookie);
	if (target == NULL)
		return B_BAD_VALUE;

	return TrackerNotifier::NotifyRestore(*target);
}


WallpaperSetter::WallpaperSetter()
	:
	fNode(NULL),
	fTarget(NULL),
	fLastError(""),
	fLastRollbackStatus(B_NO_INIT)
{
}


WallpaperSetter::WallpaperSetter(BNode& node, const BMessenger& target)
	:
	fNode(&node),
	fTarget(&target),
	fLastError(""),
	fLastRollbackStatus(B_NO_INIT)
{
}


status_t
WallpaperSetter::Apply(const ProviderResult& result)
{
	fLastRollbackStatus = B_NO_INIT;

	if (!result.HasImagePath()) {
		SetLastError(B_TRANSLATE("No wallpaper image path available."));
		return B_BAD_VALUE;
	}

	if (fNode == NULL || fTarget == NULL) {
		SetLastError(B_TRANSLATE("Wallpaper backend is not implemented yet."));
		return B_NOT_SUPPORTED;
	}

	SetLastError("");

	BMessage message;
	status_t status = HaikuWallpaperContract::BuildMessage(
		result.ImagePath().String(), message);
	if (status != B_OK)
		return status;

	return HaikuWallpaperContract::ReplaceMessage(
		*fNode, message, fLastRollbackStatus,
		NotifyStoredWallpaper, fTarget);
}


const BString&
WallpaperSetter::LastError() const
{
	return fLastError;
}


status_t
WallpaperSetter::LastRollbackStatus() const
{
	return fLastRollbackStatus;
}


void
WallpaperSetter::SetLastError(const char* message)
{
	fLastError = message;
}
