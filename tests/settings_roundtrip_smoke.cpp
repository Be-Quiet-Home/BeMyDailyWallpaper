#include "AppSettings.h"

#include <Directory.h>
#include <Entry.h>
#include <Errors.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>
#include <String.h>
#include <SupportDefs.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>


static status_t
TemporarySiblingPath(const BPath& path, BPath& temporaryPath)
{
	if (path.InitCheck() != B_OK)
		return path.InitCheck();

	BString pathString(path.Path());
	pathString << ".tmp";
	return temporaryPath.SetTo(pathString.String());
}


static void
RemoveFile(const BPath& path)
{
	BEntry entry(path.Path());
	if (entry.InitCheck() == B_OK && entry.Exists())
		entry.Remove();
}


static status_t
WritePartialSettingsFile(const BPath& path)
{
	BMessage message;

	status_t status = message.AddString("provider_name", "Partial provider");
	if (status != B_OK)
		return status;

	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status = file.InitCheck();
	if (status != B_OK)
		return status;

	return message.Flatten(&file);
}


static status_t
WriteWrongTypeSettingsFile(const BPath& path)
{
	BMessage message;

	status_t status = message.AddString("provider_name", "Wrong type provider");
	if (status != B_OK)
		return status;

	status = message.AddString("archive_enabled", "true");
	if (status != B_OK)
		return status;

	status = message.AddString(
		"last_image_path", "/boot/home/wrong-type-wallpaper.jpg");
	if (status != B_OK)
		return status;

	status = message.AddString("last_update_date", "2026-07-11");
	if (status != B_OK)
		return status;

	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status = file.InitCheck();
	if (status != B_OK)
		return status;

	return message.Flatten(&file);
}


static status_t
WriteExtendedSettingsFile(const BPath& path)
{
	BMessage message;

	status_t status = message.AddString("provider_name", "Extended provider");
	if (status != B_OK)
		return status;

	status = message.AddBool("archive_enabled", true);
	if (status != B_OK)
		return status;

	status = message.AddString(
		"last_image_path", "/boot/home/extended-wallpaper.jpg");
	if (status != B_OK)
		return status;

	status = message.AddString("last_update_date", "2026-07-10");
	if (status != B_OK)
		return status;

	status = message.AddString("future_note", "ignored");
	if (status != B_OK)
		return status;

	status = message.AddInt32("future_revision", 2);
	if (status != B_OK)
		return status;

	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status = file.InitCheck();
	if (status != B_OK)
		return status;

	return message.Flatten(&file);
}


static status_t
WriteDuplicateSettingsFile(const BPath& path)
{
	BMessage message;

	status_t status = message.AddString(
		"provider_name", "First duplicate provider");
	if (status != B_OK)
		return status;

	status = message.AddString(
		"provider_name", "Second duplicate provider");
	if (status != B_OK)
		return status;

	status = message.AddBool("archive_enabled", true);
	if (status != B_OK)
		return status;

	status = message.AddString(
		"last_image_path", "/boot/home/duplicate-wallpaper.jpg");
	if (status != B_OK)
		return status;

	status = message.AddString("last_update_date", "2026-07-09");
	if (status != B_OK)
		return status;

	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status = file.InitCheck();
	if (status != B_OK)
		return status;

	return message.Flatten(&file);
}


