#include "TrackerNotifier.h"

#include <be_apps/Tracker/Background.h>

#include <Errors.h>
#include <Looper.h>
#include <Message.h>
#include <Messenger.h>
#include <OS.h>

#include <new>
#include <stdio.h>
#include <string.h>


struct RestoreObservation {
	sem_id semaphore;
	uint32 message;
};


class RestoreTarget : public BLooper {
public:
	explicit RestoreTarget(RestoreObservation& observation)
		:
		BLooper("BeMyDailyWall Tracker notifier smoke target"),
		fObservation(observation)
	{
	}

	virtual void MessageReceived(BMessage* message)
	{
		fObservation.message = message->what;
		release_sem(fObservation.semaphore);
	}

private:
	RestoreObservation& fObservation;
};


static void
QuitTarget(RestoreTarget* target)
{
	if (target != NULL && target->Lock())
		target->Quit();
}


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall Tracker notifier smoke: %s\n", message);
	return 1;
}


int
main()
{
	if (strcmp(
			TrackerNotifier::Signature(), "application/x-vnd.Be-TRAK") != 0) {
		return Fail("Tracker signature changed");
	}

	if (TrackerNotifier::RestoreMessage()
		!= B_RESTORE_BACKGROUND_IMAGE) {
		return Fail("restore message changed");
	}

	BMessenger invalidTarget;
	if (TrackerNotifier::NotifyRestore(invalidTarget) != B_BAD_VALUE)
		return Fail("invalid target did not return B_BAD_VALUE");

	RestoreObservation observation;
	observation.semaphore = create_sem(
		0, "BeMyDailyWall Tracker notifier smoke observation");
	observation.message = 0;
	if (observation.semaphore < B_OK)
		return Fail("could not create the observation semaphore");

	RestoreTarget* target = new(std::nothrow) RestoreTarget(observation);
	if (target == NULL) {
		delete_sem(observation.semaphore);
		return Fail("could not allocate the local target");
	}

	thread_id thread = target->Run();
	if (thread < B_OK) {
		delete target;
		delete_sem(observation.semaphore);
		return Fail("could not run the local target");
	}

	BMessenger targetMessenger(target);
	if (!targetMessenger.IsValid()) {
		QuitTarget(target);
		delete_sem(observation.semaphore);
		return Fail("local target messenger was invalid");
	}

	status_t status = TrackerNotifier::NotifyRestore(targetMessenger);
	if (status != B_OK) {
		QuitTarget(target);
		delete_sem(observation.semaphore);
		return Fail("restore notification was not delivered");
	}

	status = acquire_sem_etc(
		observation.semaphore, 1, B_RELATIVE_TIMEOUT, 1000000);
	if (status != B_OK) {
		QuitTarget(target);
		delete_sem(observation.semaphore);
		return Fail("restore notification was not observed");
	}

	if (observation.message != (uint32)B_RESTORE_BACKGROUND_IMAGE) {
		QuitTarget(target);
		delete_sem(observation.semaphore);
		return Fail("local target received an unexpected message");
	}

	QuitTarget(target);
	delete_sem(observation.semaphore);

	printf("BeMyDailyWall Tracker notifier smoke: ok\n");
	return 0;
}
