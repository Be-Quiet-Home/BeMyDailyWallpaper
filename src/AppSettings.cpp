#include "AppSettings.h"


AppSettings::AppSettings()
	:
	fProviderName("Demo provider"),
	fArchiveEnabled(false),
	fLastImagePath(""),
	fLastUpdateDate("")
{
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
