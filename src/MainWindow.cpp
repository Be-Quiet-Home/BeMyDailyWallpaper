#include "MainWindow.h"

#include "DailyImageProvider.h"
#include "DailyWallpaperAction.h"
#include "DailyWallpaperPolicy.h"
#include "DeskbarView.h"
#include "DesktopWallpaperTarget.h"
#include "ProviderResolver.h"
#include "WallpaperSetter.h"

#include <Application.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
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


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


static const uint32 kApplyWallpaper = 'ApWp';
static const uint32 kChooseLocalFolder = 'ChFd';
static const uint32 kLocalFolderSelected = 'FdSl';
static const uint32 kStartupApplyChanged = 'StAp';


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


struct WallpaperActionContext {
	WallpaperSetter* setter;
};


static status_t
ApplyWallpaperCandidate(
	void* context,
	const ProviderResult& result,
	status_t& rollbackStatus)
{
	if (context == NULL)
		return B_BAD_VALUE;

	WallpaperActionContext* actionContext
		= static_cast<WallpaperActionContext*>(context);
	if (actionContext->setter == NULL)
		return B_BAD_VALUE;

	status_t status = actionContext->setter->Apply(result);
	rollbackStatus = actionContext->setter->LastRollbackStatus();
	return status;
}


static status_t
CurrentWallpaperDate(void*, BString& date)
{
	return DailyWallpaperPolicy::CurrentLocalDate(date);
}


static status_t
SaveWallpaperHistory(void*, const AppSettings& settings)
{
	return settings.Save();
}


