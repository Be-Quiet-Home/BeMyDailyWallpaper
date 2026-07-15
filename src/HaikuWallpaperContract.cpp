#include "HaikuWallpaperContract.h"

#include <be_apps/Tracker/Background.h>

#include <Entry.h>
#include <Errors.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Node.h>
#include <Path.h>
#include <Point.h>
#include <Window.h>
#include <fs_attr.h>

#include <new>
#include <string.h>


static status_t
ValidateSingleMessageField(const BMessage& message, const char* name,
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


HaikuWallpaperAttributeBackup::HaikuWallpaperAttributeBackup()
	:
	fHasAttribute(false),
	fType(0),
	fData(NULL),
	fSize(0)
{
}


HaikuWallpaperAttributeBackup::~HaikuWallpaperAttributeBackup()
{
	delete[] fData;
}


bool
HaikuWallpaperAttributeBackup::HasAttribute() const
{
	return fHasAttribute;
}


type_code
HaikuWallpaperAttributeBackup::Type() const
{
	return fType;
}


ssize_t
HaikuWallpaperAttributeBackup::Size() const
{
	return fSize;
}


void
HaikuWallpaperAttributeBackup::Reset()
{
	delete[] fData;
	fHasAttribute = false;
	fType = 0;
	fData = NULL;
	fSize = 0;
}


status_t
HaikuWallpaperContract::DesktopTarget(BPath& path)
{
	status_t status = find_directory(B_DESKTOP_DIRECTORY, &path);
	if (status != B_OK)
		return status;

	BEntry entry(path.Path(), true);
	status = entry.InitCheck();
	if (status != B_OK)
		return status;

	if (!entry.Exists())
		return B_ENTRY_NOT_FOUND;

	if (!entry.IsDirectory())
		return B_NOT_A_DIRECTORY;

	return B_OK;
}


status_t
HaikuWallpaperContract::BuildMessage(const char* imagePath, BMessage& message)
{
	message.MakeEmpty();

	if (imagePath == NULL || imagePath[0] == '\0')
		return B_BAD_VALUE;

	status_t status = message.AddString(B_BACKGROUND_IMAGE, imagePath);
	if (status != B_OK)
		return status;

	status = message.AddInt32(
		B_BACKGROUND_MODE, B_BACKGROUND_MODE_SCALED);
	if (status != B_OK)
		return status;

	status = message.AddPoint(B_BACKGROUND_ORIGIN, BPoint(0, 0));
	if (status != B_OK)
		return status;

	status = message.AddBool(B_BACKGROUND_ERASE_TEXT, true);
	if (status != B_OK)
		return status;

	return message.AddInt32(
		B_BACKGROUND_WORKSPACES, (int32)B_ALL_WORKSPACES);
}


status_t
HaikuWallpaperContract::WriteMessage(BNode& node, const BMessage& message)
{
	status_t status = node.InitCheck();
	if (status != B_OK)
		return status;

	ssize_t flattenedSize = message.FlattenedSize();
	if (flattenedSize < B_OK)
		return (status_t)flattenedSize;

	char* buffer = new(std::nothrow) char[(size_t)flattenedSize];
	if (buffer == NULL)
		return B_NO_MEMORY;

	status = message.Flatten(buffer, flattenedSize);
	if (status != B_OK) {
		delete[] buffer;
		return status;
	}

	ssize_t bytesWritten = node.WriteAttr(
		AttributeName(), B_MESSAGE_TYPE, 0, buffer, (size_t)flattenedSize);

	delete[] buffer;

	if (bytesWritten < B_OK)
		return (status_t)bytesWritten;

	if (bytesWritten != flattenedSize)
		return B_IO_ERROR;

	return node.Sync();
}


status_t
HaikuWallpaperContract::ReadMessage(const BNode& node, BMessage& message)
{
	message.MakeEmpty();

	status_t status = node.InitCheck();
	if (status != B_OK)
		return status;

	attr_info info;
	status = node.GetAttrInfo(AttributeName(), &info);
	if (status != B_OK)
		return status;

	if (info.type != B_MESSAGE_TYPE)
		return B_BAD_TYPE;

	if (info.size <= 0)
		return B_BAD_DATA;

	char* buffer = new(std::nothrow) char[(size_t)info.size];
	if (buffer == NULL)
		return B_NO_MEMORY;

	ssize_t bytesRead = node.ReadAttr(
		AttributeName(), B_MESSAGE_TYPE, 0, buffer, (size_t)info.size);
	if (bytesRead < B_OK) {
		delete[] buffer;
		return (status_t)bytesRead;
	}

	if (bytesRead != info.size) {
		delete[] buffer;
		return B_IO_ERROR;
	}

	status = message.Unflatten(buffer);
	delete[] buffer;
	return status;
}


status_t
HaikuWallpaperContract::VerifyMessage(const BMessage& expected,
	const BMessage& actual)
{
	if (expected.what != actual.what)
		return B_BAD_DATA;

	if (expected.CountNames(B_ANY_TYPE) != 5
		|| actual.CountNames(B_ANY_TYPE) != 5) {
		return B_BAD_DATA;
	}

	static const char* kFieldNames[] = {
		B_BACKGROUND_IMAGE,
		B_BACKGROUND_MODE,
		B_BACKGROUND_ORIGIN,
		B_BACKGROUND_ERASE_TEXT,
		B_BACKGROUND_WORKSPACES
	};

	static const type_code kFieldTypes[] = {
		B_STRING_TYPE,
		B_INT32_TYPE,
		B_POINT_TYPE,
		B_BOOL_TYPE,
		B_INT32_TYPE
	};

	for (size_t index = 0;
			index < sizeof(kFieldNames) / sizeof(kFieldNames[0]); index++) {
		status_t status = ValidateSingleMessageField(
			expected, kFieldNames[index], kFieldTypes[index]);
		if (status != B_OK)
			return status;

		status = ValidateSingleMessageField(
			actual, kFieldNames[index], kFieldTypes[index]);
		if (status != B_OK)
			return status;
	}

	const char* expectedImagePath = NULL;
	const char* actualImagePath = NULL;
	if (expected.FindString(
			B_BACKGROUND_IMAGE, &expectedImagePath) != B_OK
		|| actual.FindString(
			B_BACKGROUND_IMAGE, &actualImagePath) != B_OK) {
		return B_BAD_DATA;
	}

	if (strcmp(expectedImagePath, actualImagePath) != 0)
		return B_BAD_DATA;

	int32 expectedMode = 0;
	int32 actualMode = 0;
	if (expected.FindInt32(B_BACKGROUND_MODE, &expectedMode) != B_OK
		|| actual.FindInt32(B_BACKGROUND_MODE, &actualMode) != B_OK
		|| expectedMode != actualMode) {
		return B_BAD_DATA;
	}

	BPoint expectedOrigin;
	BPoint actualOrigin;
	if (expected.FindPoint(B_BACKGROUND_ORIGIN, &expectedOrigin) != B_OK
		|| actual.FindPoint(B_BACKGROUND_ORIGIN, &actualOrigin) != B_OK
		|| expectedOrigin.x != actualOrigin.x
		|| expectedOrigin.y != actualOrigin.y) {
		return B_BAD_DATA;
	}

	bool expectedTextOutline = false;
	bool actualTextOutline = false;
	if (expected.FindBool(
			B_BACKGROUND_ERASE_TEXT, &expectedTextOutline) != B_OK
		|| actual.FindBool(
			B_BACKGROUND_ERASE_TEXT, &actualTextOutline) != B_OK
		|| expectedTextOutline != actualTextOutline) {
		return B_BAD_DATA;
	}

	int32 expectedWorkspaces = 0;
	int32 actualWorkspaces = 0;
	if (expected.FindInt32(
			B_BACKGROUND_WORKSPACES, &expectedWorkspaces) != B_OK
		|| actual.FindInt32(
			B_BACKGROUND_WORKSPACES, &actualWorkspaces) != B_OK
		|| expectedWorkspaces != actualWorkspaces) {
		return B_BAD_DATA;
	}

	return B_OK;
}


status_t
HaikuWallpaperContract::CaptureAttribute(const BNode& node,
	HaikuWallpaperAttributeBackup& backup)
{
	backup.Reset();

	status_t status = node.InitCheck();
	if (status != B_OK)
		return status;

	attr_info info;
	status = node.GetAttrInfo(AttributeName(), &info);
	if (status == B_ENTRY_NOT_FOUND)
		return B_OK;

	if (status != B_OK)
		return status;

	if (info.size <= 0)
		return B_BAD_DATA;

	char* data = new(std::nothrow) char[(size_t)info.size];
	if (data == NULL)
		return B_NO_MEMORY;

	ssize_t bytesRead = node.ReadAttr(
		AttributeName(), info.type, 0, data, (size_t)info.size);
	if (bytesRead < B_OK) {
		delete[] data;
		return (status_t)bytesRead;
	}

	if (bytesRead != info.size) {
		delete[] data;
		return B_IO_ERROR;
	}

	backup.fHasAttribute = true;
	backup.fType = info.type;
	backup.fData = data;
	backup.fSize = info.size;
	return B_OK;
}


status_t
HaikuWallpaperContract::RestoreAttribute(BNode& node,
	const HaikuWallpaperAttributeBackup& backup)
{
	status_t status = node.InitCheck();
	if (status != B_OK)
		return status;

	status = node.RemoveAttr(AttributeName());
	if (status != B_OK && status != B_ENTRY_NOT_FOUND)
		return status;

	if (!backup.fHasAttribute)
		return node.Sync();

	ssize_t bytesWritten = node.WriteAttr(
		AttributeName(), backup.fType, 0, backup.fData,
		(size_t)backup.fSize);
	if (bytesWritten < B_OK)
		return (status_t)bytesWritten;

	if (bytesWritten != backup.fSize)
		return B_IO_ERROR;

	return node.Sync();
}


status_t
HaikuWallpaperContract::ReplaceMessage(BNode& node, const BMessage& message,
	status_t& rollbackStatus, HaikuWallpaperCommitAction commitAction,
	const void* cookie)
{
	rollbackStatus = B_NO_INIT;

	HaikuWallpaperAttributeBackup backup;
	status_t status = CaptureAttribute(node, backup);
	if (status != B_OK)
		return status;

	status = WriteMessage(node, message);
	if (status == B_OK) {
		BMessage storedMessage;
		status = ReadMessage(node, storedMessage);
		if (status == B_OK)
			status = VerifyMessage(message, storedMessage);

		if (status == B_OK && commitAction != NULL)
			status = commitAction(storedMessage, cookie);
	}

	if (status == B_OK)
		return B_OK;

	rollbackStatus = RestoreAttribute(node, backup);
	return status;
}


const char*
HaikuWallpaperContract::AttributeName()
{
	return B_BACKGROUND_INFO;
}


int32
HaikuWallpaperContract::RestoreMessage()
{
	return B_RESTORE_BACKGROUND_IMAGE;
}
