#include "MainWindow.h"

#include "AppSettings.h"
#include "DemoProvider.h"
#include "DeskbarView.h"
#include "ProviderResult.h"
#include "WallpaperSetter.h"

#include <Application.h>
#include <Rect.h>
#include <String.h>
#include <StringView.h>
#include <SupportDefs.h>
#include <View.h>


MainWindow::MainWindow()
	:
	BWindow(BRect(100, 100, 580, 310),
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

	BStringView* label = new BStringView(BRect(20, 20, 450, 45),
		"label",
		"BeMyDailyWall is alive.");

	background->AddChild(label);

	AppSettings settings;
	status_t settingsLoadStatus = settings.Load();

	BString settingsStatus("Settings: ");
	if (settingsLoadStatus == B_OK)
		settingsStatus << "loaded";
	else if (settingsLoadStatus == B_ENTRY_NOT_FOUND)
		settingsStatus << "using defaults";
	else
		settingsStatus << "load failed";

	settingsStatus << " provider=";
	settingsStatus << settings.ProviderName();
	settingsStatus << ", archive=";
	settingsStatus << (settings.ArchiveEnabled() ? "enabled" : "disabled");

	BStringView* settingsStatusLabel = new BStringView(BRect(20, 50, 540, 75),
		"settingsStatusLabel",
		settingsStatus.String());

	background->AddChild(settingsStatusLabel);

	DemoProvider provider;
	ProviderResult result;
	status_t providerFetchStatus = provider.Fetch(result);

	DeskbarView* deskbarPreview = new DeskbarView(BRect(20, 90, 51, 121));
	deskbarPreview->SetInfo(result.Info());
	background->AddChild(deskbarPreview);

	BStringView* previewLabel = new BStringView(BRect(65, 95, 480, 120),
		"previewLabel",
		"Deskbar icon preview with provider tooltip");

	background->AddChild(previewLabel);

	BString providerStatus("Provider: ");
	providerStatus << provider.Name();
	providerStatus << (providerFetchStatus == B_OK ? " loaded." : " failed.");

	BStringView* providerStatusLabel = new BStringView(BRect(20, 145, 540, 170),
		"providerStatusLabel",
		providerStatus.String());

	background->AddChild(providerStatusLabel);

	WallpaperSetter setter;
	status_t setterStatus = setter.Apply(result);

	BString setterStatusText("Wallpaper setter: ");
	if (setterStatus == B_OK)
		setterStatusText << "OK.";
	else
		setterStatusText << setter.LastError();

	BStringView* setterStatusLabel = new BStringView(BRect(20, 175, 540, 200),
		"setterStatusLabel",
		setterStatusText.String());

	background->AddChild(setterStatusLabel);
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