static status_t
ResolveStartupTarget(void* context)
{
	if (context == NULL)
		return B_BAD_VALUE;

	DesktopWallpaperTarget* target
		= static_cast<DesktopWallpaperTarget*>(context);
	status_t status = target->Resolve();
	if (status != B_OK)
		return status;

	return target->IsReady() ? B_OK : B_NO_INIT;
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
	fStartupApplyCheckBox(NULL),
	fStartupActionStatusLabel(NULL),
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

	fStartupApplyCheckBox = new BCheckBox(
		"startupApplyCheckBox",
		B_TRANSLATE("Apply today's wallpaper automatically at startup"),
		new BMessage(kStartupApplyChanged));
	fStartupApplyCheckBox->SetValue(
		fSettings.StartupApplyEnabled() ? B_CONTROL_ON : B_CONTROL_OFF);

	fStartupActionStatusLabel = new BStringView(
		"startupActionStatusLabel",
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
		.Add(fStartupApplyCheckBox)
		.Add(fStartupActionStatusLabel)
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

	UpdateFolderPath();
	ReloadProvider();
	ExecuteStartupAction();
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

		case kStartupApplyChanged:
			StartupApplyChanged();
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
	WallpaperActionContext actionContext = {&setter};
	DailyWallpaperActionCallbacks callbacks = {
		ApplyWallpaperCandidate,
		CurrentWallpaperDate,
		SaveWallpaperHistory,
		&actionContext
	};

	DailyWallpaperActionResult actionResult
		= DailyWallpaperAction::Execute(
			fSettings, fProviderResult, callbacks);

	if (actionResult.applyStatus == B_OK) {
		if (actionResult.historyStatus == B_OK) {
			ReloadProvider();
			fSetterStatusLabel->SetText(B_TRANSLATE(
				"Wallpaper applied and history saved."));
			fChooseFolderButton->SetEnabled(true);
			return;
		}

		BString text(B_TRANSLATE_COMMENT(
			"Wallpaper applied, but history save failed: %error%",
			"%error% is a Haiku status description."));
		text.ReplaceFirst(
			"%error%", strerror(actionResult.historyStatus));
		fSetterStatusLabel->SetText(text.String());
	} else {
		BString text = OperationFailureText(
			actionResult.applyStatus,
			actionResult.rollbackStatus);
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

	UpdateDailyStatus();
	UpdateStartupActionStatus();
	return status;
}


void
MainWindow::StartupApplyChanged()
{
	const bool enabled
		= fStartupApplyCheckBox->Value() == B_CONTROL_ON;
	const bool previousEnabled = fSettings.StartupApplyEnabled();

	if (enabled == previousEnabled)
		return;

	fStartupApplyCheckBox->SetEnabled(false);
	fSettings.SetStartupApplyEnabled(enabled);

	status_t status = fSettings.Save();
	if (status == B_OK) {
		fSetterStatusLabel->SetText(B_TRANSLATE(
			"Startup apply setting saved."));
	} else {
		fSettings.SetStartupApplyEnabled(previousEnabled);
		fStartupApplyCheckBox->SetValue(
			previousEnabled ? B_CONTROL_ON : B_CONTROL_OFF);

		BString text(B_TRANSLATE_COMMENT(
			"Settings save failed: %error%",
			"%error% is a Haiku status description."));
		text.ReplaceFirst("%error%", strerror(status));
		fSetterStatusLabel->SetText(text.String());
	}

	UpdateStartupActionStatus();
	fStartupApplyCheckBox->SetEnabled(true);
}



DailyWallpaperReadiness
MainWindow::CurrentDailyReadiness() const
{
	BString today;
	status_t status = DailyWallpaperPolicy::CurrentLocalDate(today);

	DailyWallpaperState state = status == B_OK
		? DailyWallpaperPolicy::Evaluate(
			fSettings.LastUpdateDate().String(), today.String())
		: DAILY_WALLPAPER_UNAVAILABLE;

	return DailyWallpaperPolicy::EvaluateReadiness(
		state, fProviderResult.HasImagePath());
}


DailyWallpaperStartupAction
MainWindow::CurrentStartupAction() const
{
	return DailyWallpaperStartupPlan::Plan(
		CurrentDailyReadiness(),
		fSettings.StartupApplyEnabled());
}


status_t
MainWindow::ExecuteStartupAction()
{
	DailyWallpaperStartupAction action = CurrentStartupAction();
	DesktopWallpaperTarget target;
	status_t status = DailyWallpaperStartupPlan::Execute(
		action, ResolveStartupTarget, &target);

	if (action != DAILY_WALLPAPER_STARTUP_APPLY_ONCE)
		return status;

	if (status == B_OK) {
		fStartupActionStatusLabel->SetText(B_TRANSLATE(
			"Startup target: ready."));
	} else {
		BString text(B_TRANSLATE_COMMENT(
			"Startup target failed: %error%",
			"%error% is a Haiku status description."));
		text.ReplaceFirst("%error%", strerror(status));
		fStartupActionStatusLabel->SetText(text.String());
	}

	return status;
}


void
MainWindow::UpdateDailyStatus()
{
	DailyWallpaperReadiness readiness = CurrentDailyReadiness();

	switch (readiness) {
		case DAILY_WALLPAPER_READINESS_APPLIED_TODAY:
			fDailyStatusLabel->SetText(B_TRANSLATE(
				"Daily status: today's wallpaper is already applied."));
			break;

		case DAILY_WALLPAPER_READINESS_NO_CANDIDATE:
			fDailyStatusLabel->SetText(B_TRANSLATE(
				"Daily status: no wallpaper candidate is available."));
			break;

		case DAILY_WALLPAPER_READINESS_READY:
			fDailyStatusLabel->SetText(B_TRANSLATE(
				"Daily status: wallpaper is ready to apply."));
			break;

		case DAILY_WALLPAPER_READINESS_UNAVAILABLE:
		default:
			fDailyStatusLabel->SetText(B_TRANSLATE(
				"Daily status: unavailable."));
			break;
	}
}


void
MainWindow::UpdateStartupActionStatus()
{
	DailyWallpaperStartupAction action = CurrentStartupAction();

	if (!fSettings.StartupApplyEnabled()) {
		fStartupActionStatusLabel->SetText(B_TRANSLATE(
			"Startup action: disabled."));
		return;
	}

	switch (action) {
		case DAILY_WALLPAPER_STARTUP_APPLY_ONCE:
			fStartupActionStatusLabel->SetText(B_TRANSLATE(
				"Startup action: apply once."));
			break;

		case DAILY_WALLPAPER_STARTUP_DO_NOTHING:
		default:
			fStartupActionStatusLabel->SetText(B_TRANSLATE(
				"Startup action: not needed."));
			break;
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
