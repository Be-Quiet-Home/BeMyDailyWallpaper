#include "LocalFolderProvider.h"

#include "WallpaperInfo.h"

#include <Directory.h>
#include <Entry.h>
#include <Errors.h>
#include <Path.h>

#include <string.h>
#include <strings.h>


static bool
HasSupportedImageExtension(const char* name)
{
	const char* extension = strrchr(name, '.');
	if (extension == NULL)
		return false;

	return strcasecmp(extension, ".jpg") == 0
		|| strcasecmp(extension, ".jpeg") == 0
		|| strcasecmp(extension, ".png") == 0;
}


LocalFolderProvider::LocalFolderProvider(const char* directoryPath)
	:
	fDirectoryPath(directoryPath != NULL ? directoryPath : "")
{
}


const char*
LocalFolderProvider::Name() const
{
	return "Local folder";
}


status_t
LocalFolderProvider::Fetch(ProviderResult& result)
{
	BDirectory directory(fDirectoryPath.String());
	status_t status = directory.InitCheck();
	if (status != B_OK)
		return status;

	BString selectedName;
	BPath selectedPath;

	BEntry entry;
	while ((status = directory.GetNextEntry(&entry, false)) == B_OK) {
		if (!entry.IsFile())
			continue;

		const char* name = entry.Name();
		if (name == NULL || !HasSupportedImageExtension(name))
			continue;

		if (!selectedName.IsEmpty()
			&& strcmp(name, selectedName.String()) >= 0) {
			continue;
		}

		BPath candidatePath;
		status_t pathStatus = entry.GetPath(&candidatePath);
		if (pathStatus != B_OK)
			return pathStatus;

		selectedName = name;
		selectedPath = candidatePath;
	}

	if (status != B_ENTRY_NOT_FOUND)
		return status;

	if (selectedName.IsEmpty())
		return B_ENTRY_NOT_FOUND;

	result.SetInfo(WallpaperInfo(
		selectedName.String(),
		"",
		Name(),
		"",
		""));
	result.SetImagePath(selectedPath.Path());
	return B_OK;
}
