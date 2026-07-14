#include "AppSettings.h"
#include "DailyImageProvider.h"
#include "DemoProvider.h"
#include "ProviderResolver.h"
#include "ProviderResult.h"

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
