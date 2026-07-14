#ifndef LOCAL_FOLDER_PROVIDER_H
#define LOCAL_FOLDER_PROVIDER_H

#include "DailyImageProvider.h"

#include <String.h>


class LocalFolderProvider : public DailyImageProvider {
public:
	explicit LocalFolderProvider(const char* directoryPath);

	virtual const char* Name() const;
	virtual status_t Fetch(ProviderResult& result);

private:
	BString fDirectoryPath;
};


#endif
