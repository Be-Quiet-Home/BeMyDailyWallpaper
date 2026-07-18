#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "AppSettings.h"
#include "DailyWallpaperPolicy.h"
#include "DailyWallpaperStartupPlan.h"
#include "ProviderResult.h"

#include <Window.h>


class BButton;
class BCheckBox;
class BFilePanel;
class BMessage;
class BStringView;
class DeskbarView;


class MainWindow : public BWindow {
public:
	MainWindow();
	virtual ~MainWindow();

	virtual void MessageReceived(BMessage* message);
	virtual bool QuitRequested();

private:
	void ApplyWallpaper();
	DailyWallpaperReadiness CurrentDailyReadiness() const;
	DailyWallpaperStartupAction CurrentStartupAction() const;
	status_t ExecuteStartupAction();
	void ChooseLocalFolder();
	void LocalFolderSelected(BMessage* message);
	status_t ReloadProvider();
	void StartupApplyChanged();
	void UpdateDailyStatus();
	void UpdateFolderPath();
	void UpdateStartupActionStatus();

	AppSettings fSettings;
	ProviderResult fProviderResult;
	BFilePanel* fFolderPanel;
	DeskbarView* fDeskbarPreview;
	BStringView* fSettingsStatusLabel;
	BStringView* fPreviewLabel;
	BStringView* fProviderStatusLabel;
	BStringView* fDailyStatusLabel;
	BCheckBox* fStartupApplyCheckBox;
	BStringView* fStartupActionStatusLabel;
	BStringView* fFolderPathLabel;
	BButton* fChooseFolderButton;
	BButton* fApplyButton;
	BStringView* fSetterStatusLabel;
};


#endif
