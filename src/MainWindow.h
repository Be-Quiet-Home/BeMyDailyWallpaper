#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ProviderResult.h"

#include <Window.h>


class BButton;
class BMessage;
class BStringView;


class MainWindow : public BWindow {
public:
	MainWindow();

	virtual void MessageReceived(BMessage* message);
	virtual bool QuitRequested();

private:
	void ApplyWallpaper();

	ProviderResult fProviderResult;
	BButton* fApplyButton;
	BStringView* fSetterStatusLabel;
};


#endif
