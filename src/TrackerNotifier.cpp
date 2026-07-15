#include "TrackerNotifier.h"

#include <be_apps/Tracker/Background.h>

#include <Errors.h>
#include <Messenger.h>


static const char* kTrackerApplicationSignature
	= "application/x-vnd.Be-TRAK";


const char*
TrackerNotifier::Signature()
{
	return kTrackerApplicationSignature;
}


int32
TrackerNotifier::RestoreMessage()
{
	return B_RESTORE_BACKGROUND_IMAGE;
}


status_t
TrackerNotifier::ResolveTarget(BMessenger& target)
{
	target = BMessenger();

	status_t status = B_ERROR;
	BMessenger tracker(Signature(), -1, &status);
	if (status != B_OK)
		return status;

	if (!tracker.IsValid() || tracker.Team() < 0)
		return B_BAD_VALUE;

	target = tracker;
	return B_OK;
}


status_t
TrackerNotifier::NotifyRestore(const BMessenger& target)
{
	if (!target.IsValid())
		return B_BAD_VALUE;

	return target.SendMessage((uint32)RestoreMessage());
}
