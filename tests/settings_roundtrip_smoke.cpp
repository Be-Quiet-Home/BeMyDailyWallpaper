#include "AppSettings.h"

#include <Entry.h>
#include <Errors.h>
#include <FindDirectory.h>
#include <Path.h>
#include <String.h>
#include <SupportDefs.h>

#include <stdio.h>
#include <unistd.h>


static void
RemoveFile(const BPath& path)
{
	BEntry entry(path.Path());
	if (entry.InitCheck() == B_OK && entry.Exists())
		entry.Remove();
}


class TemporarySettingsFile {
public:
	TemporarySettingsFile()
		:
		fStatus(find_directory(B_SYSTEM_TEMP_DIRECTORY, &fPath))
	{
		if (fStatus != B_OK)
			return;

		BString fileName("BeMyDailyWall_settings_smoke_");
		fileName << (int32)getpid();

		fStatus = fPath.Append(fileName.String());
		if (fStatus == B_OK)
			RemoveFile(fPath);
	}

	~TemporarySettingsFile()
	{
		if (fStatus == B_OK)
			RemoveFile(fPath);
	}

	status_t InitCheck() const
	{
		return fStatus;
	}

	const BPath& Path() const
	{
		return fPath;
	}

private:
	BPath fPath;
	status_t fStatus;
};


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall settings roundtrip smoke: %s\n", message);
	return 1;
}


int
main()
{
	TemporarySettingsFile temporaryFile;
	if (temporaryFile.InitCheck() != B_OK)
		return Fail("could not prepare a temporary settings path");

	AppSettings missingSettings;
	if (missingSettings.LoadFrom(temporaryFile.Path()) != B_ENTRY_NOT_FOUND)
		return Fail("missing settings file did not return B_ENTRY_NOT_FOUND");

	if (missingSettings.ProviderName().Compare("Demo provider") != 0
		|| missingSettings.ArchiveEnabled()
		|| !missingSettings.LastImagePath().IsEmpty()
		|| !missingSettings.LastUpdateDate().IsEmpty()) {
		return Fail("missing settings file changed the default values");
	}

	AppSettings savedSettings;
	savedSettings.SetProviderName("Roundtrip provider");
	savedSettings.SetArchiveEnabled(true);
	savedSettings.SetLastImagePath("/boot/home/test-wallpaper.jpg");
	savedSettings.SetLastUpdateDate("2026-07-14");

	if (savedSettings.SaveTo(temporaryFile.Path()) != B_OK)
		return Fail("SaveTo() failed");

	AppSettings loadedSettings;
	if (loadedSettings.LoadFrom(temporaryFile.Path()) != B_OK)
		return Fail("LoadFrom() failed");

	if (loadedSettings.ProviderName().Compare("Roundtrip provider") != 0)
		return Fail("provider name did not survive the round trip");

	if (!loadedSettings.ArchiveEnabled())
		return Fail("archive flag did not survive the round trip");

	if (loadedSettings.LastImagePath().Compare(
			"/boot/home/test-wallpaper.jpg") != 0) {
		return Fail("last image path did not survive the round trip");
	}

	if (loadedSettings.LastUpdateDate().Compare("2026-07-14") != 0)
		return Fail("last update date did not survive the round trip");

	printf("BeMyDailyWall settings roundtrip smoke: ok\n");
	return 0;
}
