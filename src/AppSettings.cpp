#include "AppSettings.h"

#include <Entry.h>
#include <Errors.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <String.h>


static const char* kSettingsFileName = "BeMyDailyWall_settings";


static void
RemoveTemporaryFile(const char* path)
{
	BEntry entry(path);
	if (entry.InitCheck() == B_OK && entry.Exists())
		entry.Remove();
}


static status_t
ValidateSingleField(const BMessage& message, const char* name,
	type_code expectedType)
{
	type_code actualType = 0;
	int32 count = 0;

	status_t status = message.GetInfo(name, &actualType, &count);
	if (status != B_OK)
		return status;

	if (actualType != expectedType || count != 1)
		return B_BAD_DATA;

	return B_OK;
}


AppSettings::AppSettings()
	:
	fProviderName("Demo provider"),
	fLocalFolderPath(""),
	fArchiveEnabled(false),
	fLastImagePath(""),
	fLastUpdateDate("")
{
}


status_t
AppSettings::Load()
{
	BPath path;
	status_t status = SettingsPath(path);
	if (status != B_OK)
		return status;

	return LoadFrom(path);
}


status_t
AppSettings::LoadFrom(const BPath& path)
{
	BFile file(path.Path(), B_READ_ONLY);
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;

	BMessage message;
	status = message.Unflatten(&file);
	if (status != B_OK)
		return status;

	status = ValidateSingleField(
		message, "provider_name", B_STRING_TYPE);
	if (status != B_OK)
		return status;

	status = ValidateSingleField(
		message, "local_folder_path", B_STRING_TYPE);
	if (status != B_OK)
		return status;

	status = ValidateSingleField(
		message, "archive_enabled", B_BOOL_TYPE);
	if (status != B_OK)
		return status;

	status = ValidateSingleField(
		message, "last_image_path", B_STRING_TYPE);
	if (status != B_OK)
		return status;

	status = ValidateSingleField(
		message, "last_update_date", B_STRING_TYPE);
	if (status != B_OK)
		return status;

	const char* providerName = 0;
	const char* localFolderPath = 0;
	bool archiveEnabled = false;
	const char* lastImagePath = 0;
	const char* lastUpdateDate = 0;

	status = message.FindString("provider_name", &providerName);
	if (status != B_OK)
		return status;

	status = message.FindString("local_folder_path", &localFolderPath);
	if (status != B_OK)
		return status;

	status = message.FindBool("archive_enabled", &archiveEnabled);
	if (status != B_OK)
		return status;

	status = message.FindString("last_image_path", &lastImagePath);
	if (status != B_OK)
		return status;

	status = message.FindString("last_update_date", &lastUpdateDate);
	if (status != B_OK)
		return status;

	SetProviderName(providerName);
	SetLocalFolderPath(localFolderPath);
	SetArchiveEnabled(archiveEnabled);
	SetLastImagePath(lastImagePath);
	SetLastUpdateDate(lastUpdateDate);

	return B_OK;
}


status_t
AppSettings::Save() const
{
	BPath path;
	status_t status = SettingsPath(path);
	if (status != B_OK)
		return status;

	return SaveTo(path);
}


status_t
AppSettings::SaveTo(const BPath& path) const
{
	BMessage message;

	status_t status = message.AddString("provider_name", fProviderName);
	if (status != B_OK)
		return status;

	status = message.AddString("local_folder_path", fLocalFolderPath);
	if (status != B_OK)
		return status;

	status = message.AddBool("archive_enabled", fArchiveEnabled);
	if (status != B_OK)
		return status;

	status = message.AddString("last_image_path", fLastImagePath);
	if (status != B_OK)
		return status;

	status = message.AddString("last_update_date", fLastUpdateDate);
	if (status != B_OK)
		return status;

	if (path.InitCheck() != B_OK)
		return path.InitCheck();

	BString temporaryPath(path.Path());
	temporaryPath << ".tmp";

	{
		BFile file(temporaryPath.String(),
			B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
		status = file.InitCheck();
		if (status != B_OK)
			return status;

		status = message.Flatten(&file);
		if (status == B_OK)
			status = file.Sync();
	}

	if (status != B_OK) {
		RemoveTemporaryFile(temporaryPath.String());
		return status;
	}

	BEntry temporaryEntry(temporaryPath.String());
	status = temporaryEntry.InitCheck();
	if (status == B_OK)
		status = temporaryEntry.Rename(path.Path(), true);

	if (status != B_OK)
		RemoveTemporaryFile(temporaryPath.String());

	return status;
}


const BString&
AppSettings::ProviderName() const
{
	return fProviderName;
}


void
AppSettings::SetProviderName(const char* providerName)
{
	fProviderName = providerName != 0 ? providerName : "";
}


const BString&
AppSettings::LocalFolderPath() const
{
	return fLocalFolderPath;
}


void
AppSettings::SetLocalFolderPath(const char* folderPath)
{
	fLocalFolderPath = folderPath != 0 ? folderPath : "";
}


bool
AppSettings::ArchiveEnabled() const
{
	return fArchiveEnabled;
}


void
AppSettings::SetArchiveEnabled(bool enabled)
{
	fArchiveEnabled = enabled;
}


const BString&
AppSettings::LastImagePath() const
{
	return fLastImagePath;
}


void
AppSettings::SetLastImagePath(const char* imagePath)
{
	fLastImagePath = imagePath != 0 ? imagePath : "";
}


const BString&
AppSettings::LastUpdateDate() const
{
	return fLastUpdateDate;
}


void
AppSettings::SetLastUpdateDate(const char* updateDate)
{
	fLastUpdateDate = updateDate != 0 ? updateDate : "";
}


status_t
AppSettings::SettingsPath(BPath& path) const
{
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status != B_OK)
		return status;

	return path.Append(kSettingsFileName);
}
