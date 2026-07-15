#include "BettributeStore.h"

#include <Errors.h>
#include <Node.h>
#include <fs_attr.h>

#include <new>


static bool
IsValidAttributeName(const char* name)
{
	return name != NULL && name[0] != '\0';
}


BettributeSnapshot::BettributeSnapshot()
	:
	fHasAttribute(false),
	fType(0),
	fData(NULL),
	fSize(0)
{
}


BettributeSnapshot::~BettributeSnapshot()
{
	delete[] fData;
}


bool
BettributeSnapshot::HasAttribute() const
{
	return fHasAttribute;
}


type_code
BettributeSnapshot::Type() const
{
	return fType;
}


const void*
BettributeSnapshot::Data() const
{
	return fData;
}


ssize_t
BettributeSnapshot::Size() const
{
	return fSize;
}


void
BettributeSnapshot::Reset()
{
	delete[] fData;
	fHasAttribute = false;
	fType = 0;
	fData = NULL;
	fSize = 0;
}


status_t
BettributeStore::Write(BNode& node, const char* name, type_code type,
	const void* data, size_t size)
{
	if (!IsValidAttributeName(name) || data == NULL || size == 0)
		return B_BAD_VALUE;

	status_t status = node.InitCheck();
	if (status != B_OK)
		return status;

	ssize_t bytesWritten = node.WriteAttr(name, type, 0, data, size);
	if (bytesWritten < B_OK)
		return (status_t)bytesWritten;

	if ((size_t)bytesWritten != size)
		return B_IO_ERROR;

	return node.Sync();
}


status_t
BettributeStore::Capture(const BNode& node, const char* name,
	BettributeSnapshot& snapshot)
{
	snapshot.Reset();

	if (!IsValidAttributeName(name))
		return B_BAD_VALUE;

	status_t status = node.InitCheck();
	if (status != B_OK)
		return status;

	attr_info info;
	status = node.GetAttrInfo(name, &info);
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
		name, info.type, 0, data, (size_t)info.size);
	if (bytesRead < B_OK) {
		delete[] data;
		return (status_t)bytesRead;
	}

	if (bytesRead != info.size) {
		delete[] data;
		return B_IO_ERROR;
	}

	snapshot.fHasAttribute = true;
	snapshot.fType = info.type;
	snapshot.fData = data;
	snapshot.fSize = info.size;
	return B_OK;
}


status_t
BettributeStore::Restore(BNode& node, const char* name,
	const BettributeSnapshot& snapshot)
{
	if (!IsValidAttributeName(name))
		return B_BAD_VALUE;

	status_t status = node.InitCheck();
	if (status != B_OK)
		return status;

	status = node.RemoveAttr(name);
	if (status != B_OK && status != B_ENTRY_NOT_FOUND)
		return status;

	if (!snapshot.fHasAttribute)
		return node.Sync();

	return Write(node, name, snapshot.fType, snapshot.fData,
		(size_t)snapshot.fSize);
}
