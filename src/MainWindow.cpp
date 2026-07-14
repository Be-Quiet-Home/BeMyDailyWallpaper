#include "MainWindow.h"

#include "AppSettings.h"
#include "DemoProvider.h"
#include "DeskbarView.h"
#include "ProviderResult.h"
#include "WallpaperSetter.h"

#include <Application.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <Rect.h>
#include <String.h>
#include <StringView.h>
#include <SupportDefs.h>


MainWindow::MainWindow()
	:
	BWindow(BRect(100, 100, 580, 310),
		"BeMyDailyWall",
		B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS)
{
	BGroupView* background = new BGroupView("background", B_VERTICAL);
	background->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BStringView* label = new BStringView(
		"label",
		"BeMyDailyWall is alive.");

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

	BStringView* settingsStatusLabel = new BStringView(
		"settingsStatusLabel",
		settingsStatus.String());

	DemoProvider provider;
	ProviderResult result;
	status_t providerFetchStatus = provider.Fetch(result);

	DeskbarView* deskbarPreview = new DeskbarView();
	if (providerFetchStatus == B_OK)
		deskbarPreview->SetInfo(result.Info());

	const char* previewText = providerFetchStatus == B_OK
		? "Deskbar icon preview with provider tooltip"
		: "Deskbar icon preview without provider data";

	BStringView* previewLabel = new BStringView(
		"previewLabel",
		previewText);

	BString providerStatus("Provider: ");
	providerStatus << provider.Name();
	providerStatus << (providerFetchStatus == B_OK ? " loaded." : " failed.");

	BStringView* providerStatusLabel = new BStringView(
		"providerStatusLabel",
		providerStatus.String());

	BString setterStatusText("Wallpaper setter: ");
	if (providerFetchStatus != B_OK) {
		setterStatusText << "skipped because provider failed.";
	} else {
		WallpaperSetter setter;
		status_t setterStatus = setter.Apply(result);

		if (setterStatus == B_OK)
			setterStatusText << "OK.";
		else
			setterStatusText << setter.LastError();
	}

	BStringView* setterStatusLabel = new BStringView(
		"setterStatusLabel",
		setterStatusText.String());

	BLayoutBuilder::Group<>(background, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(label)
		.Add(settingsStatusLabel)
		.AddGroup(B_HORIZONTAL)
			.Add(deskbarPreview)
			.Add(previewLabel)
			.AddGlue()
		.End()
		.Add(providerStatusLabel)
		.Add(setterStatusLabel)
		.AddGlue()
	.End();

	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(background);
	ResizeToPreferred();
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
