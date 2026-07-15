#ifndef HAIKU_WALLPAPER_CONTRACT_H
#define HAIKU_WALLPAPER_CONTRACT_H

#include <SupportDefs.h>


class BMessage;
class BNode;
class BPath;


typedef status_t (*HaikuWallpaperCommitAction)(
	const BMessage& message, const void* cookie);


class HaikuWallpaperAttributeBackup {
public:
	HaikuWallpaperAttributeBackup();
	~HaikuWallpaperAttributeBackup();

	bool HasAttribute() const;
	type_code Type() const;
	ssize_t Size() const;

private:
	friend class HaikuWallpaperContract;

	void Reset();

	HaikuWallpaperAttributeBackup(
		const HaikuWallpaperAttributeBackup&) = delete;
	HaikuWallpaperAttributeBackup& operator=(
		const HaikuWallpaperAttributeBackup&) = delete;

	bool fHasAttribute;
	type_code fType;
	char* fData;
	ssize_t fSize;
};


class HaikuWallpaperContract {
public:
	static status_t DesktopTarget(BPath& path);
	static status_t BuildMessage(const char* imagePath, BMessage& message);

	static status_t WriteMessage(BNode& node, const BMessage& message);
	static status_t ReadMessage(const BNode& node, BMessage& message);
	static status_t VerifyMessage(const BMessage& expected,
		const BMessage& actual);

	static status_t CaptureAttribute(const BNode& node,
		HaikuWallpaperAttributeBackup& backup);
	static status_t RestoreAttribute(BNode& node,
		const HaikuWallpaperAttributeBackup& backup);

	static status_t ReplaceMessage(BNode& node, const BMessage& message,
		status_t& rollbackStatus,
		HaikuWallpaperCommitAction commitAction = NULL,
		const void* cookie = NULL);

	static const char* AttributeName();
	static int32 RestoreMessage();
};


#endif
