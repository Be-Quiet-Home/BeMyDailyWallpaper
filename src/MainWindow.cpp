#include "MainWindow.h"

#include "AppSettings.h"
#include "DailyImageProvider.h"
#include "DeskbarView.h"
#include "DesktopWallpaperTarget.h"
#include "ProviderResolver.h"
#include "WallpaperSetter.h"

#include <Application.h>
#include <Button.h>
#include <Catalog.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Rect.h>
#include <String.h>
#include <StringView.h>
#include <SupportDefs.h>

#include <string.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


static const uint32 kApplyWallpaper = 'ApWp';


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


static BString
OperationFailureText(status_t status, status_t rollbackStatus)
{
	const char* format;
	BString text;

	if (rollbackStatus != B_NO_INIT && rollbackStatus != B_OK) {
		format = B_TRANSLATE_COMMENT(
			"Wallpaper failed: %error% (restore failed: %restore%)",
			"%error% and %restore% are Haiku status descriptions.");
		text = format;
		text.ReplaceFirst("%error%", strerror(status));
		text.ReplaceFirst("%restore%", strerror(rollbackStatus));
		return text;
	}

	format = B_TRANSLATE_COMMENT(
		"Wallpaper failed: %error%",
		"%error% is a Haiku status description.");
	text = format;
	text.ReplaceFirst("%error%", strerror(status));
	return text;
}


MainWindow::MainWindow()
	:
	BWindow(BRect(100, 100, 580, 310),
		"BeMyDailyWall",
		B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS),
	fProviderResult(),
	fApplyButton(NULL),
	fSetterStatusLabel(NULL)
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

	if (providerStatus == B_OK)
		providerStatus = provider->Fetch(fProviderResult);

	BString providerName(settings.ProviderName());
	if (provider != NULL) {
		providerName = provider->Name();
		delete provider;
		provider = NULL;
	}

	DeskbarView* deskbarPreview = new DeskbarView();
	if (providerStatus == B_OK)
		deskbarPreview->SetInfo(fProviderResult.Info());

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

	const bool canApply = providerStatus == B_OK
		&& fProviderResult.HasImagePath();

	const char* setterStatusText;
	if (providerStatus != B_OK) {
		setterStatusText = B_TRANSLATE(
			"Wallpaper: unavailable because the provider failed.");
	} else if (!fProviderResult.HasImagePath()) {
		setterStatusText = B_TRANSLATE(
			"Wallpaper: no image path from the provider.");
	} else {
		setterStatusText = B_TRANSLATE("Wallpaper: ready to apply.");
	}

	fSetterStatusLabel = new BStringView(
		"setterStatusLabel",
		setterStatusText);

	fApplyButton = new BButton(
		"applyWallpaperButton",
		B_TRANSLATE("Apply wallpaper"),
		new BMessage(kApplyWallpaper));
	fApplyButton->SetEnabled(canApply);

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
		.Add(fSetterStatusLabel)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fApplyButton)
		.End()
		.AddGlue()
	.End();

	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(background);
	ResizeToPreferred();
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kApplyWallpaper:
			ApplyWallpaper();
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::ApplyWallpaper()
{
	if (!fProviderResult.HasImagePath()) {
		fSetterStatusLabel->SetText(B_TRANSLATE(
			"Wallpaper: no image path from the provider."));
		return;
	}

	fApplyButton->SetEnabled(false);

	DesktopWallpaperTarget target;
	status_t status = target.Resolve();
	if (status != B_OK) {
		BString text(B_TRANSLATE_COMMENT(
			"Wallpaper target failed: %error%",
			"%error% is a Haiku status description."));
		text.ReplaceFirst("%error%", strerror(status));
		fSetterStatusLabel->SetText(text.String());
		fApplyButton->SetEnabled(true);
		return;
	}

	WallpaperSetter setter(target.Node(), target.Messenger());
	status = setter.Apply(fProviderResult);
	if (status == B_OK) {
		fSetterStatusLabel->SetText(B_TRANSLATE("Wallpaper applied."));
	} else {
		BString text = OperationFailureText(
			status, setter.LastRollbackStatus());
		fSetterStatusLabel->SetText(text.String());
	}

	fApplyButton->SetEnabled(true);
}
