#ifndef TRACKER_NOTIFIER_H
#define TRACKER_NOTIFIER_H

#include <SupportDefs.h>


class BMessenger;


class TrackerNotifier {
public:
	static const char* Signature();
	static int32 RestoreMessage();

	static status_t ResolveTarget(BMessenger& target);
	static status_t NotifyRestore(const BMessenger& target);
};


#endif
