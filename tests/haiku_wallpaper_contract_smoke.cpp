#include "HaikuWallpaperContract.h"

#include "BettributeStore.h"

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


static const char kWrongTypeData[] = "not a flattened BMessage";


static status_t
RejectVerifiedMessage(const BMessage& message, const void* cookie)
{
	const char* expectedImagePath = static_cast<const char*>(cookie);
	const char* actualImagePath = NULL;

	if (message.FindString(
			B_BACKGROUND_IMAGE, &actualImagePath) != B_OK) {
		return B_BAD_DATA;
	}

	if (strcmp(actualImagePath, expectedImagePath) != 0)
		return B_BAD_DATA;

	return B_CANCELED;
}


static status_t
WriteWrongTypeAttribute(BNode& node)
{

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

	if (temporaryFile.File().RemoveAttr(
			HaikuWallpaperContract::AttributeName()) != B_OK) {
		return Fail("could not clear the roundtrip attribute");
	}

	BettributeSnapshot backup;
	if (BettributeStore::Capture(
			temporaryFile.File(), HaikuWallpaperContract::AttributeName(),
			backup) != B_OK) {
		return Fail("could not capture the missing attribute state");
	}

	if (backup.HasAttribute())
		return Fail("missing attribute backup unexpectedly contained data");

	if (backup.Type() != 0 || backup.Size() != 0)
		return Fail("missing attribute backup was not neutral");

	if (HaikuWallpaperContract::WriteMessage(
			temporaryFile.File(), message) != B_OK) {
		return Fail("could not write after the missing-state capture");
	}

	if (BettributeStore::Restore(
			temporaryFile.File(), HaikuWallpaperContract::AttributeName(),
			backup) != B_OK) {
		return Fail("could not restore the missing attribute state");
	}

	if (temporaryFile.File().GetAttrInfo(
			HaikuWallpaperContract::AttributeName(), &info)
		!= B_ENTRY_NOT_FOUND) {
		return Fail("missing attribute state was not restored");
	}

	if (HaikuWallpaperContract::WriteMessage(
			temporaryFile.File(), message) != B_OK) {
		return Fail("could not write the original message fixture");
	}

	if (BettributeStore::Capture(
			temporaryFile.File(), HaikuWallpaperContract::AttributeName(),
			backup) != B_OK) {
		return Fail("could not capture the original message attribute");
	}

	if (!backup.HasAttribute())
		return Fail("message backup did not contain an attribute");

	if (backup.Type() != B_MESSAGE_TYPE)
		return Fail("message backup had an unexpected type");

	if (backup.Size() != message.FlattenedSize())
		return Fail("message backup had an unexpected size");

	const char* replacementPath
		= "/boot/home/Pictures/replacement-wallpaper.png";
	BMessage replacementMessage;
	if (HaikuWallpaperContract::BuildMessage(
			replacementPath, replacementMessage) != B_OK) {
		return Fail("could not build the replacement message");
	}

	if (HaikuWallpaperContract::WriteMessage(
			temporaryFile.File(), replacementMessage) != B_OK) {
		return Fail("could not write the replacement message");
	}

	BMessage replacementRoundTrip;
	if (HaikuWallpaperContract::ReadMessage(
			temporaryFile.File(), replacementRoundTrip) != B_OK) {
		return Fail("could not read the replacement message");
	}

	validationError = ValidateContractMessage(
		replacementRoundTrip, replacementPath);
	if (validationError != NULL)
		return Fail(validationError);

	if (BettributeStore::Restore(
			temporaryFile.File(), HaikuWallpaperContract::AttributeName(),
			backup) != B_OK) {
		return Fail("could not restore the original message attribute");
	}

	BMessage restoredMessage;
	if (HaikuWallpaperContract::ReadMessage(
			temporaryFile.File(), restoredMessage) != B_OK) {
		return Fail("could not read the restored message attribute");
	}

	validationError = ValidateContractMessage(restoredMessage, imagePath);
	if (validationError != NULL)
		return Fail(validationError);

	if (temporaryFile.File().RemoveAttr(
			HaikuWallpaperContract::AttributeName()) != B_OK) {
		return Fail("could not clear the restored message attribute");
	}

	if (WriteWrongTypeAttribute(temporaryFile.File()) != B_OK)
		return Fail("could not write the raw backup fixture");

	if (BettributeStore::Capture(
			temporaryFile.File(), HaikuWallpaperContract::AttributeName(),
			backup) != B_OK) {
		return Fail("could not capture the raw attribute");
	}

	if (!backup.HasAttribute())
		return Fail("raw backup did not contain an attribute");

	if (backup.Type() != B_STRING_TYPE)
		return Fail("raw backup did not preserve the attribute type");

	if ((size_t)backup.Size() != sizeof(kWrongTypeData))
		return Fail("raw backup did not preserve the attribute size");

	if (HaikuWallpaperContract::WriteMessage(
			temporaryFile.File(), replacementMessage) != B_OK) {
		return Fail("could not replace the raw attribute");
	}

	if (BettributeStore::Restore(
			temporaryFile.File(), HaikuWallpaperContract::AttributeName(),
			backup) != B_OK) {
		return Fail("could not restore the raw attribute");
	}

	if (temporaryFile.File().GetAttrInfo(
			HaikuWallpaperContract::AttributeName(), &info) != B_OK) {
		return Fail("restored raw attribute was not available");
	}

	if (info.type != B_STRING_TYPE)
		return Fail("restored raw attribute had an unexpected type");

	if ((size_t)info.size != sizeof(kWrongTypeData))
		return Fail("restored raw attribute had an unexpected size");

	char restoredRawData[sizeof(kWrongTypeData)];
	ssize_t bytesRead = temporaryFile.File().ReadAttr(
		HaikuWallpaperContract::AttributeName(), B_STRING_TYPE, 0,
		restoredRawData, sizeof(restoredRawData));
	if (bytesRead != (ssize_t)sizeof(restoredRawData))
		return Fail("restored raw attribute had an unexpected byte count");

	if (memcmp(
			restoredRawData, kWrongTypeData, sizeof(kWrongTypeData)) != 0) {
		return Fail("restored raw attribute changed its bytes");
	}

	if (HaikuWallpaperContract::VerifyMessage(
			message, message) != B_OK) {
		return Fail("identical messages did not verify");
	}

	BMessage extraFieldMessage(message);
	if (extraFieldMessage.AddString("unexpected", "field") != B_OK)
		return Fail("could not prepare the extra-field fixture");

	if (HaikuWallpaperContract::VerifyMessage(
			message, extraFieldMessage) != B_BAD_DATA) {
		return Fail("extra message field was not rejected");
	}

	if (HaikuWallpaperContract::WriteMessage(
			temporaryFile.File(), message) != B_OK) {
		return Fail("could not prepare the replace success fixture");
	}

	status_t rollbackStatus = B_ERROR;
	if (HaikuWallpaperContract::ReplaceMessage(
			temporaryFile.File(), replacementMessage,
			rollbackStatus) != B_OK) {
		return Fail("verified message replacement failed");
	}

	if (rollbackStatus != B_NO_INIT)
		return Fail("successful replacement reported a rollback");

	BMessage committedMessage;
	if (HaikuWallpaperContract::ReadMessage(
			temporaryFile.File(), committedMessage) != B_OK) {
		return Fail("could not read the committed replacement");
	}

	validationError = ValidateContractMessage(
		committedMessage, replacementPath);
	if (validationError != NULL)
		return Fail(validationError);

	if (HaikuWallpaperContract::WriteMessage(
			temporaryFile.File(), message) != B_OK) {
		return Fail("could not prepare the rollback message fixture");
	}

	rollbackStatus = B_ERROR;
	status_t replaceStatus = HaikuWallpaperContract::ReplaceMessage(
		temporaryFile.File(), replacementMessage, rollbackStatus,
		RejectVerifiedMessage, replacementPath);
	if (replaceStatus != B_CANCELED)
		return Fail("commit rejection returned an unexpected status");

	if (rollbackStatus != B_OK)
		return Fail("message rollback did not return B_OK");

	BMessage rolledBackMessage;
	if (HaikuWallpaperContract::ReadMessage(
			temporaryFile.File(), rolledBackMessage) != B_OK) {
		return Fail("could not read the rolled-back message");
	}

	validationError = ValidateContractMessage(rolledBackMessage, imagePath);
	if (validationError != NULL)
		return Fail(validationError);

	if (temporaryFile.File().RemoveAttr(
			HaikuWallpaperContract::AttributeName()) != B_OK) {
		return Fail("could not prepare the missing rollback fixture");
	}

	rollbackStatus = B_ERROR;
	replaceStatus = HaikuWallpaperContract::ReplaceMessage(
		temporaryFile.File(), replacementMessage, rollbackStatus,
		RejectVerifiedMessage, replacementPath);
	if (replaceStatus != B_CANCELED)
		return Fail("missing-state rejection returned an unexpected status");

	if (rollbackStatus != B_OK)
		return Fail("missing-state rollback did not return B_OK");

	if (temporaryFile.File().GetAttrInfo(
			HaikuWallpaperContract::AttributeName(), &info)
		!= B_ENTRY_NOT_FOUND) {
		return Fail("missing state was not restored after rejection");
	}

	if (WriteWrongTypeAttribute(temporaryFile.File()) != B_OK)
		return Fail("could not prepare the raw rollback fixture");

	rollbackStatus = B_ERROR;
	replaceStatus = HaikuWallpaperContract::ReplaceMessage(
		temporaryFile.File(), replacementMessage, rollbackStatus,
		RejectVerifiedMessage, replacementPath);
	if (replaceStatus != B_CANCELED)
		return Fail("raw-state rejection returned an unexpected status");

	if (rollbackStatus != B_OK)
		return Fail("raw-state rollback did not return B_OK");

	if (temporaryFile.File().GetAttrInfo(
			HaikuWallpaperContract::AttributeName(), &info) != B_OK) {
		return Fail("raw state was not restored after rejection");
	}

	if (info.type != B_STRING_TYPE
		|| (size_t)info.size != sizeof(kWrongTypeData)) {
		return Fail("rolled-back raw attribute changed its contract");
	}

	char rolledBackRawData[sizeof(kWrongTypeData)];
	bytesRead = temporaryFile.File().ReadAttr(
		HaikuWallpaperContract::AttributeName(), B_STRING_TYPE, 0,
		rolledBackRawData, sizeof(rolledBackRawData));
	if (bytesRead != (ssize_t)sizeof(rolledBackRawData))
		return Fail("rolled-back raw attribute had an unexpected size");

	if (memcmp(
			rolledBackRawData, kWrongTypeData, sizeof(kWrongTypeData)) != 0) {
		return Fail("rolled-back raw attribute changed its bytes");
	}

	printf("BeMyDailyWall Haiku wallpaper contract smoke: ok\n");
	return 0;
}
