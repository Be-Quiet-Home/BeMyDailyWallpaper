#include "DesktopWallpaperTarget.h"

#include "HaikuWallpaperContract.h"
#include "TrackerNotifier.h"

#include <Path.h>


DesktopWallpaperTarget::DesktopWallpaperTarget()
	:
	fNode(),
	fMessenger()
{
}


status_t
DesktopWallpaperTarget::Resolve()
{
	fNode.Unset();
	fMessenger = BMessenger();

	BPath path;
	status_t status = HaikuWallpaperContract::DesktopTarget(path);
	if (status != B_OK)
		return status;

	status = fNode.SetTo(path.Path());
	if (status != B_OK) {
		fNode.Unset();
		return status;
	}

	BMessenger messenger;
	status = TrackerNotifier::ResolveTarget(messenger);
	if (status != B_OK) {
		fNode.Unset();
		return status;
	}

	fMessenger = messenger;
	return B_OK;
}


bool
DesktopWallpaperTarget::IsReady() const
{
	return fNode.InitCheck() == B_OK
		&& fMessenger.IsValid()
		&& fMessenger.Team() >= 0;
}


BNode&
DesktopWallpaperTarget::Node()
{
	return fNode;
}


const BMessenger&
DesktopWallpaperTarget::Messenger() const
{
	return fMessenger;
}