static status_t
WriteCorruptSettingsFile(const BPath& path)
{
	static const char* kCorruptData = "not a flattened BMessage";

	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;

	ssize_t bytesWritten = file.Write(kCorruptData, strlen(kCorruptData));
	if (bytesWritten < 0)
		return (status_t)bytesWritten;

	if ((size_t)bytesWritten != strlen(kCorruptData))
		return B_IO_ERROR;

	return B_OK;
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
		if (fStatus != B_OK)
			return;

		RemoveFile(fPath);

		BPath temporaryPath;
		if (TemporarySiblingPath(fPath, temporaryPath) == B_OK)
			RemoveFile(temporaryPath);
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

	if (WriteCorruptSettingsFile(temporaryFile.Path()) != B_OK)
		return Fail("could not write the corrupt settings fixture");

	AppSettings corruptSettings;
	corruptSettings.SetProviderName("Preserved provider");
	corruptSettings.SetArchiveEnabled(true);
	corruptSettings.SetLastImagePath("/boot/home/preserved-wallpaper.jpg");
	corruptSettings.SetLastUpdateDate("2026-07-13");

	if (corruptSettings.LoadFrom(temporaryFile.Path()) == B_OK)
		return Fail("corrupt settings unexpectedly loaded successfully");

	if (corruptSettings.ProviderName().Compare("Preserved provider") != 0
		|| !corruptSettings.ArchiveEnabled()
		|| corruptSettings.LastImagePath().Compare(
			"/boot/home/preserved-wallpaper.jpg") != 0
		|| corruptSettings.LastUpdateDate().Compare("2026-07-13") != 0) {
		return Fail("corrupt settings changed the current values");
	}

	if (WritePartialSettingsFile(temporaryFile.Path()) != B_OK)
		return Fail("could not write the partial settings fixture");

	AppSettings partialSettings;
	partialSettings.SetProviderName("Preserved partial provider");
	partialSettings.SetArchiveEnabled(true);
	partialSettings.SetLastImagePath(
		"/boot/home/preserved-partial-wallpaper.jpg");
	partialSettings.SetLastUpdateDate("2026-07-12");

	if (partialSettings.LoadFrom(temporaryFile.Path()) == B_OK)
		return Fail("partial settings unexpectedly loaded successfully");

	if (partialSettings.ProviderName().Compare(
			"Preserved partial provider") != 0
		|| !partialSettings.ArchiveEnabled()
		|| partialSettings.LastImagePath().Compare(
			"/boot/home/preserved-partial-wallpaper.jpg") != 0
		|| partialSettings.LastUpdateDate().Compare("2026-07-12") != 0) {
		return Fail("partial settings changed the current values");
	}

	if (WriteWrongTypeSettingsFile(temporaryFile.Path()) != B_OK)
		return Fail("could not write the wrong-type settings fixture");

	AppSettings wrongTypeSettings;
	wrongTypeSettings.SetProviderName("Preserved wrong-type provider");
	wrongTypeSettings.SetArchiveEnabled(false);
	wrongTypeSettings.SetLastImagePath(
		"/boot/home/preserved-wrong-type-wallpaper.jpg");
	wrongTypeSettings.SetLastUpdateDate("2026-07-11");

	if (wrongTypeSettings.LoadFrom(temporaryFile.Path()) == B_OK)
		return Fail("wrong-type settings unexpectedly loaded successfully");

	if (wrongTypeSettings.ProviderName().Compare(
			"Preserved wrong-type provider") != 0
		|| wrongTypeSettings.ArchiveEnabled()
		|| wrongTypeSettings.LastImagePath().Compare(
			"/boot/home/preserved-wrong-type-wallpaper.jpg") != 0
		|| wrongTypeSettings.LastUpdateDate().Compare("2026-07-11") != 0) {
		return Fail("wrong-type settings changed the current values");
	}

	if (WriteExtendedSettingsFile(temporaryFile.Path()) != B_OK)
		return Fail("could not write the extended settings fixture");

	AppSettings extendedSettings;
	if (extendedSettings.LoadFrom(temporaryFile.Path()) != B_OK)
		return Fail("settings with unknown fields were rejected");

	if (extendedSettings.ProviderName().Compare("Extended provider") != 0
		|| !extendedSettings.ArchiveEnabled()
		|| extendedSettings.LastImagePath().Compare(
			"/boot/home/extended-wallpaper.jpg") != 0
		|| extendedSettings.LastUpdateDate().Compare("2026-07-10") != 0) {
		return Fail("settings with unknown fields loaded incorrect values");
	}

	if (WriteDuplicateSettingsFile(temporaryFile.Path()) != B_OK)
		return Fail("could not write the duplicate settings fixture");

	AppSettings duplicateSettings;
	duplicateSettings.SetProviderName("Preserved duplicate provider");
	duplicateSettings.SetArchiveEnabled(false);
	duplicateSettings.SetLastImagePath(
		"/boot/home/preserved-duplicate-wallpaper.jpg");
	duplicateSettings.SetLastUpdateDate("2026-07-09");

	if (duplicateSettings.LoadFrom(temporaryFile.Path()) == B_OK)
		return Fail("duplicate settings unexpectedly loaded successfully");

	if (duplicateSettings.ProviderName().Compare(
			"Preserved duplicate provider") != 0
		|| duplicateSettings.ArchiveEnabled()
		|| duplicateSettings.LastImagePath().Compare(
			"/boot/home/preserved-duplicate-wallpaper.jpg") != 0
		|| duplicateSettings.LastUpdateDate().Compare("2026-07-09") != 0) {
		return Fail("duplicate settings changed the current values");
	}

	AppSettings protectedSettings;
	protectedSettings.SetProviderName("Protected provider");
	protectedSettings.SetArchiveEnabled(false);
	protectedSettings.SetLastImagePath(
		"/boot/home/protected-wallpaper.jpg");
	protectedSettings.SetLastUpdateDate("2026-07-08");

	if (protectedSettings.SaveTo(temporaryFile.Path()) != B_OK)
		return Fail("could not write the protected settings fixture");

	BPath blockedTemporaryPath;
	if (TemporarySiblingPath(
			temporaryFile.Path(), blockedTemporaryPath) != B_OK) {
		return Fail("could not prepare the blocked temporary path");
	}

	if (create_directory(blockedTemporaryPath.Path(), 0755) != B_OK)
		return Fail("could not block the temporary settings path");

	AppSettings blockedReplacement;
	blockedReplacement.SetProviderName("Blocked replacement provider");
	blockedReplacement.SetArchiveEnabled(true);
	blockedReplacement.SetLastImagePath(
		"/boot/home/blocked-replacement-wallpaper.jpg");
	blockedReplacement.SetLastUpdateDate("2026-07-07");

	if (blockedReplacement.SaveTo(temporaryFile.Path()) == B_OK)
		return Fail("blocked temporary path unexpectedly allowed saving");

	AppSettings preservedSettings;
	if (preservedSettings.LoadFrom(temporaryFile.Path()) != B_OK)
		return Fail("protected settings were not readable after save failure");

	if (preservedSettings.ProviderName().Compare("Protected provider") != 0
		|| preservedSettings.ArchiveEnabled()
		|| preservedSettings.LastImagePath().Compare(
			"/boot/home/protected-wallpaper.jpg") != 0
		|| preservedSettings.LastUpdateDate().Compare("2026-07-08") != 0) {
		return Fail("save failure changed the protected settings file");
	}

	BEntry blockedTemporaryEntry(blockedTemporaryPath.Path());
	if (blockedTemporaryEntry.InitCheck() != B_OK)
		return Fail("could not inspect the blocked temporary path");

	if (blockedTemporaryEntry.Exists()
		&& blockedTemporaryEntry.Remove() != B_OK) {
		return Fail("could not remove the blocked temporary path");
	}

	AppSettings savedSettings;
	savedSettings.SetProviderName("Roundtrip provider");
	savedSettings.SetArchiveEnabled(true);
	savedSettings.SetLastImagePath("/boot/home/test-wallpaper.jpg");
	savedSettings.SetLastUpdateDate("2026-07-14");

	if (savedSettings.SaveTo(temporaryFile.Path()) != B_OK)
		return Fail("SaveTo() failed");

	BPath completedTemporaryPath;
	if (TemporarySiblingPath(
			temporaryFile.Path(), completedTemporaryPath) != B_OK) {
		return Fail("could not inspect the completed temporary path");
	}

	BEntry completedTemporaryEntry(completedTemporaryPath.Path());
	if (completedTemporaryEntry.InitCheck() != B_OK)
		return Fail("could not inspect the completed temporary entry");

	if (completedTemporaryEntry.Exists())
		return Fail("successful save left the temporary file behind");

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
