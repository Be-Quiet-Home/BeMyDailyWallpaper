#include "MainWindow.h"

#include "AppSettings.h"
#include "DailyImageProvider.h"
#include "DeskbarView.h"
#include "ProviderResolver.h"
#include "ProviderResult.h"
#include "WallpaperSetter.h"

#include <Application.h>
#include <Catalog.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <Rect.h>
#include <String.h>
#include <StringView.h>
#include <SupportDefs.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


static BString
SettingsStatusText(const AppSettings& settings, status_t loadStatus)
{
	const char* format;
	if (loadStatus == B_OK) {
		format = B_TRANSLATE_COMMENT(
			"Settings: loaded, provider=%provider%, archive=%archive%",
			"%provider% is a provider name; %archive% is enabled or disabled.");
	} else if (loadStatus == B_ENTRY_NOT_FOUND) {
		format = B_TRANSLATE_COMMENT(
			"Settings: using defaults, provider=%provider%, archive=%archive%",
			"%provider% is a provider name; %archive% is enabled or disabled.");
	} else {
		format = B_TRANSLATE_COMMENT(
			"Settings: load failed, provider=%provider%, archive=%archive%",
			"%provider% is a provider name; %archive% is enabled or disabled.");
	}

	BString text(format);
	text.ReplaceFirst("%provider%", settings.ProviderName().String());
	text.ReplaceFirst("%archive%", settings.ArchiveEnabled()
		? B_TRANSLATE("enabled") : B_TRANSLATE("disabled"));
	return text;
}


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
		B_TRANSLATE("BeMyDailyWall is alive."));

	AppSettings settings;
	status_t settingsLoadStatus = settings.Load();
	BString settingsStatus = SettingsStatusText(settings, settingsLoadStatus);

	BStringView* settingsStatusLabel = new BStringView(
		"settingsStatusLabel",
		settingsStatus.String());

	DailyImageProvider* provider = NULL;
	status_t providerStatus = ProviderResolver::Create(settings, provider);

	ProviderResult result;
	if (providerStatus == B_OK)
		providerStatus = provider->Fetch(result);

	BString providerName(settings.ProviderName());
	if (provider != NULL) {
		providerName = provider->Name();
		delete provider;
		provider = NULL;
	}

	DeskbarView* deskbarPreview = new DeskbarView();
	if (providerStatus == B_OK)
		deskbarPreview->SetInfo(result.Info());

	const char* previewText = providerStatus == B_OK
		? B_TRANSLATE("Deskbar icon preview with provider tooltip")
		: B_TRANSLATE("Deskbar icon preview without provider data");

	BStringView* previewLabel = new BStringView(
		"previewLabel",
		previewText);

	BString providerStatusText(providerStatus == B_OK
		? B_TRANSLATE_COMMENT(
			"Provider: %provider% loaded.",
			"%provider% is a provider name.")
		: B_TRANSLATE_COMMENT(
			"Provider: %provider% failed.",
			"%provider% is a provider name."));
	providerStatusText.ReplaceFirst(
		"%provider%", providerName.String());

	BStringView* providerStatusLabel = new BStringView(
		"providerStatusLabel",
		providerStatusText.String());

	BString setterStatusText;
	if (providerStatus != B_OK) {
		setterStatusText = B_TRANSLATE(
			"Wallpaper setter: skipped because provider failed.");
	} else {
		WallpaperSetter setter;
		status_t setterStatus = setter.Apply(result);

		if (setterStatus == B_OK) {
			setterStatusText = B_TRANSLATE("Wallpaper setter: OK.");
		} else {
			setterStatusText = B_TRANSLATE_COMMENT(
				"Wallpaper setter: %error%",
				"%error% is a component-owned error message.");
			setterStatusText.ReplaceFirst(
				"%error%", setter.LastError().String());
		}
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
