#include "LocalFolderProvider.h"
#include "ProviderResult.h"

#include <Directory.h>
#include <Entry.h>
#include <Errors.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <String.h>
#include <SupportDefs.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>


static const uint8 kOnePixelPng[] = {
	0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a,
	0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
	0x08, 0x04, 0x00, 0x00, 0x00, 0xb5, 0x1c, 0x0c,
	0x02, 0x00, 0x00, 0x00, 0x0b, 0x49, 0x44, 0x41,
	0x54, 0x78, 0xda, 0x63, 0xfc, 0xff, 0x1f, 0x00,
	0x02, 0xeb, 0x01, 0xf5, 0x8f, 0x59, 0xe2, 0x3f,
	0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
	0xae, 0x42, 0x60, 0x82
};


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

	status_t CreatePngFile(const char* name)
	{
		BPath path(fPath);
		status_t status = path.Append(name);
		if (status != B_OK)
			return status;

		BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
		status = file.InitCheck();
		if (status != B_OK)
			return status;

		ssize_t bytesWritten = file.Write(kOnePixelPng, sizeof(kOnePixelPng));
		if (bytesWritten < 0)
			return (status_t)bytesWritten;
		if ((size_t)bytesWritten != sizeof(kOnePixelPng))
			return B_IO_ERROR;

		return B_OK;
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

	if (directory.CreateFile("000-empty.jpg") != B_OK)
		return Fail("could not create the invalid image fixture");

	ProviderResult invalidResult;
	if (provider.Fetch(invalidResult) != B_ENTRY_NOT_FOUND)
		return Fail("invalid image content was selected");

	if (directory.CreatePngFile("zeta.png") != B_OK
		|| directory.CreatePngFile("Alpha.JPG") != B_OK
		|| directory.CreatePngFile("middle.jpeg") != B_OK) {
		return Fail("could not create the valid image fixtures");
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
