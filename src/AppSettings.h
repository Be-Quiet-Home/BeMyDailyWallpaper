#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <String.h>


class AppSettings {
public:
	AppSettings();

	const BString& ProviderName() const;
	void SetProviderName(const char* providerName);

	bool ArchiveEnabled() const;
	void SetArchiveEnabled(bool enabled);

	const BString& LastImagePath() const;
	void SetLastImagePath(const char* imagePath);

	const BString& LastUpdateDate() const;
	void SetLastUpdateDate(const char* updateDate);

private:
	BString fProviderName;
	bool fArchiveEnabled;
	BString fLastImagePath;
	BString fLastUpdateDate;
};

#endif
