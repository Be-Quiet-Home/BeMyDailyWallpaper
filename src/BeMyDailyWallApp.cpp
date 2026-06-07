#include "MainWindow.h"

#include <Application.h>


class BeMyDailyWallApp : public BApplication {
public:
	BeMyDailyWallApp();
	virtual void ReadyToRun();
};


BeMyDailyWallApp::BeMyDailyWallApp()
	:
	BApplication("application/x-vnd.BeQuietHome-BeMyDailyWall")
{
}


void
BeMyDailyWallApp::ReadyToRun()
{
	MainWindow* window = new MainWindow();
	window->Show();
}


int
main()
{
	BeMyDailyWallApp app;
	app.Run();
	return 0;
}
