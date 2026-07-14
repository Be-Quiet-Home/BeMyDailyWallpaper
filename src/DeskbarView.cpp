#include "DeskbarView.h"

#include <InterfaceDefs.h>
#include <Size.h>


static const float kDeskbarPreviewSize = 32.0f;


DeskbarView::DeskbarView()
	:
	BView("DeskbarView", B_WILL_DRAW)
{
	_Init();
}


DeskbarView::DeskbarView(BRect frame)
	:
	BView(frame,
		"DeskbarView",
		B_FOLLOW_NONE,
		B_WILL_DRAW)
{
	_Init();
}


void
DeskbarView::_Init()
{
	SetExplicitSize(BSize(kDeskbarPreviewSize, kDeskbarPreviewSize));
	SetInfo(WallpaperInfo());
}


void
DeskbarView::SetInfo(const WallpaperInfo& info)
{
	fInfo = info;
	fTooltipText = fInfo.TooltipText();

	SetToolTip(fTooltipText.String());
	Invalidate();
}


void
DeskbarView::AttachedToWindow()
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}


void
DeskbarView::Draw(BRect updateRect)
{
	(void)updateRect;

	BRect bounds = Bounds();

	SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	FillRect(bounds);

	bounds.InsetBy(3, 3);
	SetHighColor(0, 0, 0);
	StrokeEllipse(bounds);

	bounds.InsetBy(4, 4);
	SetHighColor(80, 120, 180);
	FillEllipse(bounds);
}


void
DeskbarView::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage)
{
	(void)where;
	(void)dragMessage;

	if (transit == B_ENTERED_VIEW)
		ShowToolTip();
}
