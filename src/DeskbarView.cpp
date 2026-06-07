#include "DeskbarView.h"

#include <InterfaceDefs.h>


DeskbarView::DeskbarView(BRect frame)
	:
	BView(frame,
		"DeskbarView",
		B_FOLLOW_NONE,
		B_WILL_DRAW)
{
	SetToolTip("BeMyDailyWall");
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
