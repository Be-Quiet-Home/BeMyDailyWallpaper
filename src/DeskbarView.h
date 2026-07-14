#ifndef DESKBAR_VIEW_H
#define DESKBAR_VIEW_H

#include "WallpaperInfo.h"

#include <Point.h>
#include <Rect.h>
#include <String.h>
#include <View.h>


class DeskbarView : public BView {
public:
	DeskbarView();
	DeskbarView(BRect frame);

	void SetInfo(const WallpaperInfo& info);

	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);
	virtual void MouseMoved(BPoint where, uint32 transit,
		const BMessage* dragMessage);

private:
	void _Init();

	WallpaperInfo fInfo;
	BString fTooltipText;
};

#endif
