#include "MainWindow.h"

#include "DeskbarView.h"
#include "WallpaperInfo.h"

#include <Application.h>
#include <Rect.h>
#include <StringView.h>
#include <View.h>


MainWindow::MainWindow()
	:
	BWindow(BRect(100, 100, 520, 260),
		"BeMyDailyWall",
		B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS)
{
	BView* background = new BView(Bounds(),
		"background",
		B_FOLLOW_ALL,
		B_WILL_DRAW);

	background->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(background);

	BStringView* label = new BStringView(BRect(20, 20, 380, 45),
		"label",
		"BeMyDailyWall is alive.");

	background->AddChild(label);

	DeskbarView* deskbarPreview = new DeskbarView(BRect(20, 60, 51, 91));
	deskbarPreview->SetInfo(WallpaperInfo(
		"Somewhere else",
		"Your daily window to somewhere else.",
		"Demo provider",
		"Not an affiliated provider.",
		""));
	background->AddChild(deskbarPreview);

	BStringView* previewLabel = new BStringView(BRect(65, 65, 380, 90),
		"previewLabel",
		"Deskbar icon preview with tooltip");

	background->AddChild(previewLabel);
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
