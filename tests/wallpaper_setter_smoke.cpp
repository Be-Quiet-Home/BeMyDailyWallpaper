#include "HaikuWallpaperContract.h"
#include "ProviderResult.h"
#include "TrackerNotifier.h"
#include "WallpaperSetter.h"

#include <be_apps/Tracker/Background.h>

#include <Entry.h>
#include <Errors.h>
#include <File.h>
#include <FindDirectory.h>
#include <Looper.h>
#include <Message.h>
#include <Messenger.h>
#include <Node.h>
#include <OS.h>
#include <Path.h>
#include <String.h>

#include <new>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


class TemporaryAttributeFile {
public:
	TemporaryAttributeFile()
		:
		fStatus(find_directory(B_SYSTEM_TEMP_DIRECTORY, &fPath))
	{
		if (fStatus != B_OK)
			return;

		BString fileName("BeMyDailyWall_setter_backend_smoke_");
		fileName << (int32)getpid();

		fStatus = fPath.Append(fileName.String());
		if (fStatus != B_OK)
			return;

		_Remove();

		fStatus = fFile.SetTo(
			fPath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	}

	~TemporaryAttributeFile()
	{
		fFile.Unset();
		if (fPath.InitCheck() == B_OK)
			_Remove();
	}

	status_t InitCheck() const
	{
		return fStatus;
	}

	BFile& File()
	{
		return fFile;
	}

private:
	void _Remove()
	{
		BEntry entry(fPath.Path());
		if (entry.InitCheck() == B_OK && entry.Exists())
			entry.Remove();
	}

	BPath fPath;
	BFile fFile;
	status_t fStatus;
};


struct RestoreObservation {
	sem_id semaphore;
	uint32 message;
};


class RestoreTarget : public BLooper {
public:
	explicit RestoreTarget(RestoreObservation& observation)
		:
		BLooper("BeMyDailyWall setter backend smoke target"),
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
	fprintf(stderr, "BeMyDailyWall wallpaper setter smoke: %s\n", message);
	return 1;
}


static status_t
ReadImagePath(const BNode& node, BString& imagePath)
{
	imagePath = "";

	BMessage message;
	status_t status = HaikuWallpaperContract::ReadMessage(node, message);
	if (status != B_OK)
		return status;

	const char* storedPath = NULL;
	status = message.FindString(B_BACKGROUND_IMAGE, &storedPath);
	if (status != B_OK)
		return status;

	imagePath = storedPath;
	return B_OK;
}


int
main()
{
	WallpaperSetter setter;
	ProviderResult result;

	if (setter.Apply(result) != B_BAD_VALUE)
		return Fail("missing image path did not return B_BAD_VALUE");

	if (setter.LastError().Compare(
		"No wallpaper image path available.") != 0) {
		return Fail("missing image path returned an unexpected error");
	}

	if (setter.LastRollbackStatus() != B_NO_INIT)
		return Fail("missing image path reported a rollback");

	result.SetImagePath("/boot/home/test-wallpaper.jpg");

	if (setter.Apply(result) != B_NOT_SUPPORTED)
		return Fail("default setter did not preserve the safe stub");

	if (setter.LastError().Compare(
		"Wallpaper backend is not implemented yet.") != 0) {
		return Fail("default setter returned an unexpected error");
	}

	if (setter.LastRollbackStatus() != B_NO_INIT)
		return Fail("default setter reported a rollback");

	TemporaryAttributeFile temporaryFile;
	if (temporaryFile.InitCheck() != B_OK)
		return Fail("could not prepare the temporary attribute file");

	RestoreObservation observation;
	observation.semaphore = create_sem(
		0, "BeMyDailyWall setter backend smoke observation");
	observation.message = 0;
	if (observation.semaphore < B_OK)
		return Fail("could not create the observation semaphore");

	RestoreTarget* target = new(std::nothrow) RestoreTarget(observation);
	if (target == NULL) {
		delete_sem(observation.semaphore);
		return Fail("could not allocate the local restore target");
	}

	thread_id thread = target->Run();
	if (thread < B_OK) {
		delete target;
		delete_sem(observation.semaphore);
		return Fail("could not run the local restore target");
	}

	BMessenger targetMessenger(target);
	if (!targetMessenger.IsValid()) {
		QuitTarget(target);
		delete_sem(observation.semaphore);
		return Fail("local restore target messenger was invalid");
	}

	WallpaperSetter injectedSetter(temporaryFile.File(), targetMessenger);
	status_t status = injectedSetter.Apply(result);
	if (status != B_OK) {
		QuitTarget(target);
		delete_sem(observation.semaphore);
		return Fail("injected setter did not apply the wallpaper");
	}

	if (!injectedSetter.LastError().IsEmpty()) {
		QuitTarget(target);
		delete_sem(observation.semaphore);
		return Fail("successful injected setter retained an error");
	}

	if (injectedSetter.LastRollbackStatus() != B_NO_INIT) {
		QuitTarget(target);
		delete_sem(observation.semaphore);
		return Fail("successful injected setter reported a rollback");
	}

	status = acquire_sem_etc(
		observation.semaphore, 1, B_RELATIVE_TIMEOUT, 1000000);
	if (status != B_OK) {
		QuitTarget(target);
		delete_sem(observation.semaphore);
		return Fail("injected setter notification was not observed");
	}

	if (observation.message != (uint32)TrackerNotifier::RestoreMessage()) {
		QuitTarget(target);
		delete_sem(observation.semaphore);
		return Fail("injected setter sent an unexpected message");
	}

	BString storedPath;
	status = ReadImagePath(temporaryFile.File(), storedPath);
	if (status != B_OK
		|| storedPath.Compare("/boot/home/test-wallpaper.jpg") != 0) {
		QuitTarget(target);
		delete_sem(observation.semaphore);
		return Fail("injected setter stored an unexpected image path");
	}

	QuitTarget(target);
	delete_sem(observation.semaphore);

	BMessage originalMessage;
	const char* originalPath = "/boot/home/original-wallpaper.jpg";
	if (HaikuWallpaperContract::BuildMessage(
			originalPath, originalMessage) != B_OK) {
		return Fail("could not build the original wallpaper fixture");
	}

	if (HaikuWallpaperContract::WriteMessage(
			temporaryFile.File(), originalMessage) != B_OK) {
		return Fail("could not write the original wallpaper fixture");
	}

	BMessenger invalidTarget;
	WallpaperSetter rejectingSetter(temporaryFile.File(), invalidTarget);

	result.SetImagePath("/boot/home/rejected-wallpaper.jpg");
	status = rejectingSetter.Apply(result);
	if (status != B_BAD_VALUE)
		return Fail("invalid notification target returned an unexpected status");

	if (rejectingSetter.LastRollbackStatus() != B_OK)
		return Fail("notification failure did not report a successful rollback");

	if (!rejectingSetter.LastError().IsEmpty())
		return Fail("internal backend failure invented a UI error");

	status = ReadImagePath(temporaryFile.File(), storedPath);
	if (status != B_OK || storedPath.Compare(originalPath) != 0)
		return Fail("notification failure did not restore the original path");

	printf("BeMyDailyWall wallpaper setter smoke: ok\n");
	return 0;
}
