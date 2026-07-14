#include "HaikuWallpaperContract.h"

#include <be_apps/Tracker/Background.h>

#include <Entry.h>
#include <Errors.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Node.h>
#include <Path.h>
#include <Point.h>
#include <String.h>
#include <SupportDefs.h>
#include <Window.h>
#include <fs_attr.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>


class TemporaryAttributeFile {
public:
	TemporaryAttributeFile()
		:
		fStatus(find_directory(B_SYSTEM_TEMP_DIRECTORY, &fPath))
	{
		if (fStatus != B_OK)
			return;

		BString fileName("BeMyDailyWall_wallpaper_attribute_smoke_");
		fileName << (int32)getpid();

		fStatus = fPath.Append(fileName.String());
		if (fStatus != B_OK)
			return;

		_Remove();

		fStatus = fFile.SetTo(
			fPath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	}

	~TemporaryAttributeFile()
	{
		fFile.Unset();
		if (fPath.InitCheck() == B_OK)
			_Remove();
	}

	status_t InitCheck() const
	{
		return fStatus;
	}

	BFile& File()
	{
		return fFile;
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
	BFile fFile;
	status_t fStatus;
};


static status_t
ValidateSingleField(const BMessage& message, const char* name,
	type_code expectedType)
{
	type_code actualType = 0;
	int32 count = 0;

	status_t status = message.GetInfo(name, &actualType, &count);
	if (status != B_OK)
		return status;

	if (actualType != expectedType || count != 1)
		return B_BAD_DATA;

	return B_OK;
}


static bool
HasStringField(const BMessage& message, const char* name)
{
	const char* value = NULL;
	return message.FindString(name, &value) == B_OK;
}


static const char*
ValidateContractMessage(const BMessage& message, const char* expectedImagePath)
{
	if (ValidateSingleField(
			message, B_BACKGROUND_IMAGE, B_STRING_TYPE) != B_OK) {
		return "image path field had an unexpected contract";
	}

	if (ValidateSingleField(
			message, B_BACKGROUND_MODE, B_INT32_TYPE) != B_OK) {
		return "mode field had an unexpected contract";
	}

	if (ValidateSingleField(
			message, B_BACKGROUND_ORIGIN, B_POINT_TYPE) != B_OK) {
		return "origin field had an unexpected contract";
	}

	if (ValidateSingleField(
			message, B_BACKGROUND_ERASE_TEXT, B_BOOL_TYPE) != B_OK) {
		return "text outline field had an unexpected contract";
	}

	if (ValidateSingleField(
			message, B_BACKGROUND_WORKSPACES, B_INT32_TYPE) != B_OK) {
		return "workspace field had an unexpected contract";
	}

	const char* storedImagePath = NULL;
	if (message.FindString(
			B_BACKGROUND_IMAGE, &storedImagePath) != B_OK) {
		return "image path field was not readable";
	}

	if (strcmp(storedImagePath, expectedImagePath) != 0)
		return "image path field changed the path";

	int32 mode = -1;
	if (message.FindInt32(B_BACKGROUND_MODE, &mode) != B_OK)
		return "mode field was not readable";

	if (mode != B_BACKGROUND_MODE_SCALED)
		return "message did not request scaled placement";

	BPoint origin;
	if (message.FindPoint(B_BACKGROUND_ORIGIN, &origin) != B_OK)
		return "origin field was not readable";

	if (origin.x != 0 || origin.y != 0)
		return "message returned an unexpected origin";

	bool textOutline = false;
	if (message.FindBool(
			B_BACKGROUND_ERASE_TEXT, &textOutline) != B_OK) {
		return "text outline field was not readable";
	}

	if (!textOutline)
		return "message did not enable icon label outline";

	int32 workspaces = 0;
	if (message.FindInt32(
			B_BACKGROUND_WORKSPACES, &workspaces) != B_OK) {
		return "workspace field was not readable";
	}

	if (workspaces != (int32)B_ALL_WORKSPACES)
		return "message did not target all workspaces";

	return NULL;
}


static status_t
WriteWrongTypeAttribute(BNode& node)
{
	static const char kWrongTypeData[] = "not a flattened BMessage";

	ssize_t bytesWritten = node.WriteAttr(
		HaikuWallpaperContract::AttributeName(), B_STRING_TYPE, 0,
		kWrongTypeData, sizeof(kWrongTypeData));
	if (bytesWritten < B_OK)
		return (status_t)bytesWritten;

	if ((size_t)bytesWritten != sizeof(kWrongTypeData))
		return B_IO_ERROR;

	return node.Sync();
}


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall Haiku wallpaper contract smoke: %s\n",
		message);
	return 1;
}


