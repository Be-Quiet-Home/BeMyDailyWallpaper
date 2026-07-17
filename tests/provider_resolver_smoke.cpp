#include "AppSettings.h"
#include "DailyImageProvider.h"
#include "DemoProvider.h"
#include "ProviderResolver.h"
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

		BString directoryName("BeMyDailyWall_resolver_images_");
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


	status_t CreatePngFile(const char* name)
	{
		BPath path(fPath);
		status_t status = path.Append(name);
		if (status != B_OK)
			return status;

		BFile file(path.Path(),
			B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
		status = file.InitCheck();
		if (status != B_OK)
			return status;

		ssize_t bytesWritten = file.Write(
			kOnePixelPng, sizeof(kOnePixelPng));
		if (bytesWritten < 0)
			return (status_t)bytesWritten;
		if ((size_t)bytesWritten != sizeof(kOnePixelPng))
			return B_IO_ERROR;

		return B_OK;
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
			while (directory.GetNextEntry(&entry, false) == B_OK)
				entry.Remove();
		}

		BEntry directoryEntry(fPath.Path());
		if (directoryEntry.InitCheck() == B_OK
			&& directoryEntry.Exists()) {
			directoryEntry.Remove();
		}
	}


private:
	BPath fPath;
	status_t fStatus;
};


class TemporaryFile {
public:
	TemporaryFile()
		:
		fStatus(find_directory(B_SYSTEM_TEMP_DIRECTORY, &fPath))
	{
		if (fStatus != B_OK)
			return;

		BString fileName("BeMyDailyWall_provider_resolver_smoke_");
		fileName << (int32)getpid();

		fStatus = fPath.Append(fileName.String());
		if (fStatus != B_OK)
			return;

		_Remove();

		BFile file(fPath.Path(),
			B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
		fStatus = file.InitCheck();
	}

	~TemporaryFile()
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

private:
	void _Remove()
	{
		BEntry entry(fPath.Path());
		if (entry.InitCheck() == B_OK && entry.Exists())
			entry.Remove();
	}

private:
	BPath fPath;
	status_t fStatus;
};


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall provider resolver smoke: %s\n", message);
	return 1;
}


int
main()
{
	AppSettings demoSettings;
	DailyImageProvider* demoProvider = NULL;

	if (ProviderResolver::Create(demoSettings, demoProvider) != B_OK)
		return Fail("default settings did not create a provider");

	if (demoProvider == NULL)
		return Fail("default settings returned no provider");

	if (strcmp(demoProvider->Name(), "Demo provider") != 0) {
		delete demoProvider;
		return Fail("default settings created an unexpected provider");
	}

	ProviderResult demoResult;
	if (demoProvider->Fetch(demoResult) != B_OK) {
		delete demoProvider;
		return Fail("resolved DemoProvider did not fetch successfully");
	}

	delete demoProvider;
	demoProvider = NULL;

	DemoProvider occupiedProvider;
	DailyImageProvider* occupiedOutput = &occupiedProvider;
	if (ProviderResolver::Create(
			demoSettings, occupiedOutput) != B_BAD_VALUE) {
		return Fail("occupied output did not return B_BAD_VALUE");
	}

	if (occupiedOutput != &occupiedProvider)
		return Fail("occupied output pointer was changed");

	TemporaryFile temporaryFile;
	if (temporaryFile.InitCheck() != B_OK)
		return Fail("could not prepare the local path fixture");

	AppSettings localSettings;
	localSettings.SetProviderName("Local folder");
	localSettings.SetLocalFolderPath(temporaryFile.Path().Path());

	DailyImageProvider* localProvider = NULL;
	if (ProviderResolver::Create(localSettings, localProvider) != B_OK)
		return Fail("local settings did not create a provider");

	if (localProvider == NULL)
		return Fail("local settings returned no provider");

	if (strcmp(localProvider->Name(), "Local folder") != 0) {
		delete localProvider;
		return Fail("local settings created an unexpected provider");
	}

	ProviderResult localResult;
	if (localProvider->Fetch(localResult) != B_NOT_A_DIRECTORY) {
		delete localProvider;
		return Fail("local folder path was not forwarded to the provider");
	}

	delete localProvider;
	localProvider = NULL;

	TemporaryImageDirectory imageDirectory;
	if (imageDirectory.InitCheck() != B_OK)
		return Fail("could not prepare the resolver image directory");

	if (imageDirectory.CreatePngFile("Alpha.JPG") != B_OK
		|| imageDirectory.CreatePngFile("middle.jpeg") != B_OK) {
		return Fail("could not create the resolver image fixtures");
	}

	BPath alphaPath;
	if (imageDirectory.PathFor("Alpha.JPG", alphaPath) != B_OK)
		return Fail("could not build the previous image path");

	BPath middlePath;
	if (imageDirectory.PathFor("middle.jpeg", middlePath) != B_OK)
		return Fail("could not build the expected next image path");

	AppSettings rotationSettings;
	rotationSettings.SetProviderName("Local folder");
	rotationSettings.SetLocalFolderPath(imageDirectory.Path().Path());
	rotationSettings.SetLastImagePath(alphaPath.Path());

	DailyImageProvider* rotationProvider = NULL;
	if (ProviderResolver::Create(
			rotationSettings, rotationProvider) != B_OK) {
		return Fail("rotation settings did not create a provider");
	}

	if (rotationProvider == NULL)
		return Fail("rotation settings returned no provider");

	ProviderResult rotationResult;
	if (rotationProvider->Fetch(rotationResult) != B_OK) {
		delete rotationProvider;
		return Fail("resolved rotating provider did not fetch successfully");
	}

	delete rotationProvider;
	rotationProvider = NULL;

	if (rotationResult.ImagePath().Compare(middlePath.Path()) != 0)
		return Fail("last image path was not forwarded to the provider");

	AppSettings unknownSettings;
	unknownSettings.SetProviderName("Unknown provider");

	DailyImageProvider* unknownProvider = NULL;
	if (ProviderResolver::Create(
			unknownSettings, unknownProvider) != B_NAME_NOT_FOUND) {
		return Fail("unknown provider did not return B_NAME_NOT_FOUND");
	}

	if (unknownProvider != NULL)
		return Fail("unknown provider unexpectedly created an object");

	printf("BeMyDailyWall provider resolver smoke: ok\n");
	return 0;
}
