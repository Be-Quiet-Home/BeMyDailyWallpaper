#include "AppSettings.h"

#include <Errors.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>


static const char* kSettingsFileName = "BeMyDailyWall_settings";


AppSettings::AppSettings()
	:
	fProviderName("Demo provider"),
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

	BFile file(path.Path(), B_READ_ONLY);
	status = file.InitCheck();
	if (status != B_OK)
		return status;

	BMessage message;
	status = message.Unflatten(&file);
	if (status != B_OK)
		return status;

	const char* stringValue = 0;
	bool boolValue = false;

	if (message.FindString("provider_name", &stringValue) == B_OK)
		SetProviderName(stringValue);

	if (message.FindBool("archive_enabled", &boolValue) == B_OK)
		SetArchiveEnabled(boolValue);

	if (message.FindString("last_image_path", &stringValue) == B_OK)
		SetLastImagePath(stringValue);

	if (message.FindString("last_update_date", &stringValue) == B_OK)
		SetLastUpdateDate(stringValue);

	return B_OK;
}


status_t
AppSettings::Save() const
{
	BPath path;
	status_t status = SettingsPath(path);
	if (status != B_OK)
		return status;

	BMessage message;
	message.AddString("provider_name", fProviderName);
	message.AddBool("archive_enabled", fArchiveEnabled);
	message.AddString("last_image_path", fLastImagePath);
	message.AddString("last_update_date", fLastUpdateDate);

	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status = file.InitCheck();
	if (status != B_OK)
		return status;

	return message.Flatten(&file);
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
