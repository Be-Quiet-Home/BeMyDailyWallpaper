#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <Path.h>
#include <String.h>
#include <SupportDefs.h>


class AppSettings {
public:
	AppSettings();

	status_t Load();
	status_t LoadFrom(const BPath& path);
	status_t Save() const;
	status_t SaveTo(const BPath& path) const;

	const BString& ProviderName() const;
	void SetProviderName(const char* providerName);

	const BString& LocalFolderPath() const;
	void SetLocalFolderPath(const char* folderPath);

	bool ArchiveEnabled() const;
	void SetArchiveEnabled(bool enabled);

	bool StartupApplyEnabled() const;
	void SetStartupApplyEnabled(bool enabled);

	const BString& LastImagePath() const;
	void SetLastImagePath(const char* imagePath);

	const BString& LastUpdateDate() const;
	void SetLastUpdateDate(const char* updateDate);

private:
	status_t SettingsPath(BPath& path) const;

	BString fProviderName;
	BString fLocalFolderPath;
	bool fArchiveEnabled;
	bool fStartupApplyEnabled;
	BString fLastImagePath;
	BString fLastUpdateDate;
};

#endif
