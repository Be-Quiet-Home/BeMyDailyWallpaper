#include "MainWindow.h"

#include "DailyImageProvider.h"
#include "DeskbarView.h"
#include "DesktopWallpaperTarget.h"
#include "ProviderResolver.h"
#include "WallpaperSetter.h"

#include <Application.h>
#include <Button.h>
#include <Catalog.h>
#include <Entry.h>
#include <Errors.h>
#include <FilePanel.h>
#include <GroupLayout.h>
#include <GroupView.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Messenger.h>
#include <Path.h>
#include <Rect.h>
#include <String.h>
#include <StringView.h>
#include <SupportDefs.h>

#include <string.h>
#include <time.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


static const uint32 kApplyWallpaper = 'ApWp';
static const uint32 kChooseLocalFolder = 'ChFd';
static const uint32 kLocalFolderSelected = 'FdSl';


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


static status_t
CurrentLocalDate(BString& date)
{
	date = "";

	time_t now = time(NULL);
	if (now == (time_t)-1)
		return B_ERROR;

	struct tm localTime;
	if (localtime_r(&now, &localTime) == NULL)
		return B_ERROR;

	char buffer[11];
	if (strftime(buffer, sizeof(buffer), "%Y-%m-%d", &localTime) != 10)
		return B_ERROR;

	date = buffer;
	return B_OK;
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
	fSettings(),
	fProviderResult(),
	fFolderPanel(NULL),
	fDeskbarPreview(NULL),
	fSettingsStatusLabel(NULL),
	fPreviewLabel(NULL),
	fProviderStatusLabel(NULL),
	fDailyStatusLabel(NULL),
	fFolderPathLabel(NULL),
	fChooseFolderButton(NULL),
	fApplyButton(NULL),
	fSetterStatusLabel(NULL)
{
	status_t settingsLoadStatus = fSettings.Load();
	BString settingsStatus = SettingsStatusText(
		fSettings, settingsLoadStatus);

	BGroupView* background = new BGroupView("background", B_VERTICAL);
	background->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BStringView* label = new BStringView(
		"label",
		B_TRANSLATE("BeMyDailyWall is alive."));

	fSettingsStatusLabel = new BStringView(
		"settingsStatusLabel",
		settingsStatus.String());

	fDeskbarPreview = new DeskbarView();

	fPreviewLabel = new BStringView(
		"previewLabel",
		B_TRANSLATE("Deskbar icon preview without provider data"));

	fProviderStatusLabel = new BStringView(
		"providerStatusLabel",
		"");

	fDailyStatusLabel = new BStringView(
		"dailyStatusLabel",
		"");

	fFolderPathLabel = new BStringView(
		"folderPathLabel",
		"");

	fChooseFolderButton = new BButton(
		"chooseFolderButton",
		B_TRANSLATE("Choose folder" B_UTF8_ELLIPSIS),
		new BMessage(kChooseLocalFolder));

	fSetterStatusLabel = new BStringView(
		"setterStatusLabel",
		"");

	fApplyButton = new BButton(
		"applyWallpaperButton",
		B_TRANSLATE("Apply wallpaper"),
		new BMessage(kApplyWallpaper));
	fApplyButton->SetEnabled(false);

	BMessenger target(this);
	BMessage selectionMessage(kLocalFolderSelected);
	fFolderPanel = new BFilePanel(
		B_OPEN_PANEL, &target, NULL, B_DIRECTORY_NODE, false,
		&selectionMessage, NULL, false, true);
	fFolderPanel->SetButtonLabel(
		B_DEFAULT_BUTTON, B_TRANSLATE("Select folder"));
	fFolderPanel->Window()->SetTitle(
		B_TRANSLATE("Choose wallpaper folder"));

	BLayoutBuilder::Group<>(background, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(label)
		.Add(fSettingsStatusLabel)
		.AddGroup(B_HORIZONTAL)
			.Add(fDeskbarPreview)
			.Add(fPreviewLabel)
			.AddGlue()
		.End()
		.Add(fProviderStatusLabel)
		.Add(fDailyStatusLabel)
		.AddGroup(B_HORIZONTAL)
			.Add(fFolderPathLabel)
			.AddGlue()
			.Add(fChooseFolderButton)
		.End()
		.Add(fSetterStatusLabel)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fApplyButton)
		.End()
		.AddGlue()
	.End();

	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(background);

	UpdateDailyStatus();
	UpdateFolderPath();
	ReloadProvider();
	ResizeToPreferred();
}


MainWindow::~MainWindow()
{
	delete fFolderPanel;
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kApplyWallpaper:
			ApplyWallpaper();
			break;

		case kChooseLocalFolder:
			ChooseLocalFolder();
			break;

		case kLocalFolderSelected:
			LocalFolderSelected(message);
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
	fChooseFolderButton->SetEnabled(false);

	DesktopWallpaperTarget target;
	status_t status = target.Resolve();
	if (status != B_OK) {
		BString text(B_TRANSLATE_COMMENT(
			"Wallpaper target failed: %error%",
			"%error% is a Haiku status description."));
		text.ReplaceFirst("%error%", strerror(status));
		fSetterStatusLabel->SetText(text.String());
		fChooseFolderButton->SetEnabled(true);
		fApplyButton->SetEnabled(true);
		return;
	}

	WallpaperSetter setter(target.Node(), target.Messenger());
	status = setter.Apply(fProviderResult);
	if (status == B_OK) {
		BString updateDate;
		status_t historyStatus = CurrentLocalDate(updateDate);
		if (historyStatus == B_OK) {
			BString previousImagePath(fSettings.LastImagePath());
			BString previousUpdateDate(fSettings.LastUpdateDate());

			fSettings.SetLastImagePath(
				fProviderResult.ImagePath().String());
			fSettings.SetLastUpdateDate(updateDate.String());

			historyStatus = fSettings.Save();
			if (historyStatus != B_OK) {
				fSettings.SetLastImagePath(previousImagePath.String());
				fSettings.SetLastUpdateDate(previousUpdateDate.String());
			}
		}

		if (historyStatus == B_OK) {
			UpdateDailyStatus();
			fSetterStatusLabel->SetText(B_TRANSLATE(
				"Wallpaper applied and history saved."));
		} else {
			BString text(B_TRANSLATE_COMMENT(
				"Wallpaper applied, but history save failed: %error%",
				"%error% is a Haiku status description."));
			text.ReplaceFirst("%error%", strerror(historyStatus));
			fSetterStatusLabel->SetText(text.String());
		}
	} else {
		BString text = OperationFailureText(
			status, setter.LastRollbackStatus());
		fSetterStatusLabel->SetText(text.String());
	}

	fChooseFolderButton->SetEnabled(true);
	fApplyButton->SetEnabled(true);
}


void
MainWindow::ChooseLocalFolder()
{
	if (!fSettings.LocalFolderPath().IsEmpty()) {
		fFolderPanel->SetPanelDirectory(
			fSettings.LocalFolderPath().String());
	}

	if (!fFolderPanel->IsShowing())
		fFolderPanel->Show();
}


void
MainWindow::LocalFolderSelected(BMessage* message)
{
	entry_ref ref;
	status_t status = message->FindRef("refs", &ref);
	if (status != B_OK) {
		BString text(B_TRANSLATE_COMMENT(
			"Folder selection failed: %error%",
			"%error% is a Haiku status description."));
		text.ReplaceFirst("%error%", strerror(status));
		fSetterStatusLabel->SetText(text.String());
		return;
	}

	BEntry entry(&ref, true);
	status = entry.InitCheck();
	if (status == B_OK && !entry.Exists())
		status = B_ENTRY_NOT_FOUND;
	if (status == B_OK && !entry.IsDirectory())
		status = B_NOT_A_DIRECTORY;

	BPath path;
	if (status == B_OK)
		status = entry.GetPath(&path);

	if (status != B_OK) {
		BString text(B_TRANSLATE_COMMENT(
			"Folder selection failed: %error%",
			"%error% is a Haiku status description."));
		text.ReplaceFirst("%error%", strerror(status));
		fSetterStatusLabel->SetText(text.String());
		return;
	}

	BString previousProvider(fSettings.ProviderName());
	BString previousPath(fSettings.LocalFolderPath());

	fSettings.SetProviderName("Local folder");
	fSettings.SetLocalFolderPath(path.Path());

	status = fSettings.Save();
	if (status != B_OK) {
		fSettings.SetProviderName(previousProvider.String());
		fSettings.SetLocalFolderPath(previousPath.String());

		BString text(B_TRANSLATE_COMMENT(
			"Settings save failed: %error%",
			"%error% is a Haiku status description."));
		text.ReplaceFirst("%error%", strerror(status));
		fSetterStatusLabel->SetText(text.String());
		return;
	}

	BString settingsStatus = SettingsStatusText(fSettings, B_OK);
	fSettingsStatusLabel->SetText(settingsStatus.String());

	UpdateFolderPath();
	ReloadProvider();
}


status_t
MainWindow::ReloadProvider()
{
	fProviderResult = ProviderResult();

	DailyImageProvider* provider = NULL;
	status_t status = ProviderResolver::Create(fSettings, provider);

	BString providerName(fSettings.ProviderName());
	if (status == B_OK) {
		providerName = provider->Name();
		status = provider->Fetch(fProviderResult);
	}

	delete provider;
	provider = NULL;

	fDeskbarPreview->SetInfo(fProviderResult.Info());
	fPreviewLabel->SetText(status == B_OK
		? B_TRANSLATE("Deskbar icon preview with provider tooltip")
		: B_TRANSLATE("Deskbar icon preview without provider data"));

	BString providerStatusText(status == B_OK
		? B_TRANSLATE_COMMENT(
			"Provider: %provider% loaded.",
			"%provider% is a provider name.")
		: B_TRANSLATE_COMMENT(
			"Provider: %provider% failed.",
			"%provider% is a provider name."));
	providerStatusText.ReplaceFirst(
		"%provider%", providerName.String());
	fProviderStatusLabel->SetText(providerStatusText.String());

	const bool canApply = status == B_OK
		&& fProviderResult.HasImagePath();
	fApplyButton->SetEnabled(canApply);

	if (status != B_OK) {
		fSetterStatusLabel->SetText(B_TRANSLATE(
			"Wallpaper: unavailable because the provider failed."));
	} else if (!fProviderResult.HasImagePath()) {
		fSetterStatusLabel->SetText(B_TRANSLATE(
			"Wallpaper: no image path from the provider."));
	} else {
		fSetterStatusLabel->SetText(B_TRANSLATE(
			"Wallpaper: ready to apply."));
	}

	return status;
}


void
MainWindow::UpdateDailyStatus()
{
	BString today;
	status_t status = CurrentLocalDate(today);
	if (status != B_OK) {
		fDailyStatusLabel->SetText(B_TRANSLATE(
			"Daily status: unavailable."));
		return;
	}

	if (fSettings.LastUpdateDate().Compare(today.String()) == 0) {
		fDailyStatusLabel->SetText(B_TRANSLATE(
			"Daily status: today's wallpaper is already applied."));
	} else {
		fDailyStatusLabel->SetText(B_TRANSLATE(
			"Daily status: no wallpaper has been applied today."));
	}
}


void
MainWindow::UpdateFolderPath()
{
	if (fSettings.LocalFolderPath().IsEmpty()) {
		fFolderPathLabel->SetText(B_TRANSLATE(
			"Wallpaper folder: not selected."));
		fFolderPathLabel->SetToolTip("");
		return;
	}

	BPath path(fSettings.LocalFolderPath().String());
	const char* folderName = path.InitCheck() == B_OK
		? path.Leaf() : fSettings.LocalFolderPath().String();
	if (folderName == NULL || folderName[0] == '\0')
		folderName = fSettings.LocalFolderPath().String();

	BString text(B_TRANSLATE_COMMENT(
		"Wallpaper folder: %folder%",
		"%folder% is the selected directory name."));
	text.ReplaceFirst("%folder%", folderName);
	fFolderPathLabel->SetText(text.String());
	fFolderPathLabel->SetToolTip(
		fSettings.LocalFolderPath().String());
}
