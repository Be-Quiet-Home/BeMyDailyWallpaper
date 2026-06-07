#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <Window.h>

class MainWindow : public BWindow {
public:
	MainWindow();
	virtual bool QuitRequested();
};

#endif