int
main()
{
	if (strcmp(HaikuWallpaperContract::AttributeName(),
			B_BACKGROUND_INFO) != 0) {
		return Fail("contract returned an unexpected attribute name");
	}

	if (HaikuWallpaperContract::RestoreMessage()
		!= B_RESTORE_BACKGROUND_IMAGE) {
		return Fail("contract returned an unexpected restore message");
	}

	BPath desktopPath;
	if (HaikuWallpaperContract::DesktopTarget(desktopPath) != B_OK)
		return Fail("desktop target was not available");

	BPath expectedDesktopPath;
	if (find_directory(B_DESKTOP_DIRECTORY, &expectedDesktopPath) != B_OK)
		return Fail("could not resolve the expected desktop path");

	if (strcmp(desktopPath.Path(), expectedDesktopPath.Path()) != 0)
		return Fail("contract returned an unexpected desktop path");

	BEntry desktopEntry(desktopPath.Path(), true);
	if (desktopEntry.InitCheck() != B_OK || !desktopEntry.Exists())
		return Fail("desktop target entry was not available");

	if (!desktopEntry.IsDirectory())
		return Fail("desktop target was not a directory");

	BMessage message;
	if (HaikuWallpaperContract::BuildMessage(NULL, message) != B_BAD_VALUE)
		return Fail("null image path did not return B_BAD_VALUE");

	if (HaikuWallpaperContract::BuildMessage("", message) != B_BAD_VALUE)
		return Fail("empty image path did not return B_BAD_VALUE");

	const char* imagePath = "/boot/home/Pictures/wallpaper.png";
	if (HaikuWallpaperContract::BuildMessage(
			imagePath, message) != B_OK) {
		return Fail("valid image path did not build a message");
	}

	const char* validationError = ValidateContractMessage(message, imagePath);
	if (validationError != NULL)
		return Fail(validationError);

	TemporaryAttributeFile temporaryFile;
	if (temporaryFile.InitCheck() != B_OK)
		return Fail("could not prepare the temporary attribute file");

	BMessage missingMessage;
	missingMessage.AddString("stale", "value");
	if (HaikuWallpaperContract::ReadMessage(
			temporaryFile.File(), missingMessage) != B_ENTRY_NOT_FOUND) {
		return Fail("missing attribute did not return B_ENTRY_NOT_FOUND");
	}

	if (HasStringField(missingMessage, "stale"))
		return Fail("missing attribute did not clear the target message");

	if (WriteWrongTypeAttribute(temporaryFile.File()) != B_OK)
		return Fail("could not write the wrong-type attribute fixture");

	BMessage wrongTypeMessage;
	wrongTypeMessage.AddString("stale", "value");
	if (HaikuWallpaperContract::ReadMessage(
			temporaryFile.File(), wrongTypeMessage) != B_BAD_TYPE) {
		return Fail("wrong attribute type did not return B_BAD_TYPE");
	}

	if (HasStringField(wrongTypeMessage, "stale"))
		return Fail("wrong attribute type did not clear the target message");

	if (temporaryFile.File().RemoveAttr(
			HaikuWallpaperContract::AttributeName()) != B_OK) {
		return Fail("could not remove the wrong-type attribute fixture");
	}

	if (HaikuWallpaperContract::WriteMessage(
			temporaryFile.File(), message) != B_OK) {
		return Fail("could not write the wallpaper message attribute");
	}

	attr_info info;
	if (temporaryFile.File().GetAttrInfo(
			HaikuWallpaperContract::AttributeName(), &info) != B_OK) {
		return Fail("written wallpaper attribute was not available");
	}

	if (info.type != B_MESSAGE_TYPE)
		return Fail("written wallpaper attribute had an unexpected type");

	if (info.size != message.FlattenedSize())
		return Fail("written wallpaper attribute had an unexpected size");

	BMessage roundTripMessage;
	roundTripMessage.AddString("stale", "value");
	if (HaikuWallpaperContract::ReadMessage(
			temporaryFile.File(), roundTripMessage) != B_OK) {
		return Fail("could not read the wallpaper message attribute");
	}

	if (HasStringField(roundTripMessage, "stale"))
		return Fail("successful read did not clear stale target fields");

	validationError = ValidateContractMessage(roundTripMessage, imagePath);
	if (validationError != NULL)
		return Fail(validationError);

	printf("BeMyDailyWall Haiku wallpaper contract smoke: ok\n");
	return 0;
}
