#include "LocalFolderProvider.h"

#include "WallpaperInfo.h"

#include <Directory.h>
#include <Entry.h>
#include <Errors.h>
#include <File.h>
#include <Path.h>
#include <TranslatorFormats.h>
#include <TranslatorRoster.h>

#include <algorithm>
#include <string.h>
#include <strings.h>
#include <vector>


struct ImageCandidate {
	BString name;
	BString path;
};


static bool
CandidateNameLess(const ImageCandidate& left, const ImageCandidate& right)
{
	return strcmp(left.name.String(), right.name.String()) < 0;
}


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


static status_t
IdentifyImage(const BPath& path, bool& isImage)
{
	isImage = false;

	BFile file(path.Path(), B_READ_ONLY);
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;

	BTranslatorRoster* roster = BTranslatorRoster::Default();
	if (roster == NULL)
		return B_NO_INIT;

	translator_info info;
	status = roster->Identify(
		&file, NULL, &info, 0, NULL, B_TRANSLATOR_BITMAP);
	if (status == B_NO_TRANSLATOR)
		return B_OK;
	if (status != B_OK)
		return status;

	isImage = true;
	return B_OK;
}


LocalFolderProvider::LocalFolderProvider(const char* directoryPath)
	:
	fDirectoryPath(directoryPath != NULL ? directoryPath : ""),
	fPreviousImagePath("")
{
}


LocalFolderProvider::LocalFolderProvider(const char* directoryPath,
	const char* previousImagePath)
	:
	fDirectoryPath(directoryPath != NULL ? directoryPath : ""),
	fPreviousImagePath(previousImagePath != NULL ? previousImagePath : "")
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
	if (fDirectoryPath.IsEmpty())
		return B_BAD_VALUE;

	BEntry directoryEntry(fDirectoryPath.String(), true);
	status_t status = directoryEntry.InitCheck();
	if (status != B_OK)
		return status;

	if (!directoryEntry.Exists())
		return B_ENTRY_NOT_FOUND;

	if (!directoryEntry.IsDirectory())
		return B_NOT_A_DIRECTORY;

	BDirectory directory(&directoryEntry);
	status = directory.InitCheck();
	if (status != B_OK)
		return status;

	std::vector<ImageCandidate> candidates;

	BEntry entry;
	while ((status = directory.GetNextEntry(&entry, false)) == B_OK) {
		if (!entry.IsFile())
			continue;

		const char* name = entry.Name();
		if (name == NULL || !HasSupportedImageExtension(name))
			continue;

		BPath candidatePath;
		status_t pathStatus = entry.GetPath(&candidatePath);
		if (pathStatus != B_OK)
			return pathStatus;

		bool isImage = false;
		status_t identifyStatus = IdentifyImage(candidatePath, isImage);
		if (identifyStatus != B_OK)
			return identifyStatus;
		if (!isImage)
			continue;

		ImageCandidate candidate;
		candidate.name = name;
		candidate.path = candidatePath.Path();
		candidates.push_back(candidate);
	}

	if (status != B_ENTRY_NOT_FOUND)
		return status;

	if (candidates.empty())
		return B_ENTRY_NOT_FOUND;

	std::sort(candidates.begin(), candidates.end(), CandidateNameLess);

	size_t selectedIndex = 0;
	if (!fPreviousImagePath.IsEmpty()) {
		for (size_t index = 0; index < candidates.size(); index++) {
			if (candidates[index].path.Compare(
					fPreviousImagePath.String()) != 0) {
				continue;
			}

			selectedIndex = (index + 1) % candidates.size();
			break;
		}
	}

	const ImageCandidate& selected = candidates[selectedIndex];

	result.SetInfo(WallpaperInfo(
		selected.name.String(),
		"",
		Name(),
		"",
		""));
	result.SetImagePath(selected.path.String());
	return B_OK;
}
