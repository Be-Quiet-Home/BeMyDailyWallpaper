#include "LocalFolderProvider.h"
#include "ProviderResult.h"

#include <Directory.h>
#include <Entry.h>
#include <Errors.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <String.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>


class TemporaryImageDirectory {
public:
	TemporaryImageDirectory()
		:
		fStatus(find_directory(B_SYSTEM_TEMP_DIRECTORY, &fPath))
	{
		if (fStatus != B_OK)
			return;

		BString directoryName("BeMyDailyWall_local_folder_smoke_");
		directoryName << (int32)getpid();

		fStatus = fPath.Append(directoryName.String());
		if (fStatus != B_OK)
			return;

		_Remove();
		fStatus = create_directory(fPath.Path(), 0755);
	}

	~TemporaryImageDirectory()
	{
		if (fPath.InitCheck() == B_OK)
			_Remove();
	}

	status_t InitCheck() const
	{
		return fStatus;
	}

	const BPath& Path() const
	{
		return fPath;
	}

	status_t CreateFile(const char* name)
	{
		BPath path(fPath);
		status_t status = path.Append(name);
		if (status != B_OK)
			return status;

		BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
		return file.InitCheck();
	}

	status_t CreateDirectory(const char* name)
	{
		BPath path(fPath);
		status_t status = path.Append(name);
		if (status != B_OK)
			return status;

		return create_directory(path.Path(), 0755);
	}

	status_t PathFor(const char* name, BPath& path) const
	{
		path = fPath;
		return path.Append(name);
	}

private:
	void _Remove()
	{
		BDirectory directory(fPath.Path());
		if (directory.InitCheck() == B_OK) {
			BEntry entry;
			while (directory.GetNextEntry(&entry, false) == B_OK) {
				if (entry.IsDirectory()) {
					BDirectory child(&entry);
					if (child.InitCheck() == B_OK) {
						BEntry childEntry;
						while (child.GetNextEntry(
								&childEntry, false) == B_OK) {
							childEntry.Remove();
						}
					}
				}

				entry.Remove();
			}
		}

		BEntry directoryEntry(fPath.Path());
		if (directoryEntry.InitCheck() == B_OK && directoryEntry.Exists())
			directoryEntry.Remove();
	}

private:
	BPath fPath;
	status_t fStatus;
};


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall local folder provider smoke: %s\n", message);
	return 1;
}


int
main()
{
	TemporaryImageDirectory directory;
	if (directory.InitCheck() != B_OK)
		return Fail("could not prepare the temporary directory");

	LocalFolderProvider provider(directory.Path().Path());
	if (strcmp(provider.Name(), "Local folder") != 0)
		return Fail("provider returned an unexpected stable name");

	ProviderResult emptyResult;
	if (provider.Fetch(emptyResult) != B_ENTRY_NOT_FOUND)
		return Fail("empty directory did not return B_ENTRY_NOT_FOUND");

	if (directory.CreateFile("000-not-an-image.txt") != B_OK)
		return Fail("could not create the unsupported fixture");

	if (directory.CreateDirectory("001-fake.jpg") != B_OK)
		return Fail("could not create the directory fixture");

	if (directory.CreateFile("zeta.png") != B_OK
		|| directory.CreateFile("Alpha.JPG") != B_OK
		|| directory.CreateFile("middle.jpeg") != B_OK) {
		return Fail("could not create the image fixtures");
	}

	ProviderResult result;
	if (provider.Fetch(result) != B_OK)
		return Fail("provider did not select an image");

	BPath expectedPath;
	if (directory.PathFor("Alpha.JPG", expectedPath) != B_OK)
		return Fail("could not build the expected image path");

	if (result.ImagePath().Compare(expectedPath.Path()) != 0)
		return Fail("provider selected an unexpected image path");

	if (!result.HasImagePath())
		return Fail("selected image path was not reported");

	if (result.Info().Title().Compare("Alpha.JPG") != 0)
		return Fail("provider returned an unexpected title");

	if (result.Info().Description().Length() != 0)
		return Fail("provider returned an unexpected description");

	if (result.Info().Source().Compare(provider.Name()) != 0)
		return Fail("provider returned an unexpected source");

	if (result.Info().Copyright().Length() != 0)
		return Fail("provider returned an unexpected attribution");

	if (result.Info().Date().Length() != 0)
		return Fail("provider returned an unexpected date");

	printf("BeMyDailyWall local folder provider smoke: ok\n");
	return 0;
}
