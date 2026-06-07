#include "MainWindow.h"

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
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
